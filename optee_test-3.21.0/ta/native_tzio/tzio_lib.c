/**
 * @brief native lib for trust zone io
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <stdio.h>
#include <stdlib.h>

#include "tzio_lib.h"
#include "pta_i2c.h"
#include "pta_spi.h"
#include "pta_dht.h"
#include "pta_bh1750fvi.h"
#include "pta_mpu6050.h"
#include "pta_tel0157.h"
#include "pta_w25qxx.h"
#include "pta_at24cxx.h"
#include "pta_atk301.h"
#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#ifdef USE_PTA_IO_HELPER
#include "pta_io_helper.h"
#endif

#define FAT_PTR_ADDRESS_MASK 0x0001FFFF
#define FAT_PTR_SIGNATURE_MASK 0xFF000000
#define FAT_PTR_TAG_MASK 0x00FE0000
#define FAT_PTR_SECURE_STORAGE_SIGN_ID "iotwr"
#define FAT_PTR_SECURE_STORAGE_SIGN_ID_LEN 5
#define FAT_PTR_SECURE_STORAGE_SIGN_LEN 2

uint8_t global_signature[2]={0xE5,0x12};

#define N_TIMES 10

#define MAX_GPIO_PIN_NUM 27
#define MAX_I2C_NUM 1
#define MAX_SPI_NUM 0

#define TA_IO_HELPER_CMD_READ_REQ		2
#define TA_IO_HELPER_CMD_GET_DATA  		3

#define N_SESS N_IO_SIG

#ifdef USE_PTA_IO_HELPER
// transfer periph to pta io helper periph no
#define periph_to_pta_periph_no(periph_type, periph_no) \
            tzio_periph_to_io_signal_no(periph_type, periph_no)

#else

#define periph_to_sess_no(periph_type, periph_no) \
            tzio_periph_to_io_signal_no(periph_type, periph_no)

#define sess_no_to_helper_uuid(sess_no, io_helper_uuid) \
            tzio_signo_to_io_helper_uuid(sess_no, io_helper_uuid)

#endif

#define teetime_to_micro(t) \
    (uint64_t)t.seconds * 1000 * 1000 + (uint64_t)t.micros

static TEE_TASessionHandle i2c_sess = TEE_HANDLE_NULL;
static TEE_TASessionHandle spi_sess = TEE_HANDLE_NULL;
static TEE_TASessionHandle dht_sess = TEE_HANDLE_NULL;
static TEE_TASessionHandle bh1750fvi_sess = TEE_HANDLE_NULL;
static TEE_TASessionHandle mpu6050_sess = TEE_HANDLE_NULL;
static TEE_TASessionHandle w25qxx_sess = TEE_HANDLE_NULL;
static TEE_TASessionHandle at24cxx_sess = TEE_HANDLE_NULL;
static TEE_TASessionHandle tel0157_sess = TEE_HANDLE_NULL;
static TEE_TASessionHandle atk301_sess = TEE_HANDLE_NULL;

#ifdef USE_PTA_IO_HELPER
static TEE_TASessionHandle io_helper_sess=TEE_HANDLE_NULL;
#else
static TEE_TASessionHandle io_helper_sess[N_SESS] = {TEE_HANDLE_NULL};
#endif

static TEE_Time times[N_TIMES];

static uint64_t global_poll_times=0;

static inline uint8_t get_signature(uint32_t fat_ptr){
    return (fat_ptr & FAT_PTR_SIGNATURE_MASK) >> 24;
}

static inline uint8_t get_tag(uint32_t fat_ptr){
    return (fat_ptr & FAT_PTR_TAG_MASK) >> 17;
}

static inline uint32_t clear_signature_and_tag(uint32_t fat_ptr){
    return fat_ptr & FAT_PTR_ADDRESS_MASK;
}

static bool check_ptr_signed(uint32_t original_buf){
    uint8_t signature=get_signature(original_buf);
    uint8_t tag=get_tag(original_buf);
    if(signature!=global_signature[0]||tag!=global_signature[1]){
        return false;
    }
    return true;
}

static TEE_Result invoke_i2c_pta(uint32_t cmd_id, uint32_t param_types,
                                       TEE_Param params[TEE_NUM_PARAMS])
{
    const TEE_UUID i2c_uuid=PTA_I2C_SERVICE_UUID; 
    TEE_Result res = TEE_ERROR_GENERIC;
    // load existed session
    if(i2c_sess == TEE_HANDLE_NULL){
        // open session
        res=TEE_OpenTASession(&i2c_uuid,TEE_TIMEOUT_INFINITE,0,NULL,&i2c_sess,NULL);
        // return error
        if(res!=TEE_SUCCESS) {
            EMSG("The session with the i2c service cannot be established. Error: %x", res);
            return res;
        }
    }
    return TEE_InvokeTACommand(i2c_sess,TEE_TIMEOUT_INFINITE,cmd_id,param_types,params,NULL);
}

static TEE_Result invoke_spi_pta(uint32_t cmd_id, uint32_t param_types,
                                       TEE_Param params[TEE_NUM_PARAMS])
{
    const TEE_UUID spi_uuid=PTA_SPI_SERVICE_UUID; 
    TEE_Result res = TEE_ERROR_GENERIC;
    // load existed session
    if(spi_sess == TEE_HANDLE_NULL){
        // open session
        res=TEE_OpenTASession(&spi_uuid,TEE_TIMEOUT_INFINITE,0,NULL,&spi_sess,NULL);
        // return error
        if(res!=TEE_SUCCESS) {
            EMSG("The session with the spi service cannot be established. Error: %x", res);
            return res;
        }
    }
    return TEE_InvokeTACommand(spi_sess,TEE_TIMEOUT_INFINITE,cmd_id,param_types,params,NULL);
}

static TEE_Result invoke_dht_pta(uint32_t cmd_id, uint32_t param_types,
                                       TEE_Param params[TEE_NUM_PARAMS])
{
    const TEE_UUID dht_uuid=PTA_DHT_SERVICE_UUID; 
    TEE_Result res = TEE_ERROR_GENERIC;
    // load existed session
    if(dht_sess == TEE_HANDLE_NULL){
        // open session
        res=TEE_OpenTASession(&dht_uuid,TEE_TIMEOUT_INFINITE,0,NULL,&dht_sess,NULL);
        // return error
        if(res!=TEE_SUCCESS) {
            EMSG("The session with the dht service cannot be established. Error: %x", res);
            return res;
        }
    }
    return TEE_InvokeTACommand(dht_sess,TEE_TIMEOUT_INFINITE,cmd_id,param_types,params,NULL);
}

static TEE_Result invoke_bh1750fvi_pta(uint32_t cmd_id, uint32_t param_types,
                                       TEE_Param params[TEE_NUM_PARAMS])
{
    const TEE_UUID bh1750fvi_uuid=PTA_BH1750FVI_SERVICE_UUID;
    TEE_Result res = TEE_ERROR_GENERIC;
    // load existed session
    if(bh1750fvi_sess == TEE_HANDLE_NULL){
        // open session
        res=TEE_OpenTASession(&bh1750fvi_uuid,TEE_TIMEOUT_INFINITE,0,NULL,&bh1750fvi_sess,NULL);
        // return error
        if(res!=TEE_SUCCESS) {
            EMSG("The session with the bh1750fvi service cannot be established. Error: %x", res);
            return res;
        }
    }
    return TEE_InvokeTACommand(bh1750fvi_sess,TEE_TIMEOUT_INFINITE,cmd_id,param_types,params,NULL);
}

static TEE_Result invoke_mpu6050_pta(uint32_t cmd_id, uint32_t param_types,
                                       TEE_Param params[TEE_NUM_PARAMS])
{
    const TEE_UUID mpu6050_uuid=PTA_MPU6050_SERVICE_UUID;
    TEE_Result res = TEE_ERROR_GENERIC;
    if(mpu6050_sess == TEE_HANDLE_NULL){
        res=TEE_OpenTASession(&mpu6050_uuid, TEE_TIMEOUT_INFINITE,0,NULL,&mpu6050_sess,NULL);
        if(res!=TEE_SUCCESS){
            EMSG("The session with the mpu6050 service cannot be established. Error: %x", res);
            return res;
        }
    }
    return TEE_InvokeTACommand(mpu6050_sess, TEE_TIMEOUT_INFINITE,cmd_id,param_types, params,NULL);                             
}

static TEE_Result invoke_w25qxx_pta(uint32_t cmd_id, uint32_t param_types,
                                       TEE_Param params[TEE_NUM_PARAMS])
{
    const TEE_UUID w25qxx_uuid=PTA_W25QXX_SERVICE_UUID;
    TEE_Result res = TEE_ERROR_GENERIC;
    if(w25qxx_sess == TEE_HANDLE_NULL){
        res=TEE_OpenTASession(&w25qxx_uuid, TEE_TIMEOUT_INFINITE,0,NULL,&w25qxx_sess,NULL);
        if(res!=TEE_SUCCESS){
            EMSG("The session with the w25qxx_sess service cannot be established. Error: %x", res);
            return res;
        }
    }
    return TEE_InvokeTACommand(w25qxx_sess, TEE_TIMEOUT_INFINITE,cmd_id,param_types, params,NULL);                             
}

static TEE_Result invoke_tel0157_pta(uint32_t cmd_id, uint32_t param_types,
                                       TEE_Param params[TEE_NUM_PARAMS])
{
    const TEE_UUID tel0157_uuid=PTA_TEL0157_SERVICE_UUID;
    TEE_Result res = TEE_ERROR_GENERIC;
    if(tel0157_sess == TEE_HANDLE_NULL){
        res=TEE_OpenTASession(&tel0157_uuid, TEE_TIMEOUT_INFINITE,0,NULL,&tel0157_sess,NULL);
        if(res!=TEE_SUCCESS){
            EMSG("The session with the tel0157_sess service cannot be established. Error: %x", res);
            return res;
        }
    }
    return TEE_InvokeTACommand(tel0157_sess, TEE_TIMEOUT_INFINITE,cmd_id,param_types, params,NULL);                             
}

static TEE_Result invoke_at24cxx_pta(uint32_t cmd_id, uint32_t param_types,
                                       TEE_Param params[TEE_NUM_PARAMS])
{
    const TEE_UUID at24cxx_uuid=PTA_AT24CXX_SERVICE_UUID;
    TEE_Result res = TEE_ERROR_GENERIC;
    if(at24cxx_sess == TEE_HANDLE_NULL){
        res=TEE_OpenTASession(&at24cxx_uuid, TEE_TIMEOUT_INFINITE,0,NULL,&at24cxx_sess,NULL);
        if(res!=TEE_SUCCESS){
            EMSG("The session with the at24cxx_sess service cannot be established. Error: %x", res);
            return res;
        }
    }
    return TEE_InvokeTACommand(at24cxx_sess, TEE_TIMEOUT_INFINITE,cmd_id,param_types, params,NULL);                             
}

static TEE_Result invoke_atk301_pta(uint32_t cmd_id, uint32_t param_types,
                                       TEE_Param params[TEE_NUM_PARAMS])
{
    const TEE_UUID atk301_uuid=PTA_ATK301_SERVICE_UUID;
    TEE_Result res = TEE_ERROR_GENERIC;
    if(atk301_sess == TEE_HANDLE_NULL){
        res=TEE_OpenTASession(&atk301_uuid, TEE_TIMEOUT_INFINITE,0,NULL,&atk301_sess,NULL);
        if(res!=TEE_SUCCESS){
            EMSG("The session with the atk301_sess service cannot be established. Error: %x", res);
            return res;
        }
    }
    return TEE_InvokeTACommand(atk301_sess, TEE_TIMEOUT_INFINITE,cmd_id,param_types, params,NULL);                             
}


#ifdef USE_PTA_IO_HELPER
static TEE_Result invoke_io_helper_pta(uint32_t cmd_id, uint32_t param_types,
                                       TEE_Param params[TEE_NUM_PARAMS]){
        TEE_Result res = TEE_ERROR_GENERIC;
        if(io_helper_sess==TEE_HANDLE_NULL){
            TEE_UUID uuid=PTA_IO_HELPER_UUID;
            res=TEE_OpenTASession(&uuid,TEE_TIMEOUT_INFINITE,0,NULL,&io_helper_sess,NULL);
            if(res!=TEE_SUCCESS){
                EMSG("The session with the io helper cannot be established. Error: %x",res);
                return res;
            }
        }
        return TEE_InvokeTACommand(io_helper_sess,TEE_TIMEOUT_INFINITE,cmd_id,param_types,params,NULL);
}
#else

static TEE_Result invoke_io_helper_ta(uint32_t sess_no, uint32_t cmd_id, uint32_t param_types,
                                       TEE_Param params[TEE_NUM_PARAMS]){
        TEE_Result res = TEE_ERROR_GENERIC;
        // load existed session
        if(io_helper_sess[sess_no]==TEE_HANDLE_NULL){
            // open session
            TEE_UUID uuid={};
            sess_no_to_helper_uuid(sess_no,&uuid);
            res=TEE_OpenTASession(&uuid,TEE_TIMEOUT_INFINITE,0,NULL,&io_helper_sess[sess_no] ,NULL);
            // return error
            if(res!=TEE_SUCCESS) {
                EMSG("The session with the io helper cannot be established. Error: %x",res);
                return res;
            }
        }
        
        return TEE_InvokeTACommand(io_helper_sess[sess_no] ,TEE_TIMEOUT_INFINITE,cmd_id,param_types,params,NULL);                           
}

#endif

static int record_benchmark_time_wrapper(wasm_exec_env_t exec_env, uint32_t idx){
    if(idx>=N_TIMES){return TEE_ERROR_BAD_PARAMETERS;}
    TEE_GetSystemTime(&times[idx]);
    return TEE_SUCCESS;
}

static int i2c_read_bytes_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_dev, uint32_t addr, uint32_t flags, uint32_t dst_buf_addr, uint32_t buf_len){
    TEE_Param params[TEE_NUM_PARAMS]={};
    uint32_t param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                            TEE_PARAM_TYPE_NONE);
#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(dst_buf_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    dst_buf_addr=clear_signature_and_tag(dst_buf_addr);
#endif
    
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(!wasm_runtime_validate_app_addr(inst,dst_buf_addr,buf_len)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    void * buf= wasm_runtime_addr_app_to_native(inst,dst_buf_addr);
    params[0].value.a=i2c_dev;
    params[0].value.b=addr;
    params[1].value.a=flags;
    params[2].memref.buffer=buf;
    params[2].memref.size=buf_len;
    return invoke_i2c_pta(PTA_I2C_READ_BYTES,param_types,params);
}

static int i2c_write_bytes_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_dev, uint32_t addr, uint32_t flags, uint32_t src_buf_addr, uint32_t buf_len){
    TEE_Param params[TEE_NUM_PARAMS]={};
    uint32_t param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_MEMREF_INPUT,
                                            TEE_PARAM_TYPE_NONE);
#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(src_buf_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    src_buf_addr=clear_signature_and_tag(src_buf_addr);
#endif
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(!wasm_runtime_validate_app_addr(inst,src_buf_addr,buf_len)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    void * buf= wasm_runtime_addr_app_to_native(inst,src_buf_addr);
    params[0].value.a=i2c_dev;
    params[0].value.b=addr;
    params[1].value.a=flags;
    params[2].memref.buffer=buf;
    params[2].memref.size=buf_len;
    return invoke_i2c_pta(PTA_I2C_WRITE_BYTES,param_types,params);
}

static int i2c_set_freq_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_dev, uint32_t i2c_freq_opt){
    TEE_Param params[TEE_NUM_PARAMS]={};
    uint32_t param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                           TEE_PARAM_TYPE_NONE,
                                           TEE_PARAM_TYPE_NONE,
                                           TEE_PARAM_TYPE_NONE);
    params[0].value.a=i2c_dev;
    params[0].value.b=i2c_freq_opt;
    return invoke_i2c_pta(PTA_I2C_SET_FREQ,param_types,params);
}

static int i2c_read_regs_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_dev, uint32_t addr, uint32_t flags,uint32_t reg, uint32_t dst_buf_addr, uint32_t buf_len){
    TEE_Param params[TEE_NUM_PARAMS]={};
    uint32_t param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                            TEE_PARAM_TYPE_NONE);
#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(dst_buf_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    dst_buf_addr=clear_signature_and_tag(dst_buf_addr);
#endif

    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(!wasm_runtime_validate_app_addr(inst,dst_buf_addr,buf_len)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    void * buf= wasm_runtime_addr_app_to_native(inst,dst_buf_addr);
    params[0].value.a=i2c_dev;
    params[0].value.b=addr;
    params[1].value.a=flags;
    params[1].value.b=reg;
    params[2].memref.buffer=buf;
    params[2].memref.size=buf_len;
    return invoke_i2c_pta(PTA_I2C_READ_REGS,param_types,params);
}

static int i2c_write_regs_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_dev, uint32_t addr, uint32_t flags,uint32_t reg, uint32_t src_buf_addr, uint32_t buf_len){
    TEE_Param params[TEE_NUM_PARAMS]={};
    uint32_t param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_MEMREF_INPUT,
                                            TEE_PARAM_TYPE_NONE);
#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(src_buf_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    src_buf_addr=clear_signature_and_tag(src_buf_addr);
#endif

    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(!wasm_runtime_validate_app_addr(inst,src_buf_addr,buf_len)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    void * buf= wasm_runtime_addr_app_to_native(inst,src_buf_addr);
    params[0].value.a=i2c_dev;
    params[0].value.b=addr;
    params[1].value.a=flags;
    params[1].value.b=reg;
    params[2].memref.buffer=buf;
    params[2].memref.size=buf_len;
    return invoke_i2c_pta(PTA_I2C_WRITE_BYTES,param_types,params);
}

/**
 * Return TEE_Result as int, 0 is TEE_SUCCESS
 */
