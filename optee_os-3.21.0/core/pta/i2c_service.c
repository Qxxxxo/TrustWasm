/**
 * @brief pta i2c service
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */

#include<periph/i2c.h>
#include<kernel/pseudo_ta.h>
#include<kernel/tee_time.h>
#include<string.h>

#include "pta_i2c.h"

#define I2C_PTA_NAME    "i2c_service.pta"

#ifdef OVERHEAD_BENCHMARK
#define I2C_SERVICE_OVERHEAD_BENCHMARK_SIZE    100
#define I2C_SERVICE_OVERHEAD_BENCHMARK_POINTS    2
#define I2C_SERVICE_OVERHEAD_BENCHMARK_TIMES    50
static uint32_t benchmark_idx=0;
static uint64_t benchmark_time[I2C_SERVICE_OVERHEAD_BENCHMARK_SIZE]={0};
#define teetime_to_micro(t) \
    (uint64_t)t.seconds * 1000 * 1000 + (uint64_t)t.micros
#endif

static TEE_Result pta_i2c_read_byte(uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS]){
    uint8_t data; // read byte
    TEE_Result res;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                               TEE_PARAM_TYPE_VALUE_INPUT,
                                               TEE_PARAM_TYPE_NONE,
                                               TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }

    if(i2c_acquire((i2c_t)params[0].value.a)!=TEE_SUCCESS){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    res=i2c_read_byte((i2c_t)params[0].value.a,
                        (uint16_t)params[0].value.b,
                        &data,
                        (uint8_t)params[1].value.a);

    i2c_release((i2c_t)params[0].value.a);
    if(res!=TEE_SUCCESS) return res;
    params[0].value.a=(uint32_t)data;
    return TEE_SUCCESS;
}

static TEE_Result pta_i2c_read_bytes(uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS]){

#ifdef OVERHEAD_BENCHMARK
    TEE_Time t1;
    tee_time_get_sys_time(&t1);
    benchmark_time[benchmark_idx*I2C_SERVICE_OVERHEAD_BENCHMARK_POINTS]=teetime_to_micro(t1);
#endif

    TEE_Result res;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                               TEE_PARAM_TYPE_VALUE_INPUT,
                                               TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                               TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }

    if(i2c_acquire((i2c_t)params[0].value.a)!=TEE_SUCCESS){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    res=i2c_read_bytes((i2c_t)params[0].value.a,
                        (uint16_t)params[0].value.b,
                        (void *)params[2].memref.buffer,
                        (size_t)params[2].memref.size,
                        (uint8_t)params[1].value.a);
    i2c_release((i2c_t)params[0].value.a);
    // DMSG("[i2c service] params[2].memref.buffer[0] %d",((uint8_t*)params[2].memref.buffer)[0]);

#ifdef OVERHEAD_BENCHMARK
    TEE_Time t2;
    tee_time_get_sys_time(&t2);
    benchmark_time[benchmark_idx*I2C_SERVICE_OVERHEAD_BENCHMARK_POINTS+1]=teetime_to_micro(t2);
    if(benchmark_idx<I2C_SERVICE_OVERHEAD_BENCHMARK_TIMES-1){
        benchmark_idx++;
    }
#endif

    return res;
}

static TEE_Result pta_i2c_write_byte(uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                               TEE_PARAM_TYPE_VALUE_INPUT,
                                               TEE_PARAM_TYPE_NONE,
                                               TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }

    if(i2c_acquire((i2c_t)params[0].value.a)!=TEE_SUCCESS){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    res=i2c_write_byte((i2c_t)params[0].value.a,
                        (uint16_t)params[0].value.b,
                        (uint8_t)params[1].value.b,
                        (uint8_t)params[1].value.a);

    i2c_release((i2c_t)params[0].value.a);
    return res;
}

static TEE_Result pta_i2c_write_bytes(uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                               TEE_PARAM_TYPE_VALUE_INPUT,
                                               TEE_PARAM_TYPE_MEMREF_INPUT,
                                               TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }

    if(i2c_acquire((i2c_t)params[0].value.a)!=TEE_SUCCESS){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    res=i2c_write_bytes((i2c_t)params[0].value.a,
                        (uint16_t)params[0].value.b,
                        (void *)params[2].memref.buffer,
                        (size_t)params[2].memref.size,
                        (uint8_t)params[1].value.a);
    i2c_release((i2c_t)params[0].value.a);
    return res;
}


static TEE_Result pta_i2c_read_reg(uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS]){
    uint8_t data; // read byte
    TEE_Result res;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                               TEE_PARAM_TYPE_VALUE_INPUT,
                                               TEE_PARAM_TYPE_NONE,
                                               TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }

    if(i2c_acquire((i2c_t)params[0].value.a)!=TEE_SUCCESS){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    res=i2c_read_reg((i2c_t)params[0].value.a,
                        (uint16_t)params[0].value.b,
                        (uint16_t)params[1].value.b, // reg
                        &data,
                        (uint8_t)params[1].value.a);

    i2c_release((i2c_t)params[0].value.a);
    if(res!=TEE_SUCCESS) return res;
    params[0].value.a=(uint32_t)data;
    return TEE_SUCCESS;
}

