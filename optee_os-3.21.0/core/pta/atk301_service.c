/**
 * @brief pta atk301 service
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */

#include <periph/i2c.h>
#include <drivers/atk301/atk301.h>
#include <kernel/pseudo_ta.h>
#include "pta_atk301.h"

#define ATK301_PTA_NAME  "atk301_service.pta"

static TEE_Result pta_atk301_init(uint32_t param_types,
                 TEE_Param params[TEE_NUM_PARAMS]){
    atk301_t dev;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    dev.i2c=(i2c_t)params[0].value.a;
    dev.addr=(uint8_t)params[0].value.b;
    return atk301_init(&dev);
}

static TEE_Result pta_atk301_get_fingerprint_image(uint32_t param_types,
                 TEE_Param params[TEE_NUM_PARAMS]){
    atk301_t dev;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    dev.i2c=(i2c_t)params[0].value.a;
    dev.addr=(uint8_t)params[0].value.b;
    return atk301_get_fingerprint_image(&dev);
}

static TEE_Result pta_atk301_upload_fingerprint_image(uint32_t param_types,
                 TEE_Param params[TEE_NUM_PARAMS]){
    atk301_t dev;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    // verify length
    if(params[1].memref.size!=ATK301_FINGERPRINT_IMAGE_DATA_LEN){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    dev.i2c=(i2c_t)params[0].value.a;
    dev.addr=(uint8_t)params[0].value.b;
    return atk301_upload_fingerprint_image(&dev,params[1].memref.buffer);
}

static TEE_Result create(void)
{
    // DMSG("has been called");
    return TEE_SUCCESS;
}

static TEE_Result invoke_command(void __maybe_unused *session_context,
                 uint32_t cmd_id,
                 uint32_t param_types,
                 TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res = TEE_SUCCESS;
    DMSG("command entry point[%d] for \"%s\"",cmd_id,ATK301_PTA_NAME);
    switch (cmd_id)
    {
    case PTA_CMD_ATK301_INIT:
        res = pta_atk301_init(param_types, params);
        break;
    case PTA_CMD_ATK301_GET_FINGERPRINT_IMAGE:
        res = pta_atk301_get_fingerprint_image(param_types, params);
        break;
    case PTA_CMD_ATK301_UPLOAD_FINGERPRINT_IMAGE:
        res = pta_atk301_upload_fingerprint_image(param_types, params);
        break;
    default:
        res = TEE_ERROR_BAD_PARAMETERS;
        break;
    }
    return res;
}

pseudo_ta_register(.uuid = PTA_ATK301_SERVICE_UUID, 
                .name = ATK301_PTA_NAME, 
                .flags = PTA_DEFAULT_FLAGS | TA_FLAG_CONCURRENT,
                .create_entry_point = create, 
                .invoke_command_entry_point = invoke_command);