static int dht_init_wrapper(wasm_exec_env_t exec_env, uint32_t pin, uint32_t dht_type)
{
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a = (uint32_t)pin;
    params[0].value.b = (uint32_t)dht_type;
    res = invoke_dht_pta(PTA_CMD_DHT_INIT,param_types,params);
    return res;
}

/**
 * Return TEE_Result as int, 0 is TEE_SUCCESS
 */
static int dht_read_wrapper(wasm_exec_env_t exec_env, uint32_t pin, uint32_t dht_type, uint32_t dst_temp_addr, uint32_t dst_hum_addr)
{
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;

#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(dst_temp_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    dst_temp_addr=clear_signature_and_tag(dst_temp_addr);
    if(!check_ptr_signed(dst_hum_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    dst_hum_addr=clear_signature_and_tag(dst_hum_addr);
#endif

    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(!wasm_runtime_validate_app_addr(inst,dst_temp_addr,sizeof(int16_t))
    || !wasm_runtime_validate_app_addr(inst,dst_hum_addr,sizeof(int16_t))){
        return TEE_ERROR_ACCESS_DENIED;
    }
    int16_t * temp = wasm_runtime_addr_app_to_native(inst,dst_temp_addr);
    int16_t * hum  = wasm_runtime_addr_app_to_native(inst,dst_hum_addr);

    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_OUTPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    
    params[0].value.a = (uint32_t)pin;
    params[0].value.b = (uint32_t)dht_type;
    res = invoke_dht_pta(PTA_CMD_DHT_READ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    *temp=(int16_t)params[1].value.a;
    *hum=(int16_t)params[1].value.b;
    return TEE_SUCCESS;
}

static int dht_read_async_wrapper(wasm_exec_env_t exec_env,uint32_t pin,uint32_t dht_type,uint32_t handler_func_ptr)
{
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
#ifdef USE_PTA_IO_HELPER
    uint16_t periph_no;
    if(pin>MAX_GPIO_PIN_NUM) return TEE_ERROR_BAD_PARAMETERS;
    // todo: may use another func
    periph_no=(uint16_t)periph_to_pta_periph_no(0,pin);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)DHT_READ;
    params[1].value.a = dht_type;
    DMSG("async read invoke to pta io helper");
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    // save to sigtable, register handler
    // params[0].value.a is sub_signo
    if(tzio_register_handler(exec_env,periph_no,params[0].value.a,handler_func_ptr,(uint32_t)DHT_READ,0)<0){
        EMSG("register handler to sigtable failed");
        return TEE_ERROR_GENERIC;
    }
#else
    uint32_t sess_no=0;

    if(pin>MAX_GPIO_PIN_NUM) return TEE_ERROR_BAD_PARAMETERS;
    sess_no=periph_to_sess_no(0,pin);
    if(sess_no>=N_SESS) return TEE_ERROR_NOT_SUPPORTED;

    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    
    params[0].value.a = (uint32_t)DHT_READ;
    params[1].value.a = dht_type;
    res = invoke_io_helper_ta(sess_no,TA_IO_HELPER_CMD_READ_REQ,param_types,params);

    if(res!=TEE_SUCCESS) return res;
    // save to sigtable, register handler
    // sess_no is signo, params[0].value.a is sub_signo
    if(tzio_register_handler(exec_env,sess_no,params[0].value.a,handler_func_ptr,(uint32_t)DHT_READ,0)<0){
        EMSG("register handler to sigtable failed");
        return TEE_ERROR_GENERIC;
    }

#endif

    return res;
}

static int at24cxx_read_async_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no, uint32_t i2c_addr,uint32_t read_addr,uint32_t read_len,uint32_t handler_func_ptr){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    if(i2c_no>MAX_I2C_NUM) return TEE_ERROR_BAD_PARAMETERS;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)AT24CXX_READ;
    params[1].value.a = i2c_addr;
    params[1].value.b = read_addr;
    params[2].value.a = read_len;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    if(tzio_register_handler(exec_env,periph_no,params[0].value.a,handler_func_ptr,(uint32_t)AT24CXX_READ,read_len)<0){
        EMSG("register handler to sigtable failed");
        return TEE_ERROR_GENERIC;
    }
    return TEE_SUCCESS;
}

static int at24cxx_write_async_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no, uint32_t i2c_addr, uint32_t write_addr, uint32_t src_buf_addr, uint32_t src_len,uint32_t handler_func_ptr){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;

#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(src_buf_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    src_buf_addr=clear_signature_and_tag(src_buf_addr);
#endif

    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(i2c_no>MAX_I2C_NUM) return TEE_ERROR_BAD_PARAMETERS;
    if(!wasm_runtime_validate_app_addr(inst,src_buf_addr,src_len)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    void * buf= wasm_runtime_addr_app_to_native(inst,src_buf_addr);
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_MEMREF_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)AT24CXX_WRITE;
    params[1].memref.buffer=buf;
    params[1].memref.size=src_len;
    params[2].value.a = i2c_addr;
    params[2].value.b = write_addr;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_WRITE_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    if(tzio_register_handler(exec_env,periph_no,params[0].value.a,handler_func_ptr,(uint32_t)AT24CXX_WRITE,src_len)<0){
        EMSG("register handler to sigtable failed");
        return TEE_ERROR_GENERIC;
    }
    return TEE_SUCCESS;
}

