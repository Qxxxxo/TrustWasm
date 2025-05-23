/**
 * @brief pta mpu6050 service
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <periph/i2c.h>
#include <drivers/mpu6050/mpu6050.h>
#include <kernel/pseudo_ta.h>
#include "pta_mpu6050.h"

#define MPU6050_PTA_NAME    "mpu6050_service.pta"

static TEE_Result pta_mpu6050_init(uint32_t param_types,
				 TEE_Param params[TEE_NUM_PARAMS]){
    mpu6050_t dev;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    dev.i2c=(i2c_t)params[0].value.a;
    dev.addr=(uint8_t)params[0].value.b;
    return mpu_init(&dev);
}

static TEE_Result pta_mpu6050_deinit(uint32_t param_types,
				 TEE_Param params[TEE_NUM_PARAMS]){
    mpu6050_t dev;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    dev.i2c=(i2c_t)params[0].value.a;
    dev.addr=(uint8_t)params[0].value.b;
    return mpu_deinit(&dev);
}

static TEE_Result pta_mpu6050_sample(uint32_t param_types,
				 TEE_Param params[TEE_NUM_PARAMS]){
    // TEE_Result res;
    mpu6050_t dev;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    if(params[1].memref.size!=14){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    dev.i2c=(i2c_t)params[0].value.a;
    dev.addr=(uint8_t)params[0].value.b;
    return mpu_sample(&dev,params[1].memref.buffer);
}

static TEE_Result create(void){
    DMSG("has been created");
    return TEE_SUCCESS;
}

static TEE_Result invoke_command(void __maybe_unused *session_context,
				 uint32_t cmd_id,
				 uint32_t param_types,
				 TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res = TEE_SUCCESS;
    DMSG("command entry point[%d] for \"%s\"",cmd_id,MPU6050_PTA_NAME);
    switch(cmd_id)
    {
    case PTA_CMD_MPU6050_INIT:
        res=pta_mpu6050_init(param_types, params);
        break;
    case PTA_CMD_MPU6050_DEINIT:
        res=pta_mpu6050_deinit(param_types, params);
        break;
    case PTA_CMD_MPU6050_SAMPLE:
        res=pta_mpu6050_sample(param_types, params);
        break;
    default:
        EMSG("cmd: %d Not supported %s\n", cmd_id, MPU6050_PTA_NAME);
        res = TEE_ERROR_NOT_SUPPORTED;
        break;
    }
    return res;
}

pseudo_ta_register(.uuid = PTA_MPU6050_SERVICE_UUID,
                .name = MPU6050_PTA_NAME,
                .flags = PTA_DEFAULT_FLAGS | TA_FLAG_CONCURRENT,
                .create_entry_point = create,
                .invoke_command_entry_point = invoke_command);