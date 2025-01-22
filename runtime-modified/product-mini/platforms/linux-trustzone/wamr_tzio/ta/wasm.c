#include <stdio.h>
#include "logging.h"
#include "wasm.h"
#include "tee_benchmarks.h"
#include "native_lib.h"

const char *native_lib_list[8]={NULL};
uint32 native_lib_count=0;
struct native_lib *native_lib_loaded_list[8];
uint32 native_lib_loaded_count=0;

wamr_context *singleton_wamr_context;

void TA_SetOutputBuffer(void *output_buffer, uint64_t output_buffer_size) {
    tzio_set_output_buffer(output_buffer, output_buffer_size);
}

TEE_Result TA_InitializeWamrRuntime(wamr_context* context, int argc, char** argv)
{
    RuntimeInitArgs init_args;
    TEE_MemFill(&init_args, 0, sizeof(RuntimeInitArgs));

    init_args.mem_alloc_type = Alloc_With_Pool;
    init_args.mem_alloc_option.pool.heap_buf = context->heap_buf;
    init_args.mem_alloc_option.pool.heap_size = context->heap_size;

    if (!wasm_runtime_full_init(&init_args)) {
        EMSG("Init runtime environment failed.\n");
        return TEE_ERROR_GENERIC;
    }

    // native-libs
    // rpi3 gpio
    const char * gpio_lib="b3091a65-9751-4784-abf7-0298a7cc35bc"; 
    // tzio
    const char * tzio_lib="a6799c0f-518a-4107-87be-33757c906262";
    native_lib_list[0]=gpio_lib;
    native_lib_list[1]=tzio_lib;
    native_lib_count=2;
    native_lib_loaded_count=load_and_register_native_libs(native_lib_list,native_lib_count,native_lib_loaded_list);
    IMSG("load %d native lib(s)",native_lib_loaded_count);


#ifdef PROFILING_LAUNCH_TIME
    TEE_GetREETime(benchmark_get_store(PROFILING_LAUNCH_TIME_END_INIT));
#endif

    char error_buf[128];
    context->module = wasm_runtime_load(context->wasm_bytecode, context->wasm_bytecode_size, error_buf, sizeof(error_buf));
    if(!context->module) {
        EMSG("Load wasm module failed. error: %s\n", error_buf);
        return TEE_ERROR_GENERIC;
    }

#ifdef PROFILING_LAUNCH_TIME
    TEE_GetREETime(benchmark_get_store(PROFILING_LAUNCH_TIME_END_LOAD));
#endif

    wasm_runtime_set_wasi_args(context->module, NULL, 0, NULL, 0, NULL, 0, argv, argc);

    uint32_t stack_size = 256 * 1024, heap_size = 8096;
    context->module_inst = wasm_runtime_instantiate(context->module,
                                         stack_size,
                                         heap_size,
                                         error_buf,
                                         sizeof(error_buf));
    if (!context->module_inst) {
        EMSG("Instantiate wasm module failed. error: %s\n", error_buf);
        return TEE_ERROR_GENERIC;
    }

#ifdef PROFILING_LAUNCH_TIME
    TEE_GetREETime(benchmark_get_store(PROFILING_LAUNCH_TIME_END_INSTANTIATE));
#endif

    singleton_wamr_context = context;

    return TEE_SUCCESS;
}

TEE_Result TA_ExecuteWamrRuntime(wamr_context* context)
{
    if (!wasm_application_execute_main(context->module_inst, 0, NULL))
    {
        EMSG("call wasm entry point test failed. %s\n", wasm_runtime_get_exception(context->module_inst));
        return TEE_ERROR_GENERIC;
    }

    return TEE_SUCCESS;
}

void TA_TearDownWamrRuntime(wamr_context* context)
{
    singleton_wamr_context = NULL;

    if (context->module_inst)
    {
        wasm_runtime_deinstantiate(context->module_inst);
    }

    if (context->module) wasm_runtime_unload(context->module);
    wasm_runtime_destroy();
}