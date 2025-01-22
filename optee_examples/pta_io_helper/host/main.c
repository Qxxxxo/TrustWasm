/**
 * @brief pta io helper
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <err.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

/* OP-TEE TEE client API (built by optee_client) */
#include <tee_client_api.h>

#include "secure_logging.h"
// #include "pta_io_helper.h"

// pta io helper uuid and cmd
// a84e4d9d-ad15-4cbc-83c9-d8e4a73935b5
#define PTA_IO_HELPER_UUID                                 \
    {                                                      \
        0xa84e4d9d, 0xad15, 0x4cbc,                        \
        {                                                  \
            0x83, 0xc9, 0xd8, 0xe4, 0xa7, 0x39, 0x35, 0xb5 \
        }                                                  \
    }   
#define PTA_IO_HELPER_CMD_START_LOOP    0   

#define LOOP_NUM 5                                                

typedef struct {
	uint32_t thread_id;
	uint32_t periph_no;
} pta_io_helper_loop_thread_args;

static inline uint64_t time_micro(struct timespec * t){
	return (uint64_t)t->tv_sec*1000000+(uint64_t)t->tv_nsec/1000;
}

static inline uint64_t cal_time_interval(struct timespec* t1, struct timespec * t2){
	return (uint64_t)(t2->tv_sec-t1->tv_sec) * 1000000 +  (uint64_t)(t2->tv_nsec-t1->tv_nsec)/1000;
}

void *  pta_io_helper_loop_thread (void * arg){
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = PTA_IO_HELPER_UUID;
	uint32_t err_origin;
	pta_io_helper_loop_thread_args * args = (pta_io_helper_loop_thread_args *)arg;
	INFO("[%d] start loop with periph_no %d",args->thread_id,args->periph_no);
	res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS){
		ERROR("TEEC_InitializeContext failed with code 0x%x", res);
		pthread_exit(NULL);
	}
	res = TEEC_OpenSession(&ctx, &sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS){
		ERROR("TEEC_Opensession failed with code 0x%x origin 0x%x",
			res, err_origin);
		pthread_exit(NULL);
	}
	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE,
					 TEEC_NONE, TEEC_NONE);
	op.params[0].value.a=args->periph_no;
	res = TEEC_InvokeCommand(&sess, PTA_IO_HELPER_CMD_START_LOOP, &op,
					&err_origin);
	TEEC_CloseSession(&sess);
	TEEC_FinalizeContext(&ctx);
	pthread_exit(NULL);
}

int main(void)
{
	struct timespec start, end;
	int thread_create_res;
	uint16_t periph_no_array[LOOP_NUM]={22, // GPIO 22
						 28, // I2C 0
						 29, // I2C 1
						 30, // SPI 0
						 31}; // SECURE STORAGE
	pthread_t *threads = (pthread_t *)malloc(LOOP_NUM*sizeof(pthread_t));
    int* thread_ids=(int *)malloc(LOOP_NUM*sizeof(int));
	if(threads==NULL||thread_ids == NULL){
        ERROR("lack memory for threads");
        return 1;
    }
	for(int i=0;i<LOOP_NUM;i++){
		thread_ids[i]=i;
		pta_io_helper_loop_thread_args * args = (pta_io_helper_loop_thread_args*)malloc(sizeof(pta_io_helper_loop_thread_args));
		args->thread_id=i;
		args->periph_no=periph_no_array[i];
		thread_create_res=pthread_create(&threads[i],NULL,pta_io_helper_loop_thread,args);
		if(thread_create_res!=0){
			ERROR("failed to create thread %d",i);
		}
		// sleep(1);
	}

	// wait all thread finish
    for(int i=0;i<LOOP_NUM;i++){
        if(threads!=NULL&&threads[i]!=(pthread_t)NULL)
            pthread_join(threads[i],NULL);
    }
    INFO("All thread exit");
    return EXIT_SUCCESS;
}