static TEE_Result pta_i2c_read_regs(uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                               TEE_PARAM_TYPE_VALUE_INPUT,
                                               TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                               TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }

    if(i2c_acquire((i2c_t)params[0].value.a)!=TEE_SUCCESS){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    res=i2c_read_regs((i2c_t)params[0].value.a,
                        (uint16_t)params[0].value.b, // addr
                        (uint16_t)params[1].value.b, // reg
                        (void *)params[2].memref.buffer,
                        (size_t)params[2].memref.size,
                        (uint8_t)params[1].value.a);
    i2c_release((i2c_t)params[0].value.a);
    return res;
}

static TEE_Result pta_i2c_write_reg(uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                               TEE_PARAM_TYPE_VALUE_INPUT,
                                               TEE_PARAM_TYPE_VALUE_INPUT,
                                               TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }

    if(i2c_acquire((i2c_t)params[0].value.a)!=TEE_SUCCESS){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    res=i2c_write_reg((i2c_t)params[0].value.a,
                        (uint16_t)params[0].value.b,
                        (uint16_t)params[1].value.b, // reg
                        (uint8_t)params[2].value.a, // write byte
                        (uint8_t)params[1].value.a);

    i2c_release((i2c_t)params[0].value.a);
    return res;
}

static TEE_Result pta_i2c_write_regs(uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                               TEE_PARAM_TYPE_VALUE_INPUT,
                                               TEE_PARAM_TYPE_MEMREF_INPUT,
                                               TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }

    if(i2c_acquire((i2c_t)params[0].value.a)!=TEE_SUCCESS){
        return TEE_ERROR_BAD_PARAMETERS;
    }

    res=i2c_write_regs((i2c_t)params[0].value.a,
                        (uint16_t)params[0].value.b,
                        (uint16_t)params[1].value.b, // reg
                        (void *)params[2].memref.buffer,
                        (size_t)params[2].memref.size,
                        (uint8_t)params[1].value.a);
    i2c_release((i2c_t)params[0].value.a);
    return res;
}

static TEE_Result pta_i2c_set_freq(uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                               TEE_PARAM_TYPE_NONE,
                                               TEE_PARAM_TYPE_NONE,
                                               TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }

    if(params[0].value.b>(uint32_t)I2C_SPEED_FAST_PLUS){
        return TEE_ERROR_NOT_SUPPORTED;
    }

    if(i2c_acquire((i2c_t)params[0].value.a)!=TEE_SUCCESS){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    
    res = i2c_set_frequency((i2c_t)params[0].value.a,(i2c_speed_t)params[0].value.b);

    i2c_release((i2c_t)params[0].value.a);

    return res;
}

static TEE_Result pta_i2c_get_overhead_timestamp(uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                               TEE_PARAM_TYPE_NONE,
                                               TEE_PARAM_TYPE_NONE,
                                               TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
#ifdef OVERHEAD_BENCHMARK
    if(params[0].memref.size<sizeof(benchmark_time)){
        return TEE_ERROR_SHORT_BUFFER;
    }
    memcpy(params[0].memref.buffer,benchmark_time,sizeof(benchmark_time));
    benchmark_idx=0;
    return TEE_SUCCESS;
#else
    return TEE_ERROR_NOT_IMPLEMENTED;
#endif
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
        case PTA_I2C_READ_BYTE:
            res=pta_i2c_read_byte(param_types,params);
            break;
        case PTA_I2C_READ_BYTES:
            res=pta_i2c_read_bytes(param_types,params);
            break;
        case PTA_I2C_WRITE_BYTE:
            res=pta_i2c_write_byte(param_types,params);
            break;
        case PTA_I2C_WRITE_BYTES:
            res=pta_i2c_write_bytes(param_types,params);
            break;
        case PTA_I2C_READ_REG:
            res=pta_i2c_read_reg(param_types,params);
            break;
        case PTA_I2C_READ_REGS:
            res=pta_i2c_read_regs(param_types,params);
            break;
        case PTA_I2C_WRITE_REG:
            res=pta_i2c_write_reg(param_types,params);
            break;
        case PTA_I2C_WRITE_REGS:
            res=pta_i2c_write_regs(param_types,params);
            break;
        case PTA_I2C_SET_FREQ:
            res=pta_i2c_set_freq(param_types,params);
            break;
        case PTA_I2C_GET_OVERHEAD_TIMESTAMP:
            res=pta_i2c_get_overhead_timestamp(param_types,params);
            break;
        default:
            EMSG("cmd: %d not supported %s",cmd_id,I2C_PTA_NAME);
            res=TEE_ERROR_NOT_SUPPORTED;
            break;
    }
    return res;
}

pseudo_ta_register(.uuid=PTA_I2C_SERVICE_UUID,
                .name = I2C_PTA_NAME,
                .flags = PTA_DEFAULT_FLAGS | TA_FLAG_CONCURRENT,
                .create_entry_point = create,
                .invoke_command_entry_point = invoke_command);