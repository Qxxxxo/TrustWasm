/**
 * @brief pta bh1750fvi service
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <periph/i2c.h>
#include <drivers/bh1750/bh1750fvi.h>
#include <kernel/pseudo_ta.h>
#include "pta_bh1750fvi.h"

#define BH1750FVI_PTA_NAME  "bh1750fvi_service.pta"

static TEE_Result pta_bh1750fvi_init(uint32_t param_types,
				 TEE_Param params[TEE_NUM_PARAMS]){
    bh1750fvi_t dev;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    dev.i2c=(i2c_t)params[0].value.a;
    dev.addr=(uint8_t)params[0].value.b;
    return bh1750fvi_init(&dev);
}

static TEE_Result pta_bh1750fvi_sample(uint32_t param_types,
				 TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res;
    bh1750fvi_t dev;
    uint16_t lux;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    dev.i2c=(i2c_t)params[0].value.a;
    dev.addr=(uint8_t)params[0].value.b;
    res=bh1750fvi_sample(&dev,&lux);
    if(res!=TEE_SUCCESS){
        return res;
    }
    params[0].value.a=(uint32_t)lux;
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
    DMSG("command entry point[%d] for \"%s\"",cmd_id,BH1750FVI_PTA_NAME);
    switch (cmd_id)
    {
    case PTA_CMD_BH1750FVI_INIT:
        res=pta_bh1750fvi_init(param_types,params);
        break;
    case PTA_CMD_BH1750FVI_SAMPLE:
        res=pta_bh1750fvi_sample(param_types,params);
        break;
    default:
        EMSG("cmd: %d Not supported %s\n", cmd_id, BH1750FVI_PTA_NAME);
        res = TEE_ERROR_NOT_SUPPORTED;
        break;
    }
    return res;
}

pseudo_ta_register(.uuid = PTA_BH1750FVI_SERVICE_UUID,
            .name = BH1750FVI_PTA_NAME,
            .flags = PTA_DEFAULT_FLAGS | TA_FLAG_CONCURRENT,
            .create_entry_point = create,
            .invoke_command_entry_point = invoke_command);

