/**
 *  Author: Quakso
 *  Email: XXXXX@XXXXXX
 *  Date: 2024-11-03
 *  Version: 1.0
 *  Description: rpi3 gpio lib, talk with pta
 */

#include <stdio.h>
#include <stdlib.h>

#include <tee_internal_api.h>
#include "ta_com_lib.h"

#define TA_DATA_SHARING_UUID \
	{ 0x9611ccf5, 0xb79b, 0xad65, \
		{ 0xa8, 0x0d, 0x55, 0x20, 0xbb, 0x88, 0x7b, 0xe5} }

#define TA_DATA_SHARING_CMD_GET_STR_DATA		0

#define TEST_STR_DATA_LEN   19

static TEE_TASessionHandle sess = TEE_HANDLE_NULL;

static TEE_Result invoke_ta(uint32_t cmd_id, uint32_t param_types,
                                       TEE_Param params[TEE_NUM_PARAMS])
{
    const TEE_UUID data_sharing_uuid=TA_DATA_SHARING_UUID;

    TEE_Result res = TEE_ERROR_GENERIC;
    // load existed session
    if(sess == TEE_HANDLE_NULL){
        // open session
        res=TEE_OpenTASession(&data_sharing_uuid,TEE_TIMEOUT_INFINITE,0,NULL,&sess,NULL);
        // return error
        if(res!=TEE_SUCCESS) {
            EMSG("The session with sharing data ta cannot be established. Error: %x", res);
            return res;
        }
    }
    return TEE_InvokeTACommand(sess,TEE_TIMEOUT_INFINITE,cmd_id,param_types,params,NULL);
}

static int get_str_data_wrapper(wasm_exec_env_t exec_env, uint32_t dst_str_addr, uint32_t dst_str_len)
{
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;

    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(!wasm_runtime_validate_app_addr(inst,(uint64_t)dst_str_addr,(uint64_t)dst_str_len)){
        return -1;
    }
    char * result = wasm_runtime_addr_app_to_native(inst,dst_str_addr);
    DMSG("call another ta transmit to %p",(void *) result);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);

    params[0].memref.buffer=result;
    params[0].memref.size=dst_str_len;
    // params[0].memref.buffer=malloc(TEST_STR_DATA_LEN);
    // params[0].memref.size=TEST_STR_DATA_LEN;

    res = invoke_ta(TA_DATA_SHARING_CMD_GET_STR_DATA,param_types,params);
    if (res!=TEE_SUCCESS){
        return -1;
    }
    DMSG("Get data: %s\n",params[0].memref.buffer);
    return 0;
}


/* clang-format off */
#define REG_NATIVE_FUNC(func_name, signature) \
    { #func_name, func_name##_wrapper, signature, NULL }

static NativeSymbol native_symbols[] = {
    REG_NATIVE_FUNC(get_str_data, "(ii)i"),
};
/* clang-format on */

uint32_t
get_native_lib(char **p_module_name, NativeSymbol **p_native_symbols)
{
    *p_module_name = "env";
    *p_native_symbols = native_symbols;
    return sizeof(native_symbols) / sizeof(NativeSymbol);
}

int init_native_lib()
{
    printf("%s in ta_com_lib.c called\n", __func__);
    return 0;
}

void deinit_native_lib()
{
    printf("%s in ta_com_lib.c called\n", __func__);
}
