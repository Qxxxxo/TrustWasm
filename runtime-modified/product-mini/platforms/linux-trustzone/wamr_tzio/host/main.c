// Standard C library headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

// GlobalPlatform Client API
#include <tee_client_api.h>

// GlobalPlatfrom TA
#include <wamr_ta.h>

#include "secure_logging.h"

#define timespec_to_micro(t) \
    t.tv_sec * 1000 * 1000 + t.tv_nsec / 1000

/* TEE resources */
typedef struct _tee_ctx {
	TEEC_Context ctx;
	TEEC_Session sess;
    uint8_t *output_buffer;
    uint64_t output_buffer_size;
    uint8_t *benchmark_buffer;
    uint64_t benchmark_buffer_size;
} tee_ctx;

static void prepare_tee_session(uint32_t thread_id, tee_ctx* ctx)
{
	TEEC_UUID uuid = TA_WAMR_UUID;
	uint32_t origin;
	TEEC_Result res;

	/* Initialize a context connecting us to the TEE */
	res = TEEC_InitializeContext(NULL, &ctx->ctx);
	if (res != TEEC_SUCCESS){
        ERROR("[%d] TEEC_InitializeContext failed with code 0x%x", thread_id,res);
        pthread_exit((void *)1);
    }

	/* Open a session with the TA */
	res = TEEC_OpenSession(&ctx->ctx, &ctx->sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, NULL, &origin);
    // STUCK HERE
	if (res != TEEC_SUCCESS){
        ERROR("[%d] TEEC_Opensession failed with code 0x%x origin 0x%x",
			thread_id,res, origin);
        pthread_exit((void *)1);
    }
}

static void configure_heap_size(uint32_t thread_id, tee_ctx *ctx, uint32_t size) {
    TEEC_Operation op;
	uint32_t origin;
	TEEC_Result res;

    memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
	op.params[0].value.a = size;

	res = TEEC_InvokeCommand(&ctx->sess, COMMAND_CONFIGURE_HEAP, &op, &origin);
    if (res != TEEC_SUCCESS) {
        ERROR("[%d] The heap size of WaTZ cannot be configured: %x",thread_id,res);
    }
}

static void allocate_buffers(uint32_t __attribute__((unused)) thread_id,tee_ctx* ctx, uint64_t buffers_size) {
    // The output buffer is used to capture writes to stdout from the WASM
    ctx->output_buffer = malloc(buffers_size);
    ctx->output_buffer_size = buffers_size;

    // The benchmark buffer is used to capture benchmark information from the TA
    ctx->benchmark_buffer = malloc(buffers_size);
    ctx->benchmark_buffer_size = buffers_size;
}

static bool start_wasm(uint32_t thread_id, tee_ctx* ctx, char* wasm_path, char* arg)
{
    TEEC_Operation op;
	uint32_t origin;
	TEEC_Result res;
    FILE* wasm_file = NULL;
    long wasm_file_length;
    unsigned char *wasm_bytecode;

    //
    // Dumping the Wasm bytecode
    //
    // Open the file in binary mode
    wasm_file = fopen(wasm_path, "rb");
    if (wasm_file == NULL)
    {
        
        ERROR("[%d] the file %s cannot be opened.",thread_id,wasm_path);
        return false;
    }
    // Jump to the end of the file
    fseek(wasm_file, 0, SEEK_END);
    // Get the current byte offset in the file
    wasm_file_length = ftell(wasm_file);
    // Allocate the buffer for the bytecode with the size of the file
    wasm_bytecode = malloc(ftell(wasm_file) * sizeof(unsigned char));
    // Jump back to the beginning of the file  
    rewind(wasm_file);
    // Dump the bytecode into the buffer
    fread(wasm_bytecode, wasm_file_length, 1, wasm_file);
    // Close the file
    fclose(wasm_file);

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
					 TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INOUT, TEEC_MEMREF_TEMP_INOUT);
	op.params[0].tmpref.buffer = wasm_bytecode;
	op.params[0].tmpref.size = wasm_file_length;
    op.params[1].tmpref.buffer = arg;
	op.params[1].tmpref.size = arg != NULL ? strlen(arg) : 0;
    op.params[2].tmpref.buffer = ctx->output_buffer;
    op.params[2].tmpref.size = ctx->output_buffer_size;
    op.params[3].tmpref.buffer = ctx->benchmark_buffer;
    op.params[3].tmpref.size = ctx->benchmark_buffer_size;

