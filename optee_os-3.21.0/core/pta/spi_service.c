/**
 * @brief pta spi service
 * @author  XXXXXXXX <XXXXX@XXXXXX>
 */

#include<periph/spi.h>
#include<kernel/pseudo_ta.h>
#include<kernel/tee_time.h>

#include "pta_spi.h"

#define SPI_PTA_NAME    "spi_service.pta"

static TEE_Result pta_spi_transfer_byte(uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS]){
    uint8_t out;
    uint8_t in;
    TEE_Result res;
    spi_t spi;
    spi_cs_t spi_cs;
    bool cont=false;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                               TEE_PARAM_TYPE_VALUE_INPUT,
                                               TEE_PARAM_TYPE_VALUE_INPUT,
                                               TEE_PARAM_TYPE_VALUE_INPUT);
    if(param_types!=exp_param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    if(params[1].value.a>(uint32_t)SPI_MODE_3||params[1].value.b>(uint32_t)SPI_CLK_40MHZ){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    if(params[0].value.b>0){
        cont=true;
    }
    
    spi=(spi_t)((params[0].value.a>>16) & 0xFFFF);
    spi_cs=(spi_cs_t)(params[0].value.a & 0xFFFF);
    DMSG("spi %d, cs %d, mode %d, clk %d, need byte out %d, need byte in %d, byte out %02x",
            spi,spi_cs,(spi_mode_t)params[1].value.a,(spi_clk_t)params[1].value.b,
            params[2].value.a, params[2].value.b,(uint8_t)params[3].value.a);
    res=spi_acquire(spi,spi_cs,(spi_mode_t)params[1].value.a,(spi_clk_t)params[1].value.b);
    if(res!=TEE_SUCCESS){
        return res;
    }
    out=(uint8_t)params[3].value.a;
    if(params[2].value.a>0&&params[2].value.b>0){
        res=spi_transfer_byte(spi,spi_cs,cont,out,&in);
        if(res==TEE_SUCCESS){
            params[0].value.a=(uint32_t)in;
        }
    }else if(params[2].value.a>0&&params[2].value.b==0){
        res=spi_transfer_byte(spi,spi_cs,cont,out,NULL);
    }else if(params[2].value.a==0&&params[2].value.b>0){
        res=spi_transfer_byte(spi,spi_cs,cont,0,&in);
        if(res==TEE_SUCCESS){
            params[0].value.a=(uint32_t)in;
        }
    }
    spi_release(spi);
    DMSG("spi transfer byte res %x",res);
    return res;
}

static TEE_Result pta_spi_transfer_bytes(uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res;
    spi_t spi;
    spi_cs_t spi_cs;
    bool cont=false;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                               TEE_PARAM_TYPE_VALUE_INPUT,
                                               TEE_PARAM_TYPE_MEMREF_INPUT,
                                               TEE_PARAM_TYPE_MEMREF_OUTPUT);
    if(param_types!=exp_param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    if(params[1].value.a>(uint32_t)SPI_MODE_3||params[1].value.b>(uint32_t)SPI_CLK_40MHZ){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    // same len or out_len=0, in_len>0 or out_len>0, in_len=0
    if(params[2].memref.size>0&&params[3].memref.size>0&&params[2].memref.size!=params[3].memref.size){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    if(params[0].value.b>0){
        cont=true;
    }
    spi=(spi_t)((params[0].value.a>>16) & 0xFFFF);
    spi_cs=(spi_cs_t)(params[0].value.a & 0xFFFF);
    DMSG("spi %d, cs %d, mode %d, clk %d, src len %d, dst len %d",
            spi,spi_cs,(spi_mode_t)params[1].value.a,(spi_clk_t)params[1].value.b,
            params[2].memref.size, params[3].memref.size);
    res=spi_acquire(spi,spi_cs,(spi_mode_t)params[1].value.a,(spi_clk_t)params[1].value.b);
    if(res!=TEE_SUCCESS){
        return res;
    }
    if(params[2].memref.size==params[3].memref.size&&params[2].memref.size>0){
        // exchange same len
        DMSG("exchange same len");
        res=spi_transfer_bytes(spi,spi_cs,cont,params[2].memref.buffer,params[3].memref.buffer,params[2].memref.size);
    }else if(params[2].memref.size>0&&params[3].memref.size==0){
        // only write
        DMSG("only write for %d",params[2].memref.size);    
        res=spi_transfer_bytes(spi,spi_cs,cont,params[2].memref.buffer,NULL,params[2].memref.size);
    }else if(params[3].memref.size>0&&params[2].memref.size==0){
        // only read
        DMSG("only read for %d",params[3].memref.size);
        res=spi_transfer_bytes(spi,spi_cs,cont,NULL,params[3].memref.buffer,params[3].memref.size);
    }
    spi_release(spi);
    DMSG("spi transfer bytes res %x",res);
    return res;
}

static TEE_Result pta_spi_transfer_diff_bytes(uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res;
    spi_t spi;
    spi_cs_t spi_cs;
    bool cont=false;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                               TEE_PARAM_TYPE_VALUE_INPUT,
                                               TEE_PARAM_TYPE_MEMREF_INPUT,
                                               TEE_PARAM_TYPE_MEMREF_OUTPUT);
    if(param_types!=exp_param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    if(params[1].value.a>(uint32_t)SPI_MODE_3||params[1].value.b>(uint32_t)SPI_CLK_40MHZ){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    if(params[0].value.b>0){
        cont=true;
    }
    spi=(spi_t)((params[0].value.a>>16) & 0xFFFF);
    spi_cs=(spi_cs_t)(params[0].value.a & 0xFFFF);
    DMSG("spi %d, cs %d, mode %d, clk %d, src len %d, dst len %d",
            spi,spi_cs,(spi_mode_t)params[1].value.a,(spi_clk_t)params[1].value.b,
            params[2].memref.size, params[3].memref.size);
    res=spi_acquire(spi,spi_cs,(spi_mode_t)params[1].value.a,(spi_clk_t)params[1].value.b);
    if(res!=TEE_SUCCESS){
        return res;
    }
    res=spi_transfer_diff_bytes(spi,spi_cs,cont,
                                params[2].memref.buffer,params[3].memref.buffer,
                                params[2].memref.size,params[3].memref.size);
    spi_release(spi);
    DMSG("spi transfer bytes res %x",res);
    return res;
}

static TEE_Result create(void)
{
    DMSG("has been called");
    return TEE_SUCCESS;
}

static TEE_Result invoke_command(void __maybe_unused * session_context,
                uint32_t cmd_id,
                uint32_t param_types,
                TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res = TEE_SUCCESS;
    switch(cmd_id){
        case PTA_CMD_SPI_TRANSFER_BYTE:
            res=pta_spi_transfer_byte(param_types,params);
            break;
        case PTA_CMD_SPI_TRANSFER_BYTES:
            res=pta_spi_transfer_bytes(param_types,params);
            break;
        case PTA_CMD_SPI_TRANSFER_DIFF_BYTES:
            res=pta_spi_transfer_diff_bytes(param_types,params);
            break;
        default:
            EMSG("cmd: %d not supported %s",cmd_id,SPI_PTA_NAME);
            res=TEE_ERROR_NOT_SUPPORTED;
            break;
    }
    return res;
}

pseudo_ta_register(.uuid=PTA_SPI_SERVICE_UUID,
                .name = SPI_PTA_NAME,
                .flags = PTA_DEFAULT_FLAGS | TA_FLAG_CONCURRENT,
                .create_entry_point = create,
                .invoke_command_entry_point = invoke_command);