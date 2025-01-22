/**
 * @brief pta gpio service
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */

#include <periph/gpio.h>
#include <kernel/pseudo_ta.h>
#include <kernel/tee_time.h>
#include <trace.h>

#include "pta_gpio.h"

#define GPIO_PTA_NAME   "gpio_service.pta"

#define teetime_to_micro(t) \
    (uint64_t)t.seconds * 1000 * 1000 + (uint64_t)t.micros

/*
 * GPIO INIT
 * [in] value[0].a: gpio pin number
 * [in] value[0].b: gpio mode 
 */
static TEE_Result pta_gpio_init(uint32_t param_types,
				  TEE_Param params[TEE_NUM_PARAMS]){
    gpio_t pin = 0;
    gpio_mode_t mode=GPIO_IN;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                            TEE_PARAM_TYPE_NONE,
                            TEE_PARAM_TYPE_NONE,
                            TEE_PARAM_TYPE_NONE
                            );
    if(exp_param_types != param_types){
        EMSG("Invalid Param types");
        return TEE_ERROR_BAD_PARAMETERS;
    }

    pin =(gpio_t)params[0].value.a;
    if(params[0].value.b>GPIO_OD_PU){
        EMSG("Invalid gpio mode");
        return TEE_ERROR_BAD_PARAMETERS;
    }
    mode=(gpio_mode_t)params[0].value.b;
    
    if(gpio_init(pin,mode)==0){
        return TEE_SUCCESS;
    }else{
        return TEE_ERROR_BAD_PARAMETERS;
    }
}


/*
 * GPIO SET
 * [in] value[0].a: gpio pin number
 * [in] value[0].b: value set to pin
 */
static TEE_Result pta_gpio_set(uint32_t param_types,
			       TEE_Param params[TEE_NUM_PARAMS]){
    gpio_t pin=0;
    gpio_level_t level=GPIO_LOW_LEVEL;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	if (exp_param_types != param_types) {
		EMSG("Invalid Param types");
		return TEE_ERROR_BAD_PARAMETERS;
	}

    pin = (gpio_t)params[0].value.a;
    level=(gpio_level_t)!!params[0].value.b;
    if(gpio_set(pin,level)==0){
        DMSG("set gpio(%d) level(%d)", pin,level);
        return TEE_SUCCESS;
    }else{
        return TEE_ERROR_BAD_PARAMETERS;
    }
}

/*
 * GPIO GET
 * [in] value[0].a: gpio pin number
 * [out] value[1].a: value get from pin
 */
static TEE_Result pta_gpio_get(uint32_t param_types,
			       TEE_Param params[TEE_NUM_PARAMS]){
    gpio_t pin = 0;
    int level=-1;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
						   TEE_PARAM_TYPE_VALUE_OUTPUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);
    if (exp_param_types != param_types) {
		EMSG("Invalid Param types");
		return TEE_ERROR_BAD_PARAMETERS;
	}
    pin = (gpio_t)params[0].value.a;
    level=gpio_get(pin);
    if(level<0){
        return TEE_ERROR_BAD_PARAMETERS;
    }else{
        params[1].value.a=(uint32_t)level;
        DMSG("get gpio(%d) level(%d)", pin,level);
        return TEE_SUCCESS;
    }
}