static int i2c_read_bytes_async_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no, uint32_t i2c_addr,uint32_t flags,uint32_t read_len, uint32_t handler_func_ptr){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    if(i2c_no>MAX_I2C_NUM) return TEE_ERROR_BAD_PARAMETERS;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)I2C_READ_BYTES;
    params[1].value.a = i2c_addr;
    params[1].value.b = read_len;
    params[2].value.a = flags;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    if(tzio_register_handler(exec_env,periph_no,params[0].value.a,handler_func_ptr,(uint32_t)I2C_READ_BYTES,read_len)<0){
        EMSG("register handler to sigtable failed");
        return TEE_ERROR_GENERIC;
    }
    return TEE_SUCCESS;
}

static int i2c_read_regs_async_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no, uint32_t i2c_addr, uint32_t flags, uint32_t reg, uint32_t read_len, uint32_t handler_func_ptr){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    if(i2c_no>MAX_I2C_NUM) return TEE_ERROR_BAD_PARAMETERS;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)I2C_READ_REGS;
    params[1].value.a = i2c_addr;
    params[1].value.b = reg;
    params[2].value.a = read_len;
    params[2].value.b = flags;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    if(tzio_register_handler(exec_env,periph_no,params[0].value.a,handler_func_ptr,(uint32_t)I2C_READ_REGS,read_len)<0){
        EMSG("register handler to sigtable failed");
        return TEE_ERROR_GENERIC;
    }
    return TEE_SUCCESS;
}

static int bh1750fvi_sample_async_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no, uint32_t i2c_addr, uint32_t handler_func_ptr){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    if(i2c_no>MAX_I2C_NUM) return TEE_ERROR_BAD_PARAMETERS;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)BH1750FVI_SAMPLE;
    params[1].value.a = i2c_addr;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    if(tzio_register_handler(exec_env,periph_no,params[0].value.a,handler_func_ptr,(uint32_t)BH1750FVI_SAMPLE,0)<0){
        EMSG("register handler to sigtable failed");
        return TEE_ERROR_GENERIC;
    }
    return TEE_SUCCESS;
}

static int mpu6050_sample_async_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no, uint32_t i2c_addr, uint32_t handler_func_ptr){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    if(i2c_no>MAX_I2C_NUM) return TEE_ERROR_BAD_PARAMETERS;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)MPU6050_SAMPLE;
    params[1].value.a = i2c_addr;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    if(tzio_register_handler(exec_env,periph_no,params[0].value.a,handler_func_ptr,(uint32_t)MPU6050_SAMPLE,0)<0){
        EMSG("register handler to sigtable failed");
        return TEE_ERROR_GENERIC;
    }
    return TEE_SUCCESS;
}

static int atk301_get_fingerprint_image_async_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no, uint32_t i2c_addr, uint32_t handler_func_ptr){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    if(i2c_no>MAX_I2C_NUM) return TEE_ERROR_BAD_PARAMETERS;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)ATK301_GET_FINGERPRINT_IMAGE;
    params[1].value.a = i2c_addr;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    if(tzio_register_handler(exec_env,periph_no,params[0].value.a,handler_func_ptr,(uint32_t)ATK301_GET_FINGERPRINT_IMAGE,0)<0){
        EMSG("register handler to sigtable failed");
        return TEE_ERROR_GENERIC;
    }
    return TEE_SUCCESS;
}

static int atk301_upload_fingerprint_image_async_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no, uint32_t i2c_addr, uint32_t handler_func_ptr){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    if(i2c_no>MAX_I2C_NUM) return TEE_ERROR_BAD_PARAMETERS;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)ATK301_UPLOAD_FINGERPRINT_IMAGE;
    params[1].value.a = i2c_addr;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    if(tzio_register_handler(exec_env,periph_no,params[0].value.a,handler_func_ptr,(uint32_t)ATK301_UPLOAD_FINGERPRINT_IMAGE,0)<0){
        EMSG("register handler to sigtable failed");
        return TEE_ERROR_GENERIC;
    }
    return TEE_SUCCESS;
}




static int dht_read_async_future_wrapper(wasm_exec_env_t exec_env,uint32_t pin,uint32_t dht_type){
    // use pta io helper
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    if(pin>MAX_GPIO_PIN_NUM) return TEE_ERROR_BAD_PARAMETERS;
    periph_no=(uint16_t)periph_to_pta_periph_no(0,pin);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)DHT_READ;
    params[1].value.a = dht_type;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    // save to sigtable, register handler
    // params[0].value.a is sub_signo, return as a descriptor
    return params[0].value.a;
}

static int dht_read_async_promise_wrapper(wasm_exec_env_t exec_env,uint32_t pin,uint32_t dht_type){
    // use pta io helper
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    if(pin>MAX_GPIO_PIN_NUM) return TEE_ERROR_BAD_PARAMETERS;
    periph_no=(uint16_t)periph_to_pta_periph_no(0,pin);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)DHT_READ;
    params[1].value.a = dht_type;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    // save to sigtable, register handler
    // params[0].value.a is sub_signo
    // for a promise, high 16 bits is perioh_no
    // low 16 bits is sub_signo
    return ((int)periph_no<<16)|(int)(params[0].value.a&0xFFFF);
}

static int bh1750fvi_sample_async_future_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no, uint32_t i2c_addr){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    if(i2c_no>MAX_I2C_NUM) return TEE_ERROR_BAD_PARAMETERS;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)BH1750FVI_SAMPLE;
    params[1].value.a = i2c_addr;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    return params[0].value.a;
}

static int mpu6050_sample_async_future_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no, uint32_t i2c_addr){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    if(i2c_no>MAX_I2C_NUM) return TEE_ERROR_BAD_PARAMETERS;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)MPU6050_SAMPLE;
    params[1].value.a = i2c_addr;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    return params[0].value.a;
}

static int atk301_get_fingerprint_image_async_future_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no, uint32_t i2c_addr){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    if(i2c_no>MAX_I2C_NUM) return TEE_ERROR_BAD_PARAMETERS;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)ATK301_GET_FINGERPRINT_IMAGE;
    params[1].value.a = i2c_addr;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    return params[0].value.a;
}

static int atk301_upload_fingerprint_image_async_future_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no, uint32_t i2c_addr){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    if(i2c_no>MAX_I2C_NUM) return TEE_ERROR_BAD_PARAMETERS;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)ATK301_UPLOAD_FINGERPRINT_IMAGE;
    params[1].value.a = i2c_addr;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    return params[0].value.a;
}

static int i2c_read_bytes_async_future_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no, uint32_t i2c_addr,uint32_t flags,uint32_t read_len){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    if(i2c_no>MAX_I2C_NUM) return TEE_ERROR_BAD_PARAMETERS;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)I2C_READ_BYTES;
    params[1].value.a = i2c_addr;
    params[1].value.b = read_len;
    params[2].value.a = flags;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    return params[0].value.a;
}

static int i2c_write_bytes_async_future_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no, uint32_t i2c_addr,uint32_t flags, uint32_t src_buf_addr, uint32_t buf_len){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;

#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(src_buf_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    src_buf_addr=clear_signature_and_tag(src_buf_addr);
#endif

    if(i2c_no>MAX_I2C_NUM) return TEE_ERROR_BAD_PARAMETERS;
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(!wasm_runtime_validate_app_addr(inst,src_buf_addr,buf_len)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    void * buf= wasm_runtime_addr_app_to_native(inst,src_buf_addr);
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_MEMREF_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)I2C_WRITE_BYTES;
    params[1].memref.buffer = buf;
    params[1].memref.size = buf_len;
    params[2].value.a = i2c_addr;
    params[2].value.b = flags;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_WRITE_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    return params[0].value.a;
}

static int i2c_read_regs_async_future_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no, uint32_t i2c_addr, uint32_t flags, uint32_t reg, uint32_t read_len){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    if(i2c_no>MAX_I2C_NUM) return TEE_ERROR_BAD_PARAMETERS;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)I2C_READ_REGS;
    params[1].value.a = i2c_addr;
    params[1].value.b = reg;
    params[2].value.a = read_len;
    params[2].value.b = flags;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    return params[0].value.a;
}

static int spi_transfer_bytes_async_future_wrapper(wasm_exec_env_t exec_env, uint32_t spi_no,uint32_t spi_cs_no, uint32_t cont,
                             uint32_t mode, uint32_t freq,  uint32_t src_len, uint32_t dst_len,
                              uint32_t src_buf_addr){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;

#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(src_buf_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    src_buf_addr=clear_signature_and_tag(src_buf_addr);
#endif

    if(spi_no>MAX_SPI_NUM) return TEE_ERROR_BAD_PARAMETERS;
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(!wasm_runtime_validate_app_addr(inst,src_buf_addr,src_len)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    void * buf= wasm_runtime_addr_app_to_native(inst,src_buf_addr);
    periph_no=(uint16_t)periph_to_pta_periph_no(2,spi_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_MEMREF_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)SPI_TRANSFER_BYTES;
    params[1].memref.buffer = buf;
    params[1].memref.size = src_len;
    params[2].value.a=spi_cs_no;
    params[2].value.b=cont;
    params[3].value.a=(mode<<16) | (freq & 0xFFFF);
    params[3].value.b=dst_len;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_WRITE_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    return params[0].value.a;
}

static int write_secure_object_async_future_wrapper(wasm_exec_env_t exec_env, uint32_t id_addr, uint32_t id_len, uint32_t data_addr, uint32_t data_len){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    char * id;
    char * data;

#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(id_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    id_addr=clear_signature_and_tag(id_addr);
    if(!check_ptr_signed(data_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    data_addr=clear_signature_and_tag(data_addr);
#endif

    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(!wasm_runtime_validate_app_addr(inst,data_addr,data_len)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    if(!wasm_runtime_validate_app_addr(inst,id_addr,id_len)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    id=wasm_runtime_addr_app_to_native(inst,id_addr);
    data=wasm_runtime_addr_app_to_native(inst,data_addr);
    periph_no=(uint16_t)periph_to_pta_periph_no(3,0);// periph secure storage
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_MEMREF_INPUT,
                                  TEE_PARAM_TYPE_MEMREF_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)WRITE_SECURE_OBJECT;
    params[1].memref.buffer = id;
    params[1].memref.size = id_len;
    params[2].memref.buffer = data;
    params[2].memref.size = data_len;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_WRITE_REQ_WITH_BUFFER,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    return params[0].value.a;
}

static int read_secure_object_async_future_wrapper(wasm_exec_env_t exec_env, uint32_t id_addr, uint32_t id_len){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    char * id;
#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(id_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    id_addr=clear_signature_and_tag(id_addr);
#endif
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(!wasm_runtime_validate_app_addr(inst,id_addr,id_len)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    id=wasm_runtime_addr_app_to_native(inst,id_addr);
    periph_no=(uint16_t)periph_to_pta_periph_no(3,0);// periph secure storage
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_MEMREF_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)READ_SECURE_OBJECT;
    params[1].memref.buffer = id;
    params[1].memref.size = id_len;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ_WITH_BUFFER,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    return params[0].value.a;
}

static int at24cxx_read_async_future_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no, uint32_t i2c_addr,uint32_t read_addr,uint32_t read_len){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    if(i2c_no>MAX_I2C_NUM) return TEE_ERROR_BAD_PARAMETERS;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)AT24CXX_READ;
    params[1].value.a = i2c_addr;
    params[1].value.b = read_addr;
    params[2].value.a = read_len;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    return params[0].value.a;
}

static int at24cxx_write_async_future_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no, uint32_t i2c_addr, uint32_t write_addr, uint32_t src_buf_addr, uint32_t src_len){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;

#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(src_buf_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    src_buf_addr=clear_signature_and_tag(src_buf_addr);
#endif

    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(i2c_no>MAX_I2C_NUM) return TEE_ERROR_BAD_PARAMETERS;
    if(!wasm_runtime_validate_app_addr(inst,src_buf_addr,src_len)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    void * buf= wasm_runtime_addr_app_to_native(inst,src_buf_addr);
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_MEMREF_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)AT24CXX_WRITE;
    params[1].memref.buffer=buf;
    params[1].memref.size=src_len;
    params[2].value.a = i2c_addr;
    params[2].value.b = write_addr;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_WRITE_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    return params[0].value.a;
}

static int tel0157_get_gnss_len_async_future_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
     param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)TEL0157_GET_GNSS_LEN;
    params[1].value.a = i2c_addr;
     res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    return params[0].value.a;
}


