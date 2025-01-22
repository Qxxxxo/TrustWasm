/**
 * @brief pta tel0157 service
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <periph/i2c.h>
#include <drivers/gnss/tel0157.h>
#include <kernel/pseudo_ta.h>
#include "pta_tel0157.h"

#define TEL0157_PTA_NAME "tel0157_service.pta"

static TEE_Result pta_tel0157_init(uint32_t param_types,
				 TEE_Param params[TEE_NUM_PARAMS]){
    tel0157_t dev;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    dev.i2c=(i2c_t)params[0].value.a;
    dev.addr=(uint8_t)params[0].value.b;
    dev.gnss_mode=(uint8_t)params[1].value.a;
    dev.rgb=(uint8_t)params[1].value.b;
    return tel0157_init(&dev);
}

static TEE_Result pta_tel0157_deinit(uint32_t param_types,
				 TEE_Param params[TEE_NUM_PARAMS]){
    tel0157_t dev;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    dev.i2c=(i2c_t)params[0].value.a;
    dev.addr=(uint8_t)params[0].value.b;
    return tel0157_deinit(&dev);
}

static TEE_Result pta_tel0157_get_utc_time(uint32_t param_types,
				 TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res;
    tel0157_t dev;
    // tel0157_time_t utc;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    if(params[1].memref.size!=sizeof(tel0157_time_t)){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    dev.i2c=(i2c_t)params[0].value.a;
    dev.addr=(uint8_t)params[0].value.b;
    res=tel0157_get_utc_time(&dev,(tel0157_time_t*)params[1].memref.buffer);
    if(res!=TEE_SUCCESS){
        return res;
    }
    // params[0].value.a=(uint32_t)utc.year;
    // params[0].value.b=(uint32_t)utc.month;
    // params[1].value.a=(uint32_t)utc.date;
    // params[1].value.b=(uint32_t)utc.hour;
    // params[2].value.a=(uint32_t)utc.minute;
    // params[2].value.b=(uint32_t)utc.second;
    return TEE_SUCCESS;
}

static TEE_Result pta_tel0157_get_lon(uint32_t param_types,
				 TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res;
    tel0157_t dev;
    // tel0157_lon_lat_t lon_lat;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    if(params[1].memref.size!=sizeof(tel0157_lon_lat_t)){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    dev.i2c=(i2c_t)params[0].value.a;
    dev.addr=(uint8_t)params[0].value.b;
    res=tel0157_get_lon(&dev,(tel0157_lon_lat_t*)params[1].memref.buffer);
    if(res!=TEE_SUCCESS){
        return res;
    }
    // params[0].value.a=(uint32_t)lon_lat.lonDD;
    // params[0].value.b=(uint32_t)lon_lat.lonMM;
    // params[1].value.a=(uint32_t)lon_lat.lonMMMMM;
    // params[1].value.b=(uint32_t)lon_lat.lonDirection;
    return TEE_SUCCESS;
}

static TEE_Result pta_tel0157_get_lat(uint32_t param_types,
				 TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res;
    tel0157_t dev;
    // tel0157_lon_lat_t lon_lat;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    if(params[1].memref.size!=sizeof(tel0157_lon_lat_t)){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    dev.i2c=(i2c_t)params[0].value.a;
    dev.addr=(uint8_t)params[0].value.b;
    res=tel0157_get_lat(&dev,(tel0157_lon_lat_t*)params[1].memref.buffer);
    if(res!=TEE_SUCCESS){
        return res;
    }
    // params[0].value.a=(uint32_t)lon_lat.latDD;
    // params[0].value.b=(uint32_t)lon_lat.latMM;
    // params[1].value.a=(uint32_t)lon_lat.latMMMMM;
    // params[1].value.b=(uint32_t)lon_lat.latDirection;
    return TEE_SUCCESS;
}

static TEE_Result pta_tel0157_get_alt(uint32_t param_types,
				 TEE_Param params[TEE_NUM_PARAMS]){
    tel0157_t dev;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    // need 3 bytes
    if(params[1].memref.size!=3){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    dev.i2c=(i2c_t)params[0].value.a;
    dev.addr=(uint8_t)params[0].value.b;
    return tel0157_get_alt(&dev,params[1].memref.buffer);
}

static TEE_Result pta_tel0157_get_sog(uint32_t param_types,
				 TEE_Param params[TEE_NUM_PARAMS]){
    tel0157_t dev;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    if(params[1].memref.size!=3){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    dev.i2c=(i2c_t)params[0].value.a;
    dev.addr=(uint8_t)params[0].value.b;
    return tel0157_get_sog(&dev,params[1].memref.buffer);
}

static TEE_Result pta_tel0157_get_cog(uint32_t param_types,
				 TEE_Param params[TEE_NUM_PARAMS]){
    tel0157_t dev;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    if(params[1].memref.size!=3){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    dev.i2c=(i2c_t)params[0].value.a;
    dev.addr=(uint8_t)params[0].value.b;
    return tel0157_get_cog(&dev,params[1].memref.buffer);
}

static TEE_Result pta_tel0157_get_num_sat_used(uint32_t param_types,
				 TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res;
    tel0157_t dev;
    uint8_t num;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    dev.i2c=(i2c_t)params[0].value.a;
    dev.addr=(uint8_t)params[0].value.b;
    res=tel0157_get_num_sat_used(&dev,&num);
    if(res!=TEE_SUCCESS){
        return res;
    }
    params[0].value.a=(uint32_t)num;
    return TEE_SUCCESS;
}

static TEE_Result pta_tel0157_get_gnss_mode(uint32_t param_types,
				 TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res;
    tel0157_t dev;
    uint8_t mode;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    dev.i2c=(i2c_t)params[0].value.a;
    dev.addr=(uint8_t)params[0].value.b;
    res=tel0157_get_gnss_mode(&dev,&mode);
    if(res!=TEE_SUCCESS){
        return res;
    }
    params[0].value.a=(uint32_t)mode;
    return TEE_SUCCESS;
}

static TEE_Result pta_tel0157_get_gnss_len(uint32_t param_types,
				 TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res;
    tel0157_t dev;
    uint16_t len;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    dev.i2c=(i2c_t)params[0].value.a;
    dev.addr=(uint8_t)params[0].value.b;
    res=tel0157_get_gnss_len(&dev,&len);
    if(res!=TEE_SUCCESS){
        return res;
    }
    params[0].value.a=(uint32_t)len;
    return TEE_SUCCESS;
}

static TEE_Result pta_tel0157_get_all_gnss(uint32_t param_types,
				 TEE_Param params[TEE_NUM_PARAMS]){
    tel0157_t dev;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    dev.i2c=(i2c_t)params[0].value.a;
    dev.addr=(uint8_t)params[0].value.b;
    return tel0157_get_all_gnss(&dev,(uint8_t*)params[1].memref.buffer,(uint16_t)params[1].memref.size);
}

static TEE_Result create(void){
    DMSG("has been called");
    return TEE_SUCCESS;
}

static TEE_Result invoke_command(void __maybe_unused *session_context,
				 uint32_t cmd_id,
				 uint32_t param_types,
				 TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res = TEE_SUCCESS;
    DMSG("command entry point[%d] for \"%s\"",cmd_id,TEL0157_PTA_NAME);
    switch (cmd_id)
    {
    case PTA_CMD_TEL0157_INIT:
        res=pta_tel0157_init(param_types,params);
        break;
    case PTA_CMD_TEL0157_DEINIT:
        res=pta_tel0157_deinit(param_types,params);
        break;
    case PTA_CMD_TEL0157_GET_UTC_TIME:
        res=pta_tel0157_get_utc_time(param_types,params);
        break;
    case PTA_CMD_TEL0157_GET_LON:
        res=pta_tel0157_get_lon(param_types,params);
        break;
    case PTA_CMD_TEL0157_GET_LAT:
        res=pta_tel0157_get_lat(param_types,params);
        break;
    case PTA_CMD_TEL0157_GET_ALT:
        res=pta_tel0157_get_alt(param_types,params);
        break;
    case PTA_CMD_TEL0157_GET_SOG:
        res=pta_tel0157_get_sog(param_types,params);
        break;
    case PTA_CMD_TEL0157_GET_COG:
        res=pta_tel0157_get_cog(param_types,params);
        break;
    case PTA_CMD_TEL0157_GET_NUM_SAT_USED:
        res=pta_tel0157_get_num_sat_used(param_types,params);
        break;
    case PTA_CMD_TEL0157_GET_GNSS_MODE:
        res=pta_tel0157_get_gnss_mode(param_types,params);
        break;
    case PTA_CMD_TEL0157_GET_GNSS_LEN:
        res=pta_tel0157_get_gnss_len(param_types,params);
        break;
    case PTA_CMD_TEL0157_GET_ALL_GNSS:
        res=pta_tel0157_get_all_gnss(param_types,params);
        break;
    default:
        EMSG("cmd: %d Not supported %s\n", cmd_id, TEL0157_PTA_NAME);
        res = TEE_ERROR_NOT_SUPPORTED;
        break;
    }
    return res;
}

pseudo_ta_register(.uuid = PTA_TEL0157_SERVICE_UUID,
                .name = TEL0157_PTA_NAME,
                .flags = PTA_DEFAULT_FLAGS | TA_FLAG_CONCURRENT,
                .create_entry_point = create,
                .invoke_command_entry_point = invoke_command);