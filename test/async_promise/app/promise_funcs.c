/**
 * promise native funcs
 */
static int promise_set_callback_wrapper(wasm_exec_env_t exec_env, int promise, uint32_t handler_func_ptr){
    // high 16 bits is periph_no
    // low 16 bits is sub signo
    uint16_t periph_no= (uint16_t)(promise>>16);
    uint16_t sub_signo= (uint16_t)(promise&0xFFFF);

    if(periph_no>=N_SESS) return TEE_ERROR_BAD_PARAMETERS;
    // sub_signo 
    if(sub_signo>=N_SESS) return TEE_ERROR_BAD_PARAMETERS;
    // poll sub signal once to check if the sub signo is using
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    param_types=TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                TEE_PARAM_TYPE_NONE,
                                TEE_PARAM_TYPE_NONE,
                                TEE_PARAM_TYPE_NONE);
    params[0].value.a=periph_no;
    params[0].value.b=sub_signo;

    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_POLL_SUBSIGNAL,param_types,params);
    if(res!=TEE_SUCCESS){
        // sub signal is not used or io helper error
        return TEE_ERROR_ACCESS_DENIED;
    }
    // sub signo is used
    // set callback
    if(tzio_register_handler(exec_env,(uint32_t)periph_no,(uint32_t)sub_signo,handler_func_ptr)<0){
        EMSG("register handler to sigtable failed");
        return TEE_ERROR_GENERIC;
    }
    return TEE_SUCCESS;
}

static int get_async_data_promise_wrapper(wasm_exec_env_t exec_env, int promise, uint32_t dst_buf_addr,uint32_t dst_buf_len_addr){
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    // high 16 bits is periph_no
    // low 16 bits is sub signo
    uint16_t periph_no= (uint16_t)(promise>>16);
    uint16_t sub_signo= (uint16_t)(promise&0xFFFF);

    if(periph_no>=N_SESS) return TEE_ERROR_BAD_PARAMETERS;
    // sub_signo 
    if(sub_signo>=N_SESS) return TEE_ERROR_BAD_PARAMETERS;

#ifdef USE_FAT_PTR_CHECK
    if(dst_buf_addr!=0){
        if(!check_ptr_signed(dst_buf_addr)){
            return TEE_ERROR_ACCESS_DENIED;
        }
        dst_buf_addr=clear_signature_and_tag(dst_buf_addr);
    }
    if(dst_buf_len_addr!=0){
        if(!check_ptr_signed(dst_buf_len_addr)){
            return TEE_ERROR_ACCESS_DENIED;
        }
        dst_buf_len_addr=clear_signature_and_tag(dst_buf_len_addr);
    }
#endif
    // int poll_times=1;
    // signo is periph_no
    if(periph_no>=N_SESS) return TEE_ERROR_BAD_PARAMETERS;
    // sub_signo 
    if(sub_signo>=N_SESS) return TEE_ERROR_BAD_PARAMETERS;

    wasm_module_inst_t inst= wasm_runtime_get_module_inst(exec_env);
    // validate buf len ptr
    if(!wasm_runtime_validate_app_addr(inst,dst_buf_len_addr,sizeof(uint32_t))){
        return TEE_ERROR_ACCESS_DENIED;
    }
    uint32_t * buf_len = wasm_runtime_addr_app_to_native(inst,dst_buf_len_addr);
    // validate buf ptr 
    if(!wasm_runtime_validate_app_addr(inst,dst_buf_addr,*buf_len)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    void * buf = wasm_runtime_addr_app_to_native(inst,dst_buf_addr);

    /**
     * Poll stage
     */
    param_types=TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                TEE_PARAM_TYPE_NONE,
                                TEE_PARAM_TYPE_NONE,
                                TEE_PARAM_TYPE_NONE);
    params[0].value.a=periph_no;
    params[0].value.b=sub_signo;
    // poll to io helper sub signo until ready
    // may need timeout here
    
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_POLL_SUBSIGNAL,param_types,params);
    while(res==TEE_SUCCESS){
        // get poll return
        // return 1, subsignal is ready
        if(params[0].value.a>0) break;
        // reset params!!!
        params[0].value.a=periph_no;
        params[0].value.b=sub_signo;
        res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_POLL_SUBSIGNAL,param_types,params);
        // poll_times++;
    }
    if(res!=TEE_SUCCESS){
        DMSG("poll subsignal pta io helper failed");
        return res;
    }

    // test only
    // record_benchmark_time_wrapper(exec_env,3);

    /**
     * Get data stage
     */
    param_types=TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                TEE_PARAM_TYPE_NONE,
                                TEE_PARAM_TYPE_NONE);
    params[0].value.a=periph_no;
    params[0].value.b=sub_signo;
    // set buf and len
    params[1].memref.buffer=buf;
    params[1].memref.size=*buf_len;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_GET_DATA,param_types,params);
    // return poll_times; // test only
    if(res==TEE_ERROR_SHORT_BUFFER){
        // get buf len needed
        // modified buf len to suggest len needed
        *buf_len=params[1].memref.size;
        return TEE_ERROR_SHORT_BUFFER;
    }else if(res!=TEE_SUCCESS){
        // error get req result
        return TEE_ERROR_NO_DATA; 
    }
    // success get req result, but req falied
    // req return value is params[0].value.a
    if(((int)params[0].value.a)<0){
        DMSG("get io req result success, but io req failed with %d",(int)params[0].value.a);
        return params[0].value.a;
    }
    // success get req result, req success
    // return value is ret, usually suggest real data len we get
    return (int)params[0].value.a;
}