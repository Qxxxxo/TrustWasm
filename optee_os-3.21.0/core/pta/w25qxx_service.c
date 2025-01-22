/**
 * @brief pta w25qxx service
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <periph/spi.h>
#include <drivers/w25qxx/w25qxx.h>
#include <kernel/pseudo_ta.h>
#include "pta_w25qxx.h"

#define W25QXX_PTA_NAME "w25qxx_service.pta"

static TEE_Result pta_w25qxx_init(uint32_t param_types,
				 TEE_Param params[TEE_NUM_PARAMS]){
    uint32_t exp_params_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_params_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    return w25qxx_init((spi_t)params[0].value.a,(spi_cs_t)params[0].value.b);
}

static TEE_Result pta_w25qxx_read_id(uint32_t param_types,
				 TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res;
    uint16_t id;
    uint32_t exp_params_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_params_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    res=w25qxx_read_id((spi_t)params[0].value.a,(spi_cs_t)params[0].value.b,&id);
    if(res!=TEE_SUCCESS){
        return res;
    }
    params[0].value.a=(uint32_t)id;
    return TEE_SUCCESS;
}

static TEE_Result pta_w25qxx_read_data(uint32_t param_types,
				 TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res;
    uint32_t exp_params_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_params_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    res=w25qxx_read_data((spi_t)params[0].value.a,(spi_cs_t)params[0].value.b,
                        (uint8_t*)params[2].memref.buffer,params[1].value.a,
                        params[2].memref.size);
    return res;
}

static TEE_Result pta_w25qxx_page_program(uint32_t param_types,
				 TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res;
    uint32_t exp_params_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_MEMREF_INPUT,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_params_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    res=w25qxx_page_program((spi_t)params[0].value.a,(spi_cs_t)params[0].value.b,
                            (uint8_t*)params[2].memref.buffer,params[1].value.a,
                            params[2].memref.size);
    return res;
}

static TEE_Result pta_w25qxx_sector_erase(uint32_t param_types,
				 TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res;
    uint32_t exp_params_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_params_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    res=w25qxx_sector_erase((spi_t)params[0].value.a,(spi_cs_t)params[0].value.b,
                            params[1].value.a);
    return res;
}

static TEE_Result pta_w25qxx_chip_erase(uint32_t param_types,
				 TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res;
    uint32_t exp_params_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_params_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    res=w25qxx_chip_erase((spi_t)params[0].value.a,(spi_cs_t)params[0].value.b);
    return res;
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
    DMSG("command entry point[%d] for \"%s\"",cmd_id,W25QXX_PTA_NAME);
    switch (cmd_id)
    {
    case PTA_CMD_W25QXX_INIT:
        res=pta_w25qxx_init(param_types,params);
        break;
    case PTA_CMD_W25QXX_READ_ID:
        res=pta_w25qxx_read_id(param_types,params);
        break;
    case PTA_CMD_W25QXX_READ_DATA:
        res=pta_w25qxx_read_data(param_types,params);
        break;
    case PTA_CMD_W25QXX_PAGE_PROGRAM:
        res=pta_w25qxx_page_program(param_types,params);
        break;
    case PTA_CMD_W25QXX_SECTOR_ERASE:
        res=pta_w25qxx_sector_erase(param_types,params);
        break;
    case PTA_CMD_W25QXX_CHIP_ERASE:
        res=pta_w25qxx_chip_erase(param_types,params);
        break;
    default:
        EMSG("cmd: %d Not supported %s\n", cmd_id, W25QXX_PTA_NAME);
        res = TEE_ERROR_NOT_SUPPORTED;
        break;
    }
    return res;
}

pseudo_ta_register(.uuid = PTA_W25QXX_SERVICE_UUID,
            .name = W25QXX_PTA_NAME,
            .flags = PTA_DEFAULT_FLAGS | TA_FLAG_CONCURRENT,
            .create_entry_point = create,
            .invoke_command_entry_point = invoke_command);