#ifdef PROFILING_LAUNCH_TIME
    // We start measuring just before the invocation of the code that launches the WASM app in the TA
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
#endif

	res = TEEC_InvokeCommand(&ctx->sess, COMMAND_RUN_WASM, &op, &origin);
    
#ifdef PROFILING_LAUNCH_TIME
    // The measurement finished in the WASM app. We are free to take some time to display numbers here.
    // We print the recorded start at the first column of data. The second one will be printed when
    // the output buffer is printed.
    printf("%ld,", timespec_to_micro(time));
#endif

	if (res != TEEC_SUCCESS)
    {
        ERROR("[%d] TEEC_InvokeCommand(START_WASM) failed 0x%x origin 0x%x",
			thread_id,res, origin);
        pthread_exit((void *)1);
    }

    return true;
}

static void terminate_tee_session(tee_ctx* ctx)
{
	TEEC_CloseSession(&ctx->sess);
	TEEC_FinalizeContext(&ctx->ctx);
}

static void print_buffers(uint32_t thread_id,tee_ctx* ctx) {
    INFO("[%d] %s", thread_id, ctx->benchmark_buffer);
    for(int i=0;i<ctx->output_buffer_size;i++){
        printf("%c",ctx->output_buffer[i]);
    }
}

static void free_buffers(tee_ctx* ctx) {
    ctx->output_buffer_size = 0;
    ctx->benchmark_buffer_size = 0;
    free(ctx->output_buffer);
    free(ctx->benchmark_buffer);
}

typedef struct {
    uint32_t thread_id;
    uint32_t heap_size;
    char * wasm_path;
    char * arg;
} run_wasm_thread_args;

void * run_wasm_thread(void * arg){
    run_wasm_thread_args * args = (run_wasm_thread_args *)arg;
    INFO("[%d] start wasm",args->thread_id);
    tee_ctx ctx;
    bool success = true;
    allocate_buffers(args->thread_id,&ctx,20*1024);
    prepare_tee_session(args->thread_id,&ctx);
    configure_heap_size(args->thread_id,&ctx,args->heap_size);
    success = start_wasm(args->thread_id,&ctx,args->wasm_path,args->arg);
    terminate_tee_session(&ctx);
    print_buffers(args->thread_id,&ctx);
    free_buffers(&ctx);
    free(args);
    if(!success)
        ERROR("[%d] start wasm failed",args->thread_id);
    else
        INFO("[%d] successfully finish run wasm",args->thread_id);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf("ERROR: The number of arguments does not match.\n");
        printf("SYNTAX: %s heap_size concurrent_thread_num wasm_path [wasm_arg]\n", argv[0]);
        exit(1);
    }

    uint32_t heap_size = atoi(argv[1]);
    uint32_t thread_num = atoi(argv[2]);
    char* wasm_path = argv[3];
    char* arg = argc > 4 ? argv[4] : NULL;
    int thread_create_res;
    pthread_t *threads = (pthread_t *)malloc(thread_num*sizeof(pthread_t));
    int* thread_ids=(int *)malloc(thread_num*sizeof(int));
    if(threads==NULL||thread_ids == NULL){
        ERROR("lack memory for threads");
        return 1;
    }
    
    for(int i=0;i<thread_num;i++){
        thread_ids[i]=i;
        run_wasm_thread_args * args = (run_wasm_thread_args *) malloc(sizeof(run_wasm_thread_args));
        args->thread_id=i;
        args->heap_size=heap_size;
        args->wasm_path=wasm_path;
        args->arg=arg;
        thread_create_res=pthread_create(&threads[i],NULL,run_wasm_thread,args);
        if(thread_create_res!=0){
            ERROR("failed to create thread %d",i);
        }
        sleep(1);
    }

    // wait all thread finish
    for(int i=0;i<thread_num;i++){
        if(threads!=NULL&&threads[i]!=(pthread_t)NULL)
            pthread_join(threads[i],NULL);
    }
    INFO("All thread exit");
    return EXIT_SUCCESS;
}