static TEE_Result pta_gpio_test_overhead(uint32_t param_types,
			       TEE_Param __maybe_unused params[TEE_NUM_PARAMS]){
    TEE_Time start_time,end_time;
    uint64_t start,end;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);
    if (exp_param_types != param_types) {
		EMSG("Invalid Param types");
		return TEE_ERROR_BAD_PARAMETERS;
	}

    if (__builtin_expect(tee_time_get_sys_time(&start_time) != TEE_SUCCESS,0)){
        return TEE_ERROR_GENERIC;
    }
    gpio_init(23,GPIO_OUT);
    if (__builtin_expect(tee_time_get_sys_time(&end_time) != TEE_SUCCESS,0)){
        return TEE_ERROR_GENERIC;
    }
    start=teetime_to_micro(start_time);
    end=teetime_to_micro(end_time);
    EMSG("gpio_init GPIO_OUT interval %lu.",end-start);

    if (__builtin_expect(tee_time_get_sys_time(&start_time) != TEE_SUCCESS,0)){
        return TEE_ERROR_GENERIC;
    }
    gpio_set(23,GPIO_HIGH_LEVEL);
    if (__builtin_expect(tee_time_get_sys_time(&end_time) != TEE_SUCCESS,0)){
        return TEE_ERROR_GENERIC;
    }
    EMSG("gpio_set GPIO_HIGH_LEVEL interval %lu.",end-start);

    if (__builtin_expect(tee_time_get_sys_time(&start_time) != TEE_SUCCESS,0)){
        return TEE_ERROR_GENERIC;
    }
    gpio_set(23,GPIO_LOW_LEVEL);
    if (__builtin_expect(tee_time_get_sys_time(&end_time) != TEE_SUCCESS,0)){
        return TEE_ERROR_GENERIC;
    }
    EMSG("gpio_set GPIO_LOW_LEVEL interval %lu.",end-start);

    if (__builtin_expect(tee_time_get_sys_time(&start_time) != TEE_SUCCESS,0)){
        return TEE_ERROR_GENERIC;
    }
    gpio_init(23,GPIO_IN);
    if (__builtin_expect(tee_time_get_sys_time(&end_time) != TEE_SUCCESS,0)){
        return TEE_ERROR_GENERIC;
    }
    start=teetime_to_micro(start_time);
    end=teetime_to_micro(end_time);
    EMSG("gpio_init GPIO_IN interval %lu.",end-start);

    if (__builtin_expect(tee_time_get_sys_time(&start_time) != TEE_SUCCESS,0)){
        return TEE_ERROR_GENERIC;
    }
    gpio_get(23);
    if (__builtin_expect(tee_time_get_sys_time(&end_time) != TEE_SUCCESS,0)){
        return TEE_ERROR_GENERIC;
    }
    start=teetime_to_micro(start_time);
    end=teetime_to_micro(end_time);
    EMSG("gpio_get interval %lu.",end-start);

    if (__builtin_expect(tee_time_get_sys_time(&start_time) != TEE_SUCCESS,0)){
        return TEE_ERROR_GENERIC;
    }
    EMSG("handled at %lu",start);
    if (__builtin_expect(tee_time_get_sys_time(&end_time) != TEE_SUCCESS,0)){
        return TEE_ERROR_GENERIC;
    }
    start=teetime_to_micro(start_time);
    end=teetime_to_micro(end_time);
    EMSG("EMSG output systime overhead %lu.",end-start);

    return TEE_SUCCESS;
}

static TEE_Result create(void)
{
    DMSG("has been called");
	return TEE_SUCCESS;
}

static TEE_Result invoke_command(void __maybe_unused *session_context,
				 uint32_t cmd_id,
				 uint32_t param_types,
				 TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res = TEE_SUCCESS;
    DMSG("command entry point[%d] for \"%s\"",cmd_id,GPIO_PTA_NAME);
    switch (cmd_id)
    {
    case PTA_GPIO_CMD_INIT:
        res=pta_gpio_init(param_types,params);
        break;
    case PTA_GPIO_CMD_SET:
        res=pta_gpio_set(param_types,params);
        break;
    case PTA_GPIO_CMD_GET:
        res=pta_gpio_get(param_types,params);
        break;
    case PTA_GPIO_CMD_TEST_OVERHEAD:
        res=pta_gpio_test_overhead(param_types,params);
        break;
    default:
        EMSG("cmd: %d not supported %s", cmd_id, GPIO_PTA_NAME);
        res = TEE_ERROR_NOT_SUPPORTED;
        break;
    }
    return res;
}

pseudo_ta_register(.uuid = PTA_GPIO_SERVICE_UUID,
		   .name = GPIO_PTA_NAME,
		   .flags = PTA_DEFAULT_FLAGS | TA_FLAG_CONCURRENT,
           .create_entry_point = create,
		   .invoke_command_entry_point = invoke_command);