static int tel0157_get_all_gnss_async_future_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr, uint32_t gnss_len){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)TEL0157_GET_ALL_GNSS;
    params[1].value.a = i2c_addr;
    params[1].value.b = gnss_len;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    return params[0].value.a;
}

static int tel0157_get_utc_time_async_future_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)TEL0157_GET_UTC_TIME;
    params[1].value.a = i2c_addr;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    return params[0].value.a;
}

static int tel0157_get_utc_time_async_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr, uint32_t handler_func_ptr){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)TEL0157_GET_UTC_TIME;
    params[1].value.a = i2c_addr;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    if(tzio_register_handler(exec_env,periph_no,params[0].value.a,handler_func_ptr,(uint32_t)TEL0157_GET_UTC_TIME,0)<0){
        EMSG("register handler to sigtable failed");
        return TEE_ERROR_GENERIC;
    }
    return TEE_SUCCESS;
}

static int tel0157_get_gnss_mode_async_future_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)TEL0157_GET_GNSS_MODE;
    params[1].value.a = i2c_addr;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    return params[0].value.a;
}

static int tel0157_get_num_sat_used_async_future_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)TEL0157_GET_NUM_SAT_USED;
    params[1].value.a = i2c_addr;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    return params[0].value.a;
}

static int tel0157_get_lon_async_future_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)TEL0157_GET_LON;
    params[1].value.a = i2c_addr;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    return params[0].value.a;
}

static int tel0157_get_lat_async_future_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)TEL0157_GET_LAT;
    params[1].value.a = i2c_addr;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    return params[0].value.a;
}

static int tel0157_get_alt_async_future_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)TEL0157_GET_ALT;
    params[1].value.a = i2c_addr;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    return params[0].value.a;
}

static int tel0157_get_lon_async_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr, uint32_t handler_func_ptr){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)TEL0157_GET_LON;
    params[1].value.a = i2c_addr;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    if(tzio_register_handler(exec_env,periph_no,params[0].value.a,handler_func_ptr,(uint32_t)TEL0157_GET_LON,0)<0){
        EMSG("register handler to sigtable failed");
        return TEE_ERROR_GENERIC;
    }
    return TEE_SUCCESS;
}

static int tel0157_get_lat_async_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr, uint32_t handler_func_ptr){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)TEL0157_GET_LAT;
    params[1].value.a = i2c_addr;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    if(tzio_register_handler(exec_env,periph_no,params[0].value.a,handler_func_ptr,(uint32_t)TEL0157_GET_LAT,0)<0){
        EMSG("register handler to sigtable failed");
        return TEE_ERROR_GENERIC;
    }
    return TEE_SUCCESS;
}

static int tel0157_get_alt_async_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr, uint32_t handler_func_ptr){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)TEL0157_GET_ALT;
    params[1].value.a = i2c_addr;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    if(tzio_register_handler(exec_env,periph_no,params[0].value.a,handler_func_ptr,(uint32_t)TEL0157_GET_ALT,0)<0){
        EMSG("register handler to sigtable failed");
        return TEE_ERROR_GENERIC;
    }
    return TEE_SUCCESS;
}

static int tel0157_get_sog_async_future_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)TEL0157_GET_SOG;
    params[1].value.a = i2c_addr;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    return params[0].value.a;
}

static int tel0157_get_cog_async_future_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr){
    TEE_Result res = TEE_SUCCESS;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint16_t periph_no;
    periph_no=(uint16_t)periph_to_pta_periph_no(1,i2c_no);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a = (uint32_t)periph_no;
    params[0].value.b = (uint32_t)TEL0157_GET_COG;
    params[1].value.a = i2c_addr;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_READ_REQ,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    return params[0].value.a;
}

static int get_async_data_wrapper(wasm_exec_env_t exec_env,uint32_t signo,uint32_t sub_signo,uint32_t dst_buf_addr,uint32_t dst_buf_len_addr)
{
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
#ifdef USE_FAT_PTR_CHECK
    if(dst_buf_addr!=0){
        if(!check_ptr_signed(dst_buf_addr)){
            return TEE_ERROR_ACCESS_DENIED;
        }
        dst_buf_addr=clear_signature_and_tag(dst_buf_addr);
    }
    if(dst_buf_len_addr!=0){
        if(!check_ptr_signed(dst_buf_len_addr)){
            return TEE_ERROR_ACCESS_DENIED;
        }
        dst_buf_len_addr=clear_signature_and_tag(dst_buf_len_addr);
    }
#endif
    // signo is sess_no
    if(signo>=N_SESS) return TEE_ERROR_BAD_PARAMETERS;
    // sub_signo 
    if(sub_signo>=N_SESS) return TEE_ERROR_BAD_PARAMETERS;

    wasm_module_inst_t inst= wasm_runtime_get_module_inst(exec_env);
    // validate buf len ptr
    if(!wasm_runtime_validate_app_addr(inst,dst_buf_len_addr,sizeof(uint32_t))){
        return TEE_ERROR_ACCESS_DENIED;
    }
    uint32_t * buf_len = wasm_runtime_addr_app_to_native(inst,dst_buf_len_addr);
    // validate buf ptr 
    if(!wasm_runtime_validate_app_addr(inst,dst_buf_addr,*buf_len)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    void * buf = wasm_runtime_addr_app_to_native(inst,dst_buf_addr);

    param_types=TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                TEE_PARAM_TYPE_NONE,
                                TEE_PARAM_TYPE_NONE);
#ifdef USE_PTA_IO_HELPER
    // for now signo is periph no
    params[0].value.a=signo;
    params[0].value.b=sub_signo;
    // set buf and len
    params[1].memref.buffer=buf;
    params[1].memref.size=*buf_len;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_GET_DATA,param_types,params);
#else
    // set sub signo
    params[0].value.a=sub_signo;
    // set buf and len
    params[1].memref.buffer=buf;
    params[1].memref.size=*buf_len;
    res=invoke_io_helper_ta(signo,TA_IO_HELPER_CMD_GET_DATA,param_types,params);
#endif
    if(res==TEE_ERROR_SHORT_BUFFER){
        // get buf len needed
        // modified buf len to suggest len needed
        *buf_len=params[1].memref.size;
        return TEE_ERROR_SHORT_BUFFER;
    }else if(res!=TEE_SUCCESS){
        // error get req result
        return TEE_ERROR_NO_DATA; 
    }
    // sucess get req result, but req falied
    // req return value is params[0].value.a
    if(((int)params[0].value.a)<0){
        DMSG("get io req result success, but io req failed with %d",(int)params[0].value.a);
        return params[0].value.a;
    }
    // success get req result, req success
    // return value is ret, usually suggest real data len we get
    // return (int)params[0].value.a;

    // for test, we return params[0].value.b here, which is poll times
    return params[0].value.b;
}

static int get_async_data_future_wrapper(wasm_exec_env_t exec_env,uint32_t signo,uint32_t sub_signo,uint32_t dst_buf_addr,uint32_t dst_buf_len_addr){
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;

#ifdef USE_FAT_PTR_CHECK
    if(dst_buf_addr!=0){
        if(!check_ptr_signed(dst_buf_addr)){
            return TEE_ERROR_ACCESS_DENIED;
        }
        dst_buf_addr=clear_signature_and_tag(dst_buf_addr);
    }
    if(dst_buf_len_addr!=0){
        if(!check_ptr_signed(dst_buf_len_addr)){
            return TEE_ERROR_ACCESS_DENIED;
        }
        dst_buf_len_addr=clear_signature_and_tag(dst_buf_len_addr);
    }
#endif

    // int poll_times=1;
    // signo is periph_no
    if(signo>=N_SESS) return TEE_ERROR_BAD_PARAMETERS;
    // sub_signo 
    if(sub_signo>=N_SESS) return TEE_ERROR_BAD_PARAMETERS;

    wasm_module_inst_t inst= wasm_runtime_get_module_inst(exec_env);
    // validate buf len ptr
    if(!wasm_runtime_validate_app_addr(inst,dst_buf_len_addr,sizeof(uint32_t))){
        return TEE_ERROR_ACCESS_DENIED;
    }
    uint32_t * buf_len = wasm_runtime_addr_app_to_native(inst,dst_buf_len_addr);
    // validate buf ptr 
    if(!wasm_runtime_validate_app_addr(inst,dst_buf_addr,*buf_len)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    void * buf = wasm_runtime_addr_app_to_native(inst,dst_buf_addr);

    /**
     * Poll stage
     */
    param_types=TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                TEE_PARAM_TYPE_NONE,
                                TEE_PARAM_TYPE_NONE,
                                TEE_PARAM_TYPE_NONE);
    params[0].value.a=signo;
    params[0].value.b=sub_signo;
    // poll to io helper sub signo until ready
    // TODO: do we need timeout?
    
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_POLL_SUBSIGNAL,param_types,params);
    while(res==TEE_SUCCESS){
        // get poll return
        // return 1, subsignal is ready
        if(params[0].value.a>0) break;
        // reset params!!!
        params[0].value.a=signo;
        params[0].value.b=sub_signo;
        res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_POLL_SUBSIGNAL,param_types,params);
        // poll_times++;
    }
    if(res!=TEE_SUCCESS){
        DMSG("poll subsignal pta io helper failed");
        return res;
    }

#ifdef OVERHEAD_BENCHMARK   
    record_benchmark_time_wrapper(exec_env,3);
#endif
    

    /**
     * Get data stage
     */
    param_types=TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                TEE_PARAM_TYPE_NONE,
                                TEE_PARAM_TYPE_NONE);
    // for now signo is periph no
    params[0].value.a=signo;
    params[0].value.b=sub_signo;
    // set buf and len
    params[1].memref.buffer=buf;
    params[1].memref.size=*buf_len;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_GET_DATA,param_types,params);
    // return poll_times; // test only
    if(res==TEE_ERROR_SHORT_BUFFER){
        // get buf len needed
        // modified buf len to suggest len needed
        *buf_len=params[1].memref.size;
        return TEE_ERROR_SHORT_BUFFER;
    }else if(res!=TEE_SUCCESS){
        // error get req result
        return TEE_ERROR_NO_DATA; 
    }
    // success get req result, but req falied
    // req return value is params[0].value.a
    if(((int)params[0].value.a)<0){
        DMSG("get io req result success, but io req failed with %d",(int)params[0].value.a);
        return params[0].value.a;
    }
    // success get req result, req success
    // return value is ret, usually suggest real data len we get
    // return (int)params[0].value.a;

    // for test, we return params[0].value.b here, which is poll times
    return params[0].value.b;
}

static int get_benchmark_time_wrapper(wasm_exec_env_t exec_env, uint32_t idx, uint32_t out_time_addr){
    if(idx>=N_TIMES){return TEE_ERROR_BAD_PARAMETERS;}
    wasm_module_inst_t inst= wasm_runtime_get_module_inst(exec_env);
    // validate out time ptr
    if(!wasm_runtime_validate_app_addr(inst,out_time_addr,sizeof(uint64_t))){
        return TEE_ERROR_ACCESS_DENIED;
    }
    uint64_t * out_time = wasm_runtime_addr_app_to_native(inst,out_time_addr);
    *out_time=teetime_to_micro(times[idx]);
    return TEE_SUCCESS;
}

static int test_ta_get_sys_time_overhead_wrapper(wasm_exec_env_t exec_env){
    TEE_Time t0,t1;
    uint64_t start,end;
    TEE_GetSystemTime(&t0);
    TEE_GetSystemTime(&t1); 
    start=teetime_to_micro(t0);
    end=teetime_to_micro(t1);
    return end-start;
}

static int test_tee_get_sys_time_overhead_wrapper(wasm_exec_env_t exec_env){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    return invoke_dht_pta(PTA_CMD_TEST_GET_SYS_TIME_OVERHEAD,param_types,params);
}

static int bh1750fvi_init_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t addr)
{
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a = (uint32_t)i2c_no;
    params[0].value.b = (uint32_t)addr;
    res = invoke_bh1750fvi_pta(PTA_CMD_BH1750FVI_INIT,param_types,params);
    return res;
}

static int bh1750fvi_sample_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t addr,uint32_t dst_lux_addr){
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;

#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(dst_lux_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    dst_lux_addr=clear_signature_and_tag(dst_lux_addr);
#endif

    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(!wasm_runtime_validate_app_addr(inst,dst_lux_addr,sizeof(uint16_t))){
        return TEE_ERROR_ACCESS_DENIED;
    }
    uint16_t * lux = wasm_runtime_addr_app_to_native(inst,dst_lux_addr);

    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a = (uint32_t)i2c_no;
    params[0].value.b = (uint32_t)addr;
    res = invoke_bh1750fvi_pta(PTA_CMD_BH1750FVI_SAMPLE,param_types,params);
    if(res!=TEE_SUCCESS) return res;
    *lux=(uint16_t)params[0].value.a;
    return TEE_SUCCESS;
}

static int mpu6050_init_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a=i2c_no;
    params[0].value.b=i2c_addr;
    return invoke_mpu6050_pta(PTA_CMD_MPU6050_INIT,param_types,params);
}

static int mpu6050_deinit_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a=i2c_no;
    params[0].value.b=i2c_addr;
    return invoke_mpu6050_pta(PTA_CMD_MPU6050_DEINIT,param_types,params);
}

