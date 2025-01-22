/**
 * @brief pta dht service
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <periph/gpio.h>
#include <drivers/dht/dht.h>
#include <kernel/pseudo_ta.h>
#include <kernel/tee_time.h>
#include "pta_dht.h"

#define  DHT_PTA_NAME   "dht_service.pta"

#define teetime_to_micro(t) \
    (uint64_t)t.seconds * 1000 * 1000 + (uint64_t)t.micros

/**
 * DHT INIT 
 * [in] value[0].a: gpio pin number
 * [in] value[0].b: dht type, see dht_type_t
 */
static TEE_Result pta_dht_init(uint32_t param_types,
				  TEE_Param params[TEE_NUM_PARAMS]){
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_param_types != param_types){
        EMSG("Invalid Param types");
        return TEE_ERROR_BAD_PARAMETERS;
    }
    dht_params_t dev_params={
        .pin=(gpio_t)params[0].value.a,
        .type=(dht_type_t)params[0].value.b,
        .in_mode=GPIO_IN_PU,
    };
    dht_t dev={
        .params=dev_params,
    };
    return dht_init(&dev,&dev_params);
}

/**
 * DHT READ
 * [in]     value[0].a: gpio pin number
 * [in]     value[0].b: dht type, see dht_type_t
 * [out]    value[1].a: temperature
 * [out]    value[1].b: humidity
 */
static TEE_Result pta_dht_read(uint32_t param_types,
				  TEE_Param params[TEE_NUM_PARAMS]){
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_VALUE_OUTPUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    int16_t temp=0;
    int16_t hum=0;
    TEE_Result res;
    if(exp_param_types != param_types){
        EMSG("Invalid Param types");
        return TEE_ERROR_BAD_PARAMETERS;
    }
    dht_params_t dev_params={
        .pin=(gpio_t)params[0].value.a,
        .type=(dht_type_t)params[0].value.b,
        .in_mode=GPIO_IN_PU,
    };
    dht_t dev={
        .params=dev_params,
    };
    res=dht_read(&dev,&temp,&hum);
    params[1].value.a=(uint32_t)temp;
    params[1].value.b=(uint32_t)hum;
    return res;
}

static TEE_Result pta_test_get_sys_time_overhead(uint32_t param_types,
				  TEE_Param __maybe_unused params[TEE_NUM_PARAMS]){
    TEE_Time start_time,end_time;
    uint64_t start,end;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_param_types != param_types){
        EMSG("Invalid Param types");
        return TEE_ERROR_BAD_PARAMETERS;
    }
    if (__builtin_expect(tee_time_get_sys_time(&start_time) != TEE_SUCCESS,0)){
        return TEE_ERROR_GENERIC;
    }
    start=teetime_to_micro(start_time);
    if (__builtin_expect(tee_time_get_sys_time(&start_time) != TEE_SUCCESS,0)){
        return TEE_ERROR_GENERIC;
    }
    EMSG("handle at %lu",start);
    if (__builtin_expect(tee_time_get_sys_time(&end_time) != TEE_SUCCESS,0)){
        return TEE_ERROR_GENERIC;
    }
    start=teetime_to_micro(start_time);
    end=teetime_to_micro(end_time);
    EMSG("EMSG output systime interval %lu.",end-start);
    return TEE_SUCCESS;
}


static TEE_Result create(void)
{
    DMSG("has been called");
	return TEE_SUCCESS;
}


static TEE_Result invoke_command(void *session_context,
				 uint32_t cmd_id,
				 uint32_t param_types,
				 TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res = TEE_SUCCESS;
    DMSG("command entry point[%d] for \"%s\"",cmd_id,DHT_PTA_NAME);
    switch (cmd_id)
    {
    case PTA_CMD_DHT_INIT:
        res=pta_dht_init(param_types,params);
        break;
    case PTA_CMD_DHT_READ:
        res=pta_dht_read(param_types,params);
        break;
    case PTA_CMD_TEST_GET_SYS_TIME_OVERHEAD:
        res=pta_test_get_sys_time_overhead(param_types,params);
        break;
    default:
        EMSG("cmd: %d Not supported %s\n", cmd_id, DHT_PTA_NAME);
        res = TEE_ERROR_NOT_SUPPORTED;
        break;
    }
    return res;
}

pseudo_ta_register(.uuid = PTA_DHT_SERVICE_UUID,
            .name = DHT_PTA_NAME,
            .flags = PTA_DEFAULT_FLAGS | TA_FLAG_CONCURRENT,
            .create_entry_point = create,
            .invoke_command_entry_point = invoke_command);