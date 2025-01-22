/**
 * @brief native lib for gpio
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <stdio.h>
#include <stdlib.h>

#include "gpio_lib.h"
#include <pta_gpio.h>
#include <tee_internal_api.h>

static TEE_TASessionHandle sess = TEE_HANDLE_NULL;

static TEE_Result invoke_gpio_pta(uint32_t cmd_id, uint32_t param_types,
                                       TEE_Param params[TEE_NUM_PARAMS])
{
    const TEE_UUID gpio_uuid=PTA_GPIO_SERVICE_UUID; 
    TEE_Result res = TEE_ERROR_GENERIC;
    // load existed session
    if(sess == TEE_HANDLE_NULL){
        // open session
        res=TEE_OpenTASession(&gpio_uuid,TEE_TIMEOUT_INFINITE,0,NULL,&sess,NULL);
        // return error
        if(res!=TEE_SUCCESS) {
            EMSG("The session with the gpio service cannot be established. Error: %x", res);
            return res;
        }
    }
    return TEE_InvokeTACommand(sess,TEE_TIMEOUT_INFINITE,cmd_id,param_types,params,NULL);
}

/**
 * Return TEE_Result as int, 0 is TEE_SUCCESS
 */
static int gpio_init_wrapper(wasm_exec_env_t exec_env, int pin, int mode)
{
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    if (pin < 0)
    {
        EMSG("Invalid pin num");
        return TEE_ERROR_BAD_PARAMETERS;
    }
    if (mode < 0)
    {
        EMSG("Invalid Function Code");
        return TEE_ERROR_BAD_PARAMETERS;
    }

    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a = (uint32_t)pin;
    params[0].value.b = (uint32_t)mode;
    res = invoke_gpio_pta(PTA_GPIO_CMD_INIT,param_types,params);
    return res;
}

/**
 * Return TEE_Result as int, 0 is TEE_SUCCESS
 */
static int gpio_set_wrapper(wasm_exec_env_t exec_env, int pin, int level)
{
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    if (pin < 0)
    {
        EMSG("Invalid Pin Num");
        return TEE_ERROR_BAD_PARAMETERS;
    }
    // val should >= 0, 0 is low level, >0 is high level 
    if (level < 0)
    {
        EMSG("Invalid Pin Value");
        return TEE_ERROR_BAD_PARAMETERS;
    }

    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a = (uint32_t)pin;
    params[0].value.b = (uint32_t)level;
    res = invoke_gpio_pta(PTA_GPIO_CMD_SET,param_types,params);
    return res;
}

/**
 * Return is pin level as int
 * 0 is low
 * 1 is high
 * -1 is error
 */
static int gpio_get_wrapper(wasm_exec_env_t exec_env, int pin)
{
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    if (pin < 0)
    {
        EMSG("Invalid Pin Num");
        return TEE_ERROR_BAD_PARAMETERS;
    }
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_OUTPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a = (uint32_t)pin;
    res = invoke_gpio_pta(PTA_GPIO_CMD_GET,param_types,params);
    if(res) return -1;
    if(params[1].value.a) return 1;
    else return 0;
}

static int test_gpio_overhead_wrapper(wasm_exec_env_t exec_env){
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    res = invoke_gpio_pta(PTA_GPIO_CMD_TEST_OVERHEAD,param_types,params);
    return 0;
}

/**
 * native sleep
 * self-spinning
 */
static int native_sleep_wrapper(wasm_exec_env_t exec_env,int ms){
    TEE_Time start,now;
    if(ms<0){
        return 0;
    }
    TEE_GetSystemTime(&start);
    do{
        TEE_GetSystemTime(&now);
    }while((now.seconds-start.seconds)*1000+(now.millis-start.millis)<ms);
    return 0;
}

static int native_wait_wrapper(wasm_exec_env_t exec_env,int ms){
    return TEE_Wait(ms);
}

/* clang-format off */
#define REG_NATIVE_FUNC(func_name, signature) \
    { #func_name, func_name##_wrapper, signature, NULL }

static NativeSymbol native_symbols[] = {
    REG_NATIVE_FUNC(test_gpio_overhead,"()i"),
    REG_NATIVE_FUNC(gpio_init, "(ii)i"),
    REG_NATIVE_FUNC(gpio_set, "(ii)i"),
    REG_NATIVE_FUNC(gpio_get, "(i)i"),
    REG_NATIVE_FUNC(native_sleep,"(i)i"),
    REG_NATIVE_FUNC(native_wait,"(i)i"),
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
    printf("%s in gpio_lib.c called\n", __func__);
    return 0;
}

void deinit_native_lib()
{
    printf("%s in gpio_lib.c called\n", __func__);
}