static int mpu6050_sample_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr, uint32_t dst_addr){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(dst_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    dst_addr=clear_signature_and_tag(dst_addr);
#endif
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    // need 14 byte for mpu6050 data
    if(!wasm_runtime_validate_app_addr(inst,dst_addr,14)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    uint8_t * mpu6050_data=wasm_runtime_addr_app_to_native(inst,dst_addr);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a=i2c_no;
    params[0].value.b=i2c_addr;
    params[1].memref.buffer=(void *)mpu6050_data;
    params[1].memref.size=14;
    return invoke_mpu6050_pta(PTA_CMD_MPU6050_SAMPLE,param_types,params);
}

static int w25qxx_init_wrapper(wasm_exec_env_t exec_env, uint32_t spi_no,uint32_t spi_cs_no){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    
    params[0].value.a=spi_no;
    params[0].value.b=spi_cs_no;
    return invoke_w25qxx_pta(PTA_CMD_W25QXX_INIT,param_types,params);
}

static int w25qxx_read_id_wrapper(wasm_exec_env_t exec_env, uint32_t spi_no,uint32_t spi_cs_no, uint32_t id_dst_addr){
    TEE_Result res;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(id_dst_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    id_dst_addr=clear_signature_and_tag(id_dst_addr);
#endif
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(!wasm_runtime_validate_app_addr(inst,id_dst_addr,sizeof(uint16_t))){
        return TEE_ERROR_ACCESS_DENIED;
    }
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    uint16_t * id=wasm_runtime_addr_app_to_native(inst,id_dst_addr);
    params[0].value.a=spi_no;
    params[0].value.b=spi_cs_no;
    res=invoke_w25qxx_pta(PTA_CMD_W25QXX_READ_ID,param_types,params);
    if(res!=TEE_SUCCESS){
        return res;
    }
    *id=(uint16_t)params[0].value.a;
    return TEE_SUCCESS;
}

static int w25qxx_read_data_wrapper(wasm_exec_env_t exec_env, uint32_t spi_no,uint32_t spi_cs_no,
                                    uint32_t read_addr, uint32_t dst_buf_addr, uint32_t len){
    TEE_Result res;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(dst_buf_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    dst_buf_addr=clear_signature_and_tag(dst_buf_addr);
#endif
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(!wasm_runtime_validate_app_addr(inst,dst_buf_addr,len)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                  TEE_PARAM_TYPE_NONE);
    void * buf=wasm_runtime_addr_app_to_native(inst,dst_buf_addr);
    params[0].value.a=spi_no;
    params[0].value.b=spi_cs_no;
    params[1].value.a=read_addr;
    params[2].memref.buffer=buf;
    params[2].memref.size=len;
    return invoke_w25qxx_pta(PTA_CMD_W25QXX_READ_DATA,param_types,params);
}

static int w25qxx_page_program_wrapper(wasm_exec_env_t exec_env, uint32_t spi_no, uint32_t spi_cs_no, uint32_t write_addr, uint32_t dst_buf_addr, uint32_t len){
    TEE_Result res;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(dst_buf_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    dst_buf_addr=clear_signature_and_tag(dst_buf_addr);
#endif
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(!wasm_runtime_validate_app_addr(inst,dst_buf_addr,len)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_MEMREF_INPUT,
                                  TEE_PARAM_TYPE_NONE);
    uint8_t * input_data_buf=wasm_runtime_addr_app_to_native(inst,dst_buf_addr);
    params[0].value.a=spi_no;
    params[0].value.b=spi_cs_no;
    params[1].value.a=write_addr;
    params[2].memref.buffer=input_data_buf;
    params[2].memref.size=len;
    return invoke_w25qxx_pta(PTA_CMD_W25QXX_PAGE_PROGRAM,param_types,params);
}

static int w25qxx_sector_erase_wrapper(wasm_exec_env_t exec_env, uint32_t spi_no, uint32_t spi_cs_no, uint32_t sector_addr){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    
    params[0].value.a=spi_no;
    params[0].value.b=spi_cs_no;
    params[1].value.a=sector_addr;
    return invoke_w25qxx_pta(PTA_CMD_W25QXX_SECTOR_ERASE,param_types,params);
}

static int w25qxx_chip_erase_wrapper(wasm_exec_env_t exec_env, uint32_t spi_no, uint32_t spi_cs_no){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    
    params[0].value.a=spi_no;
    params[0].value.b=spi_cs_no;
    return invoke_w25qxx_pta(PTA_CMD_W25QXX_CHIP_ERASE,param_types,params);
}


static int tel0157_init_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr, uint32_t gnss_mode, uint32_t rgb){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a=i2c_no;
    params[0].value.b=i2c_addr;
    params[1].value.a=gnss_mode;
    params[1].value.b=rgb;
    return invoke_tel0157_pta(PTA_CMD_TEL0157_INIT,param_types,params);
}

static int tel0157_deinit_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a=i2c_no;
    params[0].value.b=i2c_addr;
    return invoke_tel0157_pta(PTA_CMD_TEL0157_DEINIT,param_types,params);
}

static int tel0157_get_gnss_len_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr, uint32_t len_dst_addr){
    TEE_Result res;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(len_dst_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    len_dst_addr=clear_signature_and_tag(len_dst_addr);
#endif
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(!wasm_runtime_validate_app_addr(inst,len_dst_addr,sizeof(uint16_t))){
        return TEE_ERROR_ACCESS_DENIED;
    }
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    uint16_t * len=wasm_runtime_addr_app_to_native(inst,len_dst_addr);
    params[0].value.a=i2c_no;
    params[0].value.b=i2c_addr;
    res=invoke_tel0157_pta(PTA_CMD_TEL0157_GET_GNSS_LEN,param_types,params);
    if(res!=TEE_SUCCESS){
        return res;
    }
    *len=(uint16_t)params[0].value.a;
    return TEE_SUCCESS;
}

static int tel0157_get_all_gnss_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr, uint32_t dst_buf_addr, uint32_t len){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(dst_buf_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    dst_buf_addr=clear_signature_and_tag(dst_buf_addr);
#endif
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(!wasm_runtime_validate_app_addr(inst,dst_buf_addr,len)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    uint8_t * buf=wasm_runtime_addr_app_to_native(inst,dst_buf_addr);
    params[0].value.a=i2c_no;
    params[0].value.b=i2c_addr;
    params[1].memref.buffer=(void *)buf;
    params[1].memref.size=len;
    return invoke_tel0157_pta(PTA_CMD_TEL0157_GET_ALL_GNSS,param_types,params);
}

static int tel0157_get_utc_time_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr, uint32_t utc_dst_addr){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(utc_dst_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    utc_dst_addr=clear_signature_and_tag(utc_dst_addr);
#endif
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(!wasm_runtime_validate_app_addr(inst,utc_dst_addr,sizeof(tel0157_time_t))){
        return TEE_ERROR_ACCESS_DENIED;
    }
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    void * buf=wasm_runtime_addr_app_to_native(inst,utc_dst_addr);
    params[0].value.a=i2c_no;
    params[0].value.b=i2c_addr;
    params[1].memref.buffer=buf;
    params[1].memref.size=sizeof(tel0157_time_t);
    return invoke_tel0157_pta(PTA_CMD_TEL0157_GET_UTC_TIME,param_types,params);
}

static int tel0157_get_gnss_mode_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr, uint32_t mode_dst_addr){
    TEE_Result res;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(mode_dst_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    mode_dst_addr=clear_signature_and_tag(mode_dst_addr);
#endif
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(!wasm_runtime_validate_app_addr(inst,mode_dst_addr,sizeof(uint8_t))){
        return TEE_ERROR_ACCESS_DENIED;
    }
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    uint8_t * mode=wasm_runtime_addr_app_to_native(inst,mode_dst_addr);
    params[0].value.a=i2c_no;
    params[0].value.b=i2c_addr;
    res=invoke_tel0157_pta(PTA_CMD_TEL0157_GET_GNSS_MODE,param_types,params);
    if(res!=TEE_SUCCESS){
        return res;
    }
    *mode=(uint8_t)params[0].value.a;
    return res;
}

static int tel0157_get_num_sat_used_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr, uint32_t num_dst_addr){
    TEE_Result res;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(num_dst_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    num_dst_addr=clear_signature_and_tag(num_dst_addr);
#endif
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(!wasm_runtime_validate_app_addr(inst,num_dst_addr,sizeof(uint8_t))){
        return TEE_ERROR_ACCESS_DENIED;
    }
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    uint8_t * num=wasm_runtime_addr_app_to_native(inst,num_dst_addr);
    params[0].value.a=i2c_no;
    params[0].value.b=i2c_addr;
    res=invoke_tel0157_pta(PTA_CMD_TEL0157_GET_NUM_SAT_USED,param_types,params);
    if(res!=TEE_SUCCESS){
        return res;
    }
    *num=(uint8_t)params[0].value.a;
    return res;
}

static int tel0157_get_lon_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr, uint32_t lon_dst_addr){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(lon_dst_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    lon_dst_addr=clear_signature_and_tag(lon_dst_addr);
#endif
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(!wasm_runtime_validate_app_addr(inst,lon_dst_addr,sizeof(tel0157_lon_lat_t))){
        return TEE_ERROR_ACCESS_DENIED;
    }
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    void * lon=wasm_runtime_addr_app_to_native(inst,lon_dst_addr);
    params[0].value.a=i2c_no;
    params[0].value.b=i2c_addr;
    params[1].memref.buffer=lon;
    params[1].memref.size=sizeof(tel0157_lon_lat_t);
    return invoke_tel0157_pta(PTA_CMD_TEL0157_GET_LON,param_types,params);
}

static int tel0157_get_lat_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr, uint32_t lat_dst_addr){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(lat_dst_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    lat_dst_addr=clear_signature_and_tag(lat_dst_addr);
#endif
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(!wasm_runtime_validate_app_addr(inst,lat_dst_addr,sizeof(tel0157_lon_lat_t))){
        return TEE_ERROR_ACCESS_DENIED;
    }
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    void * lat=wasm_runtime_addr_app_to_native(inst,lat_dst_addr);
    params[0].value.a=i2c_no;
    params[0].value.b=i2c_addr;
    params[1].memref.buffer=lat;
    params[1].memref.size=sizeof(tel0157_lon_lat_t);
    return invoke_tel0157_pta(PTA_CMD_TEL0157_GET_LAT,param_types,params);
}

static int tel0157_get_alt_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr, uint32_t alt_dst_addr){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;

#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(alt_dst_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    alt_dst_addr=clear_signature_and_tag(alt_dst_addr);
#endif

    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    // need 3 bytes
    if(!wasm_runtime_validate_app_addr(inst,alt_dst_addr,3)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    void * alt=wasm_runtime_addr_app_to_native(inst,alt_dst_addr);
    params[0].value.a=i2c_no;
    params[0].value.b=i2c_addr;
    params[1].memref.buffer=alt;
    params[1].memref.size=3;
    return invoke_tel0157_pta(PTA_CMD_TEL0157_GET_ALT,param_types,params);
}

static int tel0157_get_sog_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr, uint32_t sog_dst_addr){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(sog_dst_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    sog_dst_addr=clear_signature_and_tag(sog_dst_addr);
#endif
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    // need 3 bytes
    if(!wasm_runtime_validate_app_addr(inst,sog_dst_addr,3)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    void * sog=wasm_runtime_addr_app_to_native(inst,sog_dst_addr);
    params[0].value.a=i2c_no;
    params[0].value.b=i2c_addr;
    params[1].memref.buffer=sog;
    params[1].memref.size=3;
    return invoke_tel0157_pta(PTA_CMD_TEL0157_GET_SOG,param_types,params);
}

static int tel0157_get_cog_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr, uint32_t cog_dst_addr){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(cog_dst_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    cog_dst_addr=clear_signature_and_tag(cog_dst_addr);
#endif
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    // need 3 bytes
    if(!wasm_runtime_validate_app_addr(inst,cog_dst_addr,3)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    void * cog=wasm_runtime_addr_app_to_native(inst,cog_dst_addr);
    params[0].value.a=i2c_no;
    params[0].value.b=i2c_addr;
    params[1].memref.buffer=cog;
    params[1].memref.size=3;
    return invoke_tel0157_pta(PTA_CMD_TEL0157_GET_COG,param_types,params);
}


static int spi_transfer_byte_wrapper(wasm_exec_env_t exec_env, uint32_t spi_no,uint32_t spi_cs_no, uint32_t cont,
                             uint32_t mode, uint32_t freq,  uint32_t need_byte_out, uint32_t need_byte_in,
                              uint32_t byte_out, uint32_t dst_byte_addr){
    TEE_Result res;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint8_t * byte_in=NULL;

#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(dst_byte_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    dst_byte_addr=clear_signature_and_tag(dst_byte_addr);
#endif

    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(need_byte_in>0){
        if(!wasm_runtime_validate_app_addr(inst,dst_byte_addr,1)){
            return TEE_ERROR_ACCESS_DENIED;
        }
        byte_in=wasm_runtime_addr_app_to_native(inst,dst_byte_addr);  
    }
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                    TEE_PARAM_TYPE_VALUE_INPUT,
                                    TEE_PARAM_TYPE_VALUE_INPUT,
                                    TEE_PARAM_TYPE_VALUE_INPUT);
    params[0].value.a=spi_no<<16 | (spi_cs_no&0xffff);
    params[0].value.b=cont;
    params[1].value.a=mode;
    params[1].value.b=freq;
    params[2].value.a=need_byte_out;
    params[2].value.b=need_byte_in;
    params[3].value.a=byte_out;
    res=invoke_spi_pta(PTA_CMD_SPI_TRANSFER_BYTE,param_types,params); 
    if(res!=TEE_SUCCESS){
        return res;
    }
    if(need_byte_in>0){
        *byte_in=(uint8_t)params[0].value.a;
    }      
    return TEE_SUCCESS;  
}

static int spi_transfer_bytes_wrapper(wasm_exec_env_t exec_env, uint32_t spi_no,uint32_t spi_cs_no, uint32_t cont,
                             uint32_t mode, uint32_t freq,  uint32_t src_len, uint32_t dst_len,
                              uint32_t src_buf_addr, uint32_t dst_buf_addr){
    TEE_Result res;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint8_t * src_buf=NULL;
    uint8_t * dst_buf=NULL;
#ifdef USE_FAT_PTR_CHECK
    if(dst_buf_addr!=0){
        if(!check_ptr_signed(dst_buf_addr)){
            return TEE_ERROR_ACCESS_DENIED;
        }
        dst_buf_addr=clear_signature_and_tag(dst_buf_addr);
    }
    if(src_buf_addr!=0){
        if(!check_ptr_signed(src_buf_addr)){
            return TEE_ERROR_ACCESS_DENIED;
        }
        src_buf_addr=clear_signature_and_tag(src_buf_addr);
    }
#endif
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(src_len>0){
        if(!wasm_runtime_validate_app_addr(inst,src_buf_addr,src_len)){
            return TEE_ERROR_ACCESS_DENIED;
        }
        src_buf=wasm_runtime_addr_app_to_native(inst,src_buf_addr);  
    }
    if(dst_len>0){
        if(!wasm_runtime_validate_app_addr(inst,dst_buf_addr,dst_len)){
            return TEE_ERROR_ACCESS_DENIED;
        }
        dst_buf=wasm_runtime_addr_app_to_native(inst,dst_buf_addr);
    }
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                    TEE_PARAM_TYPE_VALUE_INPUT,
                                    TEE_PARAM_TYPE_MEMREF_INPUT,
                                    TEE_PARAM_TYPE_MEMREF_OUTPUT);
    params[0].value.a=spi_no<<16 | (spi_cs_no&0xffff);
    params[0].value.b=cont;
    params[1].value.a=mode;
    params[1].value.b=freq;
    params[2].memref.buffer=src_buf;
    params[2].memref.size=src_len;
    params[3].memref.buffer=dst_buf;
    params[3].memref.size=dst_len;
    
    return invoke_spi_pta(PTA_CMD_SPI_TRANSFER_BYTES,param_types,params);  
}

static int spi_transfer_diff_bytes_wrapper(wasm_exec_env_t exec_env, uint32_t spi_no,uint32_t spi_cs_no, uint32_t cont,
                             uint32_t mode, uint32_t freq,  uint32_t src_len, uint32_t dst_len,
                              uint32_t src_buf_addr, uint32_t dst_buf_addr){
    TEE_Result res;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint8_t * src_buf=NULL;
    uint8_t * dst_buf=NULL;
#ifdef USE_FAT_PTR_CHECK
    if(dst_buf_addr!=0){
        if(!check_ptr_signed(dst_buf_addr)){
            return TEE_ERROR_ACCESS_DENIED;
        }
        dst_buf_addr=clear_signature_and_tag(dst_buf_addr);
    }
    if(src_buf_addr!=0){
        if(!check_ptr_signed(src_buf_addr)){
            return TEE_ERROR_ACCESS_DENIED;
        }
        src_buf_addr=clear_signature_and_tag(src_buf_addr);
    }
#endif
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(src_len>0){
        if(!wasm_runtime_validate_app_addr(inst,src_buf_addr,src_len)){
            return TEE_ERROR_ACCESS_DENIED;
        }
        src_buf=wasm_runtime_addr_app_to_native(inst,src_buf_addr);  
    }
    if(dst_len>0){
        if(!wasm_runtime_validate_app_addr(inst,dst_buf_addr,dst_len)){
            return TEE_ERROR_ACCESS_DENIED;
        }
        dst_buf=wasm_runtime_addr_app_to_native(inst,dst_buf_addr);
    }
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                    TEE_PARAM_TYPE_VALUE_INPUT,
                                    TEE_PARAM_TYPE_MEMREF_INPUT,
                                    TEE_PARAM_TYPE_MEMREF_OUTPUT);
    params[0].value.a=spi_no<<16 | (spi_cs_no&0xffff);
    params[0].value.b=cont;
    params[1].value.a=mode;
    params[1].value.b=freq;
    params[2].memref.buffer=src_buf;
    params[2].memref.size=src_len;
    params[3].memref.buffer=dst_buf;
    params[3].memref.size=dst_len;
    
    return invoke_spi_pta(PTA_CMD_SPI_TRANSFER_DIFF_BYTES,param_types,params);  
}

static int write_secure_object_wrapper(wasm_exec_env_t exec_env, uint32_t id_addr, uint32_t id_len, uint32_t data_addr, uint32_t data_len){
    TEE_Result res;
    TEE_ObjectHandle object;
    uint32_t obj_data_flag;
    char * id;
    char * data;
#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(id_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    id_addr=clear_signature_and_tag(id_addr);
    if(!check_ptr_signed(data_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    data_addr=clear_signature_and_tag(data_addr);
#endif
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(!wasm_runtime_validate_app_addr(inst,data_addr,data_len)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    if(!wasm_runtime_validate_app_addr(inst,id_addr,id_len)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    id=wasm_runtime_addr_app_to_native(inst,id_addr);
    data=wasm_runtime_addr_app_to_native(inst,data_addr);
    obj_data_flag = TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE | TEE_DATA_FLAG_ACCESS_WRITE_META | TEE_DATA_FLAG_OVERWRITE;
    res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE_RPMB, id, id_len, obj_data_flag, TEE_HANDLE_NULL, 
                                    NULL,0,&object);
    if(res!=TEE_SUCCESS){
        return res;
    }
    res = TEE_WriteObjectData(object,data, data_len);
    if(res!=TEE_SUCCESS){
        TEE_CloseAndDeletePersistentObject(object);
    }else{
        TEE_CloseObject(object);
    }
    return res;
}

static int read_secure_object_wrapper(wasm_exec_env_t exec_env, uint32_t id_addr, uint32_t id_len, uint32_t data_addr, uint32_t data_len_addr){
    TEE_Result res;
    TEE_ObjectHandle object;
    TEE_ObjectInfo info;
    char * id;
    char * data;
    uint32_t * data_len;
    char * buf;
#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(data_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    data_addr=clear_signature_and_tag(data_addr);

    if(!check_ptr_signed(data_len_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    data_len_addr=clear_signature_and_tag(data_len_addr);
    
    if(!check_ptr_signed(id_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    id_addr=clear_signature_and_tag(id_addr);
#endif
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(!wasm_runtime_validate_app_addr(inst,data_len_addr,sizeof(uint32_t))){
        return TEE_ERROR_ACCESS_DENIED;
    }
    data_len=wasm_runtime_addr_app_to_native(inst,data_len_addr);

    if(!wasm_runtime_validate_app_addr(inst,data_addr,*data_len)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    if(!wasm_runtime_validate_app_addr(inst,id_addr,id_len)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    id=wasm_runtime_addr_app_to_native(inst,id_addr);
    data=wasm_runtime_addr_app_to_native(inst,data_addr);
    buf=(char*)malloc(*data_len);
    if(!buf){
        return TEE_ERROR_OUT_OF_MEMORY;
    }
    res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE_RPMB, id, id_len, 
            TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_SHARE_READ, &object);
    if(res!=TEE_SUCCESS){
        TEE_Free(buf);
        return res;
    }
    res = TEE_GetObjectInfo1(object,&info);
    if(res!=TEE_SUCCESS){
        TEE_Free(buf);
        return res;
    }
    if(info.dataSize>*data_len){
        *data_len=info.dataSize;
        TEE_CloseObject(object);
        TEE_Free(buf);
        return TEE_ERROR_SHORT_BUFFER;
    }
    res = TEE_ReadObjectData(object,buf,info.dataSize,data_len);
    if(res==TEE_SUCCESS){
        TEE_MemMove(data,buf,*data_len);
    }
    if(res!=TEE_SUCCESS||*data_len!=info.dataSize){
        TEE_CloseObject(object);
        TEE_Free(buf);
        return res;
    }
    TEE_CloseObject(object);
    TEE_Free(buf);
    return res;
}

static int delete_secure_object_wrapper(wasm_exec_env_t exec_env, uint32_t id_addr, uint32_t id_len){
    TEE_Result res;
    TEE_ObjectHandle object;
    char * id;
#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(id_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    id_addr=clear_signature_and_tag(id_addr);
#endif
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(!wasm_runtime_validate_app_addr(inst,id_addr,id_len)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    id=wasm_runtime_addr_app_to_native(inst,id_addr);
    res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE_RPMB, id, id_len, 
            TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE_META, &object);
    if(res!=TEE_SUCCESS){
        return res;
    }
    TEE_CloseAndDeletePersistentObject(object);
    return res;
}

static int at24cxx_read_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr, uint32_t read_addr, uint32_t dst_buf_addr, uint32_t len){
    TEE_Result res;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint8_t * buf;

#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(dst_buf_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    dst_buf_addr=clear_signature_and_tag(dst_buf_addr);
#endif

    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(!wasm_runtime_validate_app_addr(inst,dst_buf_addr,len)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    buf=wasm_runtime_addr_app_to_native(inst,dst_buf_addr);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a=i2c_no;
    params[0].value.b=i2c_addr;
    params[1].value.a=read_addr;
    params[2].memref.buffer=buf;
    params[2].memref.size=len;
    res=invoke_at24cxx_pta(PTA_CMD_AT24CXX_READ,param_types,params);
    return res;
}

static int at24cxx_write_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no,uint32_t i2c_addr, uint32_t write_addr, uint32_t src_buf_addr, uint32_t len){
    TEE_Result res;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint8_t * buf;

#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(src_buf_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    src_buf_addr=clear_signature_and_tag(src_buf_addr);
#endif

    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(!wasm_runtime_validate_app_addr(inst,src_buf_addr,len)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    buf=wasm_runtime_addr_app_to_native(inst,src_buf_addr);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_MEMREF_INPUT,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a=i2c_no;
    params[0].value.b=i2c_addr;
    params[1].value.a=write_addr;
    params[2].memref.buffer=buf;
    params[2].memref.size=len;
    res=invoke_at24cxx_pta(PTA_CMD_AT24CXX_WRITE,param_types,params);
    return res;
}

static int atk301_init_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no, uint32_t i2c_addr){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a=(uint32_t)i2c_no;
    params[0].value.b=(uint32_t)i2c_addr;
    return invoke_atk301_pta(PTA_CMD_ATK301_INIT,param_types,params);
}

static int atk301_get_fingerprint_image_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no, uint32_t i2c_addr){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a=(uint32_t)i2c_no;
    params[0].value.b=(uint32_t)i2c_addr;
    return invoke_atk301_pta(PTA_CMD_ATK301_GET_FINGERPRINT_IMAGE,param_types,params);
}

static int atk301_upload_fingerprint_image_wrapper(wasm_exec_env_t exec_env, uint32_t i2c_no, uint32_t i2c_addr, uint32_t dst_buf_addr){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    uint8_t * buf;
#ifdef USE_FAT_PTR_CHECK
    if(!check_ptr_signed(dst_buf_addr)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    dst_buf_addr=clear_signature_and_tag(dst_buf_addr);
#endif
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if(!wasm_runtime_validate_app_addr(inst,dst_buf_addr,ATK301_FINGERPRINT_IMAGE_DATA_LEN)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    buf=wasm_runtime_addr_app_to_native(inst,dst_buf_addr);
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a=(uint32_t)i2c_no;
    params[0].value.b=(uint32_t)i2c_addr;
    params[1].memref.buffer=buf;
    params[1].memref.size=ATK301_FINGERPRINT_IMAGE_DATA_LEN;
    return invoke_atk301_pta(PTA_CMD_ATK301_UPLOAD_FINGERPRINT_IMAGE,param_types,params);
}

static uint32_t sign_ptr_wrapper(wasm_exec_env_t exec_env, uint32_t original_ptr, uint32_t signature, uint32_t tag){
    uint32_t fat_ptr = original_ptr & FAT_PTR_ADDRESS_MASK;
    // 8 bits signature & 7 bits tag
    fat_ptr |= ((((uint32_t)(signature & 0xFF))<<24) | (((uint32_t)(tag & 0x7F))<<17));
    return fat_ptr;
}


static int promise_set_callback_wrapper(wasm_exec_env_t exec_env, int promise, uint32_t handler_func_ptr){
    // high 16 bits is periph_no
    // low 16 bits is sub signo
    uint16_t periph_no= (uint16_t)(promise>>16);
    uint16_t sub_signo= (uint16_t)(promise&0xFFFF);

    if(periph_no>=N_SESS) return TEE_ERROR_BAD_PARAMETERS;
    // sub_signo 
    if(sub_signo>=N_SESS) return TEE_ERROR_BAD_PARAMETERS;
    // poll sub signal once to check if the sub signo is using
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    param_types=TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                TEE_PARAM_TYPE_NONE,
                                TEE_PARAM_TYPE_NONE,
                                TEE_PARAM_TYPE_NONE);
    params[0].value.a=periph_no;
    params[0].value.b=sub_signo;

    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_POLL_SUBSIGNAL,param_types,params);
    if(res!=TEE_SUCCESS){
        // sub signal is not used or io helper error
        return TEE_ERROR_ACCESS_DENIED;
    }
    // sub signo is used
    // set callback
    if(tzio_register_handler(exec_env,(uint32_t)periph_no,(uint32_t)sub_signo,handler_func_ptr,(uint32_t)REQ_TYPE_NONE,0)<0){
        EMSG("register handler to sigtable failed");
        return TEE_ERROR_GENERIC;
    }
    return TEE_SUCCESS;
}

static int get_async_data_promise_wrapper(wasm_exec_env_t exec_env, int promise, uint32_t dst_buf_addr,uint32_t dst_buf_len_addr){
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    // high 16 bits is periph_no
    // low 16 bits is sub signo
    uint16_t periph_no= (uint16_t)(promise>>16);
    uint16_t sub_signo= (uint16_t)(promise&0xFFFF);

    if(periph_no>=N_SESS) return TEE_ERROR_BAD_PARAMETERS;
    // sub_signo 
    if(sub_signo>=N_SESS) return TEE_ERROR_BAD_PARAMETERS;

#ifdef USE_FAT_PTR_CHECK
    if(dst_buf_addr!=0){
        if(!check_ptr_signed(dst_buf_addr)){
            return TEE_ERROR_ACCESS_DENIED;
        }
        dst_buf_addr=clear_signature_and_tag(dst_buf_addr);
    }
    if(dst_buf_len_addr!=0){
        if(!check_ptr_signed(dst_buf_len_addr)){
            return TEE_ERROR_ACCESS_DENIED;
        }
        dst_buf_len_addr=clear_signature_and_tag(dst_buf_len_addr);
    }
#endif
    // int poll_times=1;
    // signo is periph_no
    if(periph_no>=N_SESS) return TEE_ERROR_BAD_PARAMETERS;
    // sub_signo 
    if(sub_signo>=N_SESS) return TEE_ERROR_BAD_PARAMETERS;

    wasm_module_inst_t inst= wasm_runtime_get_module_inst(exec_env);
    // validate buf len ptr
    if(!wasm_runtime_validate_app_addr(inst,dst_buf_len_addr,sizeof(uint32_t))){
        return TEE_ERROR_ACCESS_DENIED;
    }
    uint32_t * buf_len = wasm_runtime_addr_app_to_native(inst,dst_buf_len_addr);
    // validate buf ptr 
    if(!wasm_runtime_validate_app_addr(inst,dst_buf_addr,*buf_len)){
        return TEE_ERROR_ACCESS_DENIED;
    }
    void * buf = wasm_runtime_addr_app_to_native(inst,dst_buf_addr);

    /**
     * Poll stage
     */
    param_types=TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                TEE_PARAM_TYPE_NONE,
                                TEE_PARAM_TYPE_NONE,
                                TEE_PARAM_TYPE_NONE);
    params[0].value.a=periph_no;
    params[0].value.b=sub_signo;
    // poll to io helper sub signo until ready
    // TODO: do we need timeout?
    
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_POLL_SUBSIGNAL,param_types,params);
    while(res==TEE_SUCCESS){
        // get poll return
        // return 1, subsignal is ready
        if(params[0].value.a>0) break;
        // reset params!!!
        params[0].value.a=periph_no;
        params[0].value.b=sub_signo;
        res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_POLL_SUBSIGNAL,param_types,params);
        // poll_times++;
    }
    if(res!=TEE_SUCCESS){
        DMSG("poll subsignal pta io helper failed");
        return res;
    }

    // test only
    // record_benchmark_time_wrapper(exec_env,3);

    /**
     * Get data stage
     */
    param_types=TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                TEE_PARAM_TYPE_NONE,
                                TEE_PARAM_TYPE_NONE);
    params[0].value.a=periph_no;
    params[0].value.b=sub_signo;
    // set buf and len
    params[1].memref.buffer=buf;
    params[1].memref.size=*buf_len;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_GET_DATA,param_types,params);
    // return poll_times; // test only
    if(res==TEE_ERROR_SHORT_BUFFER){
        // get buf len needed
        // modified buf len to suggest len needed
        *buf_len=params[1].memref.size;
        return TEE_ERROR_SHORT_BUFFER;
    }else if(res!=TEE_SUCCESS){
        // error get req result
        return TEE_ERROR_NO_DATA; 
    }
    // success get req result, but req falied
    // req return value is params[0].value.a
    if(((int)params[0].value.a)<0){
        DMSG("get io req result success, but io req failed with %d",(int)params[0].value.a);
        return params[0].value.a;
    }
    // success get req result, req success
    // return value is ret, usually suggest real data len we get
    return (int)params[0].value.a;
}

static int record_poll_times_wrapper(wasm_exec_env_t exec_env,uint32_t times){
    global_poll_times+=times;
    return 0;
}

static int get_poll_times_wrapper(wasm_exec_env_t exec_env, uint32_t dst_poll_times_addr){
    wasm_module_inst_t inst= wasm_runtime_get_module_inst(exec_env);
    // validate poll times ptr
    if(!wasm_runtime_validate_app_addr(inst,dst_poll_times_addr,sizeof(uint64_t))){
        return TEE_ERROR_ACCESS_DENIED;
    }
    uint64_t * poll_times = wasm_runtime_addr_app_to_native(inst,dst_poll_times_addr);
    *poll_times=global_poll_times;
    // reset
    global_poll_times=0;
    return 0;
}

static int get_runtime_poll_times_wrapper(wasm_exec_env_t exec_env, uint32_t dst_poll_times_addr){
    wasm_module_inst_t inst= wasm_runtime_get_module_inst(exec_env);
    // validate poll times ptr
    if(!wasm_runtime_validate_app_addr(inst,dst_poll_times_addr,sizeof(uint64_t))){
        return TEE_ERROR_ACCESS_DENIED;
    }
    uint64_t * poll_times = wasm_runtime_addr_app_to_native(inst,dst_poll_times_addr);
    *poll_times=tzio_get_poll_times();
    // reset
    tzio_reset_poll_times();
    return 0;
}

static int get_runtime_profile_times_wrapper(wasm_exec_env_t exec_env, uint32_t dst_profile_times_addr){
    wasm_module_inst_t inst= wasm_runtime_get_module_inst(exec_env);
    // validate poll times ptr
    if(!wasm_runtime_validate_app_addr(inst,dst_profile_times_addr,sizeof(uint64_t))){
        return TEE_ERROR_ACCESS_DENIED;
    }
    uint64_t * profile_times = wasm_runtime_addr_app_to_native(inst,dst_profile_times_addr);
    *profile_times=tzio_get_profile_times();
    // reset
    tzio_reset_profile_times();
    return 0;
}


static int all_async_req_handled_wrapper(wasm_exec_env_t exec_env){
    return all_signal_handled();
}

static int i2c_get_overhead_timestamp_wrapper(wasm_exec_env_t exec_env, uint32_t buf_addr){
    TEE_Param params[TEE_NUM_PARAMS]={};
    uint32_t param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    wasm_module_inst_t inst= wasm_runtime_get_module_inst(exec_env);
    // validate poll times ptr
    if(!wasm_runtime_validate_app_addr(inst,buf_addr,100*sizeof(uint64_t))){
        return TEE_ERROR_ACCESS_DENIED;
    }
    uint64_t * buf=wasm_runtime_addr_app_to_native(inst,buf_addr);
    params[0].memref.buffer=buf;
    params[0].memref.size=100*sizeof(uint64_t);
    return invoke_i2c_pta(PTA_I2C_GET_OVERHEAD_TIMESTAMP,param_types,params);
}

static int io_helper_get_overhead_timestamp_wrapper(wasm_exec_env_t exec_env, uint32_t buf_addr){
    TEE_Param params[TEE_NUM_PARAMS]={};
    uint32_t param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE,
                                            TEE_PARAM_TYPE_NONE);
    wasm_module_inst_t inst= wasm_runtime_get_module_inst(exec_env);
    // validate poll times ptr
    if(!wasm_runtime_validate_app_addr(inst,buf_addr,200*sizeof(uint64_t))){
        return TEE_ERROR_ACCESS_DENIED;
    }
    uint64_t * buf=wasm_runtime_addr_app_to_native(inst,buf_addr);
    params[0].memref.buffer=buf;
    params[0].memref.size=200*sizeof(uint64_t);
    return invoke_io_helper_pta(PTA_IO_HELPER_CMD_GET_OVERHEAD_TIMESTAMP,param_types,params);
}

/* clang-format off */
#define REG_NATIVE_FUNC(func_name, signature) \
    { #func_name, func_name##_wrapper, signature, NULL }

static NativeSymbol native_symbols[] = {
    REG_NATIVE_FUNC(io_helper_get_overhead_timestamp,"(i)i"),
    REG_NATIVE_FUNC(all_async_req_handled,"()i"),
    REG_NATIVE_FUNC(get_runtime_profile_times,"(i)i"),
    REG_NATIVE_FUNC(get_runtime_poll_times,"(i)i"),
    REG_NATIVE_FUNC(record_poll_times,"(i)i"),
    REG_NATIVE_FUNC(get_poll_times,"(i)i"),
    REG_NATIVE_FUNC(test_ta_get_sys_time_overhead,"()i"),
    REG_NATIVE_FUNC(test_tee_get_sys_time_overhead,"()i"),
    REG_NATIVE_FUNC(record_benchmark_time,"(i)i"),
    REG_NATIVE_FUNC(get_benchmark_time,"(ii)i"),
    REG_NATIVE_FUNC(sign_ptr,"(iii)i"),
    REG_NATIVE_FUNC(i2c_read_bytes,"(iiiii)i"),
    REG_NATIVE_FUNC(i2c_write_bytes,"(iiiii)i"),
    REG_NATIVE_FUNC(i2c_read_bytes_async,"(iiiii)i"),
    REG_NATIVE_FUNC(i2c_read_bytes_async_future,"(iiii)i"),
    REG_NATIVE_FUNC(i2c_write_bytes_async_future,"(iiiii)i"),
    REG_NATIVE_FUNC(i2c_read_regs,"(iiiiii)i"),
    REG_NATIVE_FUNC(i2c_write_regs,"(iiiiii)i"),
    REG_NATIVE_FUNC(i2c_read_regs_async,"(iiiiii)i"),
    REG_NATIVE_FUNC(i2c_read_regs_async_future,"(iiiii)i"),
    REG_NATIVE_FUNC(i2c_set_freq,"(ii)i"),
    REG_NATIVE_FUNC(i2c_get_overhead_timestamp,"(i)i"),
    REG_NATIVE_FUNC(spi_transfer_bytes,"(iiiiiiiii)i"),
    REG_NATIVE_FUNC(spi_transfer_byte,"(iiiiiiiii)i"),
    REG_NATIVE_FUNC(spi_transfer_diff_bytes,"(iiiiiiiii)i"),
    REG_NATIVE_FUNC(spi_transfer_bytes_async_future,"(iiiiiiii)i"),
    REG_NATIVE_FUNC(write_secure_object, "(iiii)i"),
    REG_NATIVE_FUNC(read_secure_object, "(iiii)i"),
    REG_NATIVE_FUNC(delete_secure_object, "(ii)i"),
    REG_NATIVE_FUNC(write_secure_object_async_future, "(iiii)i"),
    REG_NATIVE_FUNC(read_secure_object_async_future, "(ii)i"),
    REG_NATIVE_FUNC(dht_init, "(ii)i"),
    REG_NATIVE_FUNC(dht_read, "(iiii)i"),
    REG_NATIVE_FUNC(dht_read_async,"(iii)i"),
    REG_NATIVE_FUNC(dht_read_async_future,"(ii)i"),
    REG_NATIVE_FUNC(dht_read_async_promise,"(ii)i"),
    REG_NATIVE_FUNC(bh1750fvi_init,"(ii)i"),
    REG_NATIVE_FUNC(bh1750fvi_sample,"(iii)i"),
    REG_NATIVE_FUNC(bh1750fvi_sample_async,"(iii)i"),
    REG_NATIVE_FUNC(bh1750fvi_sample_async_future,"(ii)i"),
    REG_NATIVE_FUNC(mpu6050_init,"(ii)i"),
    REG_NATIVE_FUNC(mpu6050_deinit,"(ii)i"),
    REG_NATIVE_FUNC(mpu6050_sample,"(iii)i"),
    REG_NATIVE_FUNC(mpu6050_sample_async,"(iii)i"),
    REG_NATIVE_FUNC(mpu6050_sample_async_future,"(ii)i"),
    REG_NATIVE_FUNC(w25qxx_init,"(ii)i"),
    REG_NATIVE_FUNC(w25qxx_read_id,"(iii)i"),
    REG_NATIVE_FUNC(w25qxx_read_data,"(iiiii)i"),
    REG_NATIVE_FUNC(w25qxx_page_program,"(iiiii)i"),
    REG_NATIVE_FUNC(w25qxx_sector_erase,"(iii)i"),
    REG_NATIVE_FUNC(w25qxx_chip_erase,"(ii)i"),
    REG_NATIVE_FUNC(at24cxx_read,"(iiiii)i"),
    REG_NATIVE_FUNC(at24cxx_write,"(iiiii)i"),
    REG_NATIVE_FUNC(at24cxx_read_async,"(iiiii)i"),
    REG_NATIVE_FUNC(at24cxx_read_async_future,"(iiii)i"),
    REG_NATIVE_FUNC(at24cxx_write_async,"(iiiiii)i"),
    REG_NATIVE_FUNC(at24cxx_write_async_future,"(iiiii)i"),
    REG_NATIVE_FUNC(tel0157_init,"(iiii)i"),
    REG_NATIVE_FUNC(tel0157_deinit,"(ii)i"),
    REG_NATIVE_FUNC(tel0157_get_gnss_len,"(iii)i"),
    REG_NATIVE_FUNC(tel0157_get_all_gnss,"(iiii)i"),
    REG_NATIVE_FUNC(tel0157_get_utc_time,"(iii)i"),
    REG_NATIVE_FUNC(tel0157_get_gnss_mode,"(iii)i"),
    REG_NATIVE_FUNC(tel0157_get_num_sat_used,"(iii)i"),
    REG_NATIVE_FUNC(tel0157_get_lon,"(iii)i"),
    REG_NATIVE_FUNC(tel0157_get_lat,"(iii)i"),
    REG_NATIVE_FUNC(tel0157_get_alt,"(iii)i"),
    REG_NATIVE_FUNC(tel0157_get_sog,"(iii)i"),
    REG_NATIVE_FUNC(tel0157_get_cog,"(iii)i"),
    REG_NATIVE_FUNC(tel0157_get_utc_time_async,"(iii)i"),
    REG_NATIVE_FUNC(tel0157_get_lon_async,"(iii)i"),
    REG_NATIVE_FUNC(tel0157_get_lat_async,"(iii)i"),
    REG_NATIVE_FUNC(tel0157_get_alt_async,"(iii)i"),
    REG_NATIVE_FUNC(tel0157_get_gnss_len_async_future,"(ii)i"),
    REG_NATIVE_FUNC(tel0157_get_all_gnss_async_future,"(iii)i"),
    REG_NATIVE_FUNC(tel0157_get_utc_time_async_future,"(ii)i"),
    REG_NATIVE_FUNC(tel0157_get_gnss_mode_async_future,"(ii)i"),
    REG_NATIVE_FUNC(tel0157_get_num_sat_used_async_future,"(ii)i"),
    REG_NATIVE_FUNC(tel0157_get_lon_async_future,"(ii)i"),
    REG_NATIVE_FUNC(tel0157_get_lat_async_future,"(ii)i"),
    REG_NATIVE_FUNC(tel0157_get_alt_async_future,"(ii)i"),
    REG_NATIVE_FUNC(tel0157_get_sog_async_future,"(ii)i"),
    REG_NATIVE_FUNC(tel0157_get_cog_async_future,"(ii)i"),
    REG_NATIVE_FUNC(atk301_init,"(ii)i"),
    REG_NATIVE_FUNC(atk301_get_fingerprint_image,"(ii)i"),
    REG_NATIVE_FUNC(atk301_upload_fingerprint_image,"(iii)i"),
    REG_NATIVE_FUNC(atk301_get_fingerprint_image_async,"(iii)i"),
    REG_NATIVE_FUNC(atk301_upload_fingerprint_image_async,"(iii)i"),
    REG_NATIVE_FUNC(atk301_get_fingerprint_image_async_future,"(ii)i"),
    REG_NATIVE_FUNC(atk301_upload_fingerprint_image_async_future,"(ii)i"),
    REG_NATIVE_FUNC(get_async_data,"(iiii)i"),
    REG_NATIVE_FUNC(get_async_data_future,"(iiii)i"),
    REG_NATIVE_FUNC(promise_set_callback,"(ii)i"),
    REG_NATIVE_FUNC(get_async_data_promise,"(iii)i"),
};
/* clang-format on */

uint32_t
get_native_lib(char **p_module_name, NativeSymbol **p_native_symbols)
{
    *p_module_name = "env";
    *p_native_symbols = native_symbols;
    return sizeof(native_symbols) / sizeof(NativeSymbol);
}

int init_native_lib()
{
    printf("%s in tzio_lib.c 202412292352 called\n", __func__);
    return 0;
}

void deinit_native_lib()
{
    printf("%s in tzio_lib.c called\n", __func__);
}
