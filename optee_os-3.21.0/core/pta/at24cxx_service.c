/**
 * @brief pta at24cxx service
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */

#include <periph/i2c.h>
#include <drivers/at24cxx/at24cxx.h>
#include <kernel/pseudo_ta.h>
#include "pta_at24cxx.h"


#define AT24CXX_PTA_NAME  "at24cxx_service.pta"

static TEE_Result pta_at24cxx_read(uint32_t param_types,
                 TEE_Param params[TEE_NUM_PARAMS]){
    at24cxx_t dev;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    dev.dev=(i2c_t)params[0].value.a;
    dev.addr=(uint8_t)params[0].value.b;
    return at24cxx_read(&dev, (uint16_t)params[1].value.a, params[2].memref.buffer, params[2].memref.size);
}

static TEE_Result pta_at24cxx_write(uint32_t param_types,
                 TEE_Param params[TEE_NUM_PARAMS]){
    at24cxx_t dev;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_MEMREF_INPUT,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    dev.dev=(i2c_t)params[0].value.a;
    dev.addr=(uint8_t)params[0].value.b;
    return at24cxx_write(&dev, (uint16_t)params[1].value.a, params[2].memref.buffer, params[2].memref.size);
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
    DMSG("command entry point[%d] for \"%s\"",cmd_id,AT24CXX_PTA_NAME);
    switch (cmd_id)
    {
    case PTA_CMD_AT24CXX_READ:
        res = pta_at24cxx_read(param_types, params);
        break;
    case PTA_CMD_AT24CXX_WRITE:
        res = pta_at24cxx_write(param_types, params);
        break;
    default:
        res = TEE_ERROR_BAD_PARAMETERS;
        break;
    }
    return res;
}

pseudo_ta_register(.uuid = PTA_AT24CXX_SERVICE_UUID,
            .name = AT24CXX_PTA_NAME, 
            .flags = PTA_DEFAULT_FLAGS | TA_FLAG_CONCURRENT,
            .create_entry_point = create,
            .invoke_command_entry_point = invoke_command);