/**
 * @brief native funcs for trust zone io
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include "tzio.h"
#include "pta_i2c.h"
#include "pta_spi.h"
#include "pta_dht.h"
#include "pta_bh1750fvi.h"
#include "pta_mpu6050.h"
#include "pta_w25qxx.h"
#include "pta_at24cxx.h"
#include "pta_gpio.h"
#include <trace.h>
#include <stdlib.h>

#define N_TIMES 10

#define MAX_GPIO_PIN_NUM 27
#define MAX_I2C_NUM 1
#define MAX_SPI_NUM 0

#define teetime_to_micro(t) \
    (uint64_t)t.seconds * 1000 * 1000 + (uint64_t)t.micros

static TEE_TASessionHandle gpio_sess = TEE_HANDLE_NULL;
static TEE_TASessionHandle i2c_sess = TEE_HANDLE_NULL;
static TEE_TASessionHandle spi_sess = TEE_HANDLE_NULL;
static TEE_TASessionHandle dht_sess = TEE_HANDLE_NULL;
static TEE_TASessionHandle bh1750fvi_sess = TEE_HANDLE_NULL;
static TEE_TASessionHandle mpu6050_sess = TEE_HANDLE_NULL;
static TEE_TASessionHandle w25qxx_sess = TEE_HANDLE_NULL;
static TEE_TASessionHandle at24cxx_sess = TEE_HANDLE_NULL;
static TEE_TASessionHandle tel0157_sess = TEE_HANDLE_NULL;
static TEE_TASessionHandle atk301_sess = TEE_HANDLE_NULL;

static TEE_Time times[N_TIMES];

static TEE_Result invoke_gpio_pta(uint32_t cmd_id, uint32_t param_types,
                                       TEE_Param params[TEE_NUM_PARAMS])
{
    const TEE_UUID gpio_uuid=PTA_GPIO_SERVICE_UUID; 
    TEE_Result res = TEE_ERROR_GENERIC;
    // load existed session
    if(gpio_sess == TEE_HANDLE_NULL){
        // open session
        res=TEE_OpenTASession(&gpio_uuid,TEE_TIMEOUT_INFINITE,0,NULL,&gpio_sess,NULL);
        // return error
        if(res!=TEE_SUCCESS) {
            EMSG("The session with the gpio service cannot be established. Error: %x", res);
            return res;
        }
    }
    return TEE_InvokeTACommand(gpio_sess,TEE_TIMEOUT_INFINITE,cmd_id,param_types,params,NULL);
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


int record_benchmark_time(uint32_t idx){
    if(idx>=N_TIMES){return TEE_ERROR_BAD_PARAMETERS;}
    TEE_GetSystemTime(&times[idx]);
    return TEE_SUCCESS;
}

int i2c_read_bytes(uint32_t i2c_dev, uint32_t addr, uint32_t flags, void * buf, uint32_t buf_len){
    TEE_Param params[TEE_NUM_PARAMS]={};
    uint32_t param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                            TEE_PARAM_TYPE_NONE);
    params[0].value.a=i2c_dev;
    params[0].value.b=addr;
    params[1].value.a=flags;
    params[2].memref.buffer=buf;
    params[2].memref.size=buf_len;
    return invoke_i2c_pta(PTA_I2C_READ_BYTES,param_types,params);
}

int i2c_write_bytes(uint32_t i2c_dev, uint32_t addr, uint32_t flags,   void * buf, uint32_t buf_len){
    TEE_Param params[TEE_NUM_PARAMS]={};
    uint32_t param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_MEMREF_INPUT,
                                            TEE_PARAM_TYPE_NONE);

    params[0].value.a=i2c_dev;
    params[0].value.b=addr;
    params[1].value.a=flags;
    params[2].memref.buffer=buf;
    params[2].memref.size=buf_len;
    return invoke_i2c_pta(PTA_I2C_WRITE_BYTES,param_types,params);
}

int i2c_set_freq(uint32_t i2c_dev, uint32_t i2c_freq_opt){
    TEE_Param params[TEE_NUM_PARAMS]={};
    uint32_t param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                           TEE_PARAM_TYPE_NONE,
                                           TEE_PARAM_TYPE_NONE,
                                           TEE_PARAM_TYPE_NONE);
    params[0].value.a=i2c_dev;
    params[0].value.b=i2c_freq_opt;
    return invoke_i2c_pta(PTA_I2C_SET_FREQ,param_types,params);
}

int i2c_read_regs(uint32_t i2c_dev, uint32_t addr, uint32_t flags,uint32_t reg, void * buf, uint32_t buf_len){
    TEE_Param params[TEE_NUM_PARAMS]={};
    uint32_t param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                            TEE_PARAM_TYPE_NONE);
    params[0].value.a=i2c_dev;
    params[0].value.b=addr;
    params[1].value.a=flags;
    params[1].value.b=reg;
    params[2].memref.buffer=buf;
    params[2].memref.size=buf_len;
    return invoke_i2c_pta(PTA_I2C_READ_REGS,param_types,params);
}

int i2c_write_regs(uint32_t i2c_dev, uint32_t addr, uint32_t flags,uint32_t reg,   void * buf, uint32_t buf_len){
    TEE_Param params[TEE_NUM_PARAMS]={};
    uint32_t param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_VALUE_INPUT,
                                            TEE_PARAM_TYPE_MEMREF_INPUT,
                                            TEE_PARAM_TYPE_NONE);
    params[0].value.a=i2c_dev;
    params[0].value.b=addr;
    params[1].value.a=flags;
    params[1].value.b=reg;
    params[2].memref.buffer=buf;
    params[2].memref.size=buf_len;
    return invoke_i2c_pta(PTA_I2C_WRITE_BYTES,param_types,params);
}

int dht_init(uint32_t pin, uint32_t dht_type)
{
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a = (uint32_t)pin;
    params[0].value.b = (uint32_t)dht_type;
    return invoke_dht_pta(PTA_CMD_DHT_INIT,param_types,params);
}

int dht_read(uint32_t pin, uint32_t dht_type, int16_t * temp, int16_t * hum)
{
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;

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

int get_benchmark_time(uint32_t idx, uint64_t * out_time){
    if(idx>=N_TIMES){return TEE_ERROR_BAD_PARAMETERS;}
    *out_time=teetime_to_micro(times[idx]);
    return TEE_SUCCESS;
}

int bh1750fvi_init(uint32_t i2c_no,uint32_t addr)
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

int bh1750fvi_sample(uint32_t i2c_no,uint32_t addr,uint16_t * lux){
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
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

int mpu6050_init(uint32_t i2c_no,uint32_t i2c_addr){
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

int mpu6050_deinit(uint32_t i2c_no,uint32_t i2c_addr){
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

int mpu6050_sample(uint32_t i2c_no,uint32_t i2c_addr, uint8_t mpu6050_data[14]){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
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

int w25qxx_init(uint32_t spi_no,uint32_t spi_cs_no){
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

int w25qxx_read_id(uint32_t spi_no,uint32_t spi_cs_no, uint16_t * id){
    TEE_Result res;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a=spi_no;
    params[0].value.b=spi_cs_no;
    res=invoke_w25qxx_pta(PTA_CMD_W25QXX_READ_ID,param_types,params);
    if(res!=TEE_SUCCESS){
        return res;
    }
    *id=(uint16_t)params[0].value.a;
    return TEE_SUCCESS;
}

int w25qxx_read_data(uint32_t spi_no,uint32_t spi_cs_no,
                                    uint32_t read_addr, void * buf, uint32_t len){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                  TEE_PARAM_TYPE_NONE);

    params[0].value.a=spi_no;
    params[0].value.b=spi_cs_no;
    params[1].value.a=read_addr;
    params[2].memref.buffer=buf;
    params[2].memref.size=len;
    return invoke_w25qxx_pta(PTA_CMD_W25QXX_READ_DATA,param_types,params);
}

int w25qxx_page_program(uint32_t spi_no, uint32_t spi_cs_no, uint32_t write_addr, void * input_data_buf, uint32_t len){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_MEMREF_INPUT,
                                  TEE_PARAM_TYPE_NONE);
    
    params[0].value.a=spi_no;
    params[0].value.b=spi_cs_no;
    params[1].value.a=write_addr;
    params[2].memref.buffer=input_data_buf;
    params[2].memref.size=len;
    return invoke_w25qxx_pta(PTA_CMD_W25QXX_PAGE_PROGRAM,param_types,params);
}

int w25qxx_sector_erase(uint32_t spi_no, uint32_t spi_cs_no, uint32_t sector_addr){
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

int w25qxx_chip_erase(uint32_t spi_no, uint32_t spi_cs_no){
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

int tel0157_init(uint32_t i2c_no,uint32_t i2c_addr, uint32_t gnss_mode, uint32_t rgb){
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

int tel0157_deinit(uint32_t i2c_no,uint32_t i2c_addr){
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

int tel0157_get_gnss_len(uint32_t i2c_no,uint32_t i2c_addr, uint16_t * len){
    TEE_Result res;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a=i2c_no;
    params[0].value.b=i2c_addr;
    res=invoke_tel0157_pta(PTA_CMD_TEL0157_GET_GNSS_LEN,param_types,params);
    if(res!=TEE_SUCCESS){
        return res;
    }
    *len=(uint16_t)params[0].value.a;
    return TEE_SUCCESS;
}

int tel0157_get_all_gnss(uint32_t i2c_no,uint32_t i2c_addr, void * buf, uint32_t len){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a=i2c_no;
    params[0].value.b=i2c_addr;
    params[1].memref.buffer=buf;
    params[1].memref.size=len;
    return invoke_tel0157_pta(PTA_CMD_TEL0157_GET_ALL_GNSS,param_types,params);
}

int tel0157_get_utc_time(uint32_t i2c_no,uint32_t i2c_addr,  tel0157_time_t * buf){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a=i2c_no;
    params[0].value.b=i2c_addr;
    params[1].memref.buffer=buf;
    params[1].memref.size=sizeof(tel0157_time_t);
    return invoke_tel0157_pta(PTA_CMD_TEL0157_GET_UTC_TIME,param_types,params);
}

int tel0157_get_gnss_mode(uint32_t i2c_no,uint32_t i2c_addr, uint8_t * mode){
    TEE_Result res;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a=i2c_no;
    params[0].value.b=i2c_addr;
    res=invoke_tel0157_pta(PTA_CMD_TEL0157_GET_GNSS_MODE,param_types,params);
    if(res!=TEE_SUCCESS){
        return res;
    }
    *mode=(uint8_t)params[0].value.a;
    return res;
}

int tel0157_get_num_sat_used(uint32_t i2c_no,uint32_t i2c_addr, uint8_t * num){
    TEE_Result res;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;

    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a=i2c_no;
    params[0].value.b=i2c_addr;
    res=invoke_tel0157_pta(PTA_CMD_TEL0157_GET_NUM_SAT_USED,param_types,params);
    if(res!=TEE_SUCCESS){
        return res;
    }
    *num=(uint8_t)params[0].value.a;
    return res;
}

int tel0157_get_lon(uint32_t i2c_no,uint32_t i2c_addr, tel0157_lon_lat_t * lon){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    
    params[0].value.a=i2c_no;
    params[0].value.b=i2c_addr;
    params[1].memref.buffer=(void *)lon;
    params[1].memref.size=sizeof(tel0157_lon_lat_t);
    return invoke_tel0157_pta(PTA_CMD_TEL0157_GET_LON,param_types,params);
}

int tel0157_get_lat(uint32_t i2c_no,uint32_t i2c_addr, tel0157_lon_lat_t * lat){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a=i2c_no;
    params[0].value.b=i2c_addr;
    params[1].memref.buffer=(void *)lat;
    params[1].memref.size=sizeof(tel0157_lon_lat_t);
    return invoke_tel0157_pta(PTA_CMD_TEL0157_GET_LAT,param_types,params);
}

int tel0157_get_alt(uint32_t i2c_no,uint32_t i2c_addr, uint8_t alt[3]){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a=i2c_no;
    params[0].value.b=i2c_addr;
    params[1].memref.buffer=alt;
    params[1].memref.size=3;
    return invoke_tel0157_pta(PTA_CMD_TEL0157_GET_ALT,param_types,params);
}

int tel0157_get_sog(uint32_t i2c_no,uint32_t i2c_addr, uint8_t sog[3]){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a=i2c_no;
    params[0].value.b=i2c_addr;
    params[1].memref.buffer=sog;
    params[1].memref.size=3;
    return invoke_tel0157_pta(PTA_CMD_TEL0157_GET_ALT,param_types,params);
}

int tel0157_get_cog(uint32_t i2c_no,uint32_t i2c_addr, uint8_t cog[3]){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a=i2c_no;
    params[0].value.b=i2c_addr;
    params[1].memref.buffer=cog;
    params[1].memref.size=3;
    return invoke_tel0157_pta(PTA_CMD_TEL0157_GET_ALT,param_types,params);
}

int spi_transfer_byte(uint32_t spi_no,uint32_t spi_cs_no, uint32_t cont,
                             uint32_t mode, uint32_t freq,  uint32_t need_byte_out, uint32_t need_byte_in,
                              uint32_t byte_out, uint8_t * byte_in){
    TEE_Result res;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;

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

int spi_transfer_bytes(uint32_t spi_no,uint32_t spi_cs_no, uint32_t cont,
                             uint32_t mode, uint32_t freq,  uint32_t src_len, uint32_t dst_len,
                                uint8_t * src_buf, uint8_t * dst_buf){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;

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

int spi_transfer_diff_bytes(uint32_t spi_no,uint32_t spi_cs_no, uint32_t cont,
                             uint32_t mode, uint32_t freq,  uint32_t src_len, uint32_t dst_len,
                                uint8_t * src_buf, uint8_t * dst_buf){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
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

int write_secure_object(char * id, uint32_t id_len, char * data, uint32_t data_len){
    TEE_Result res;
    TEE_ObjectHandle object;
    uint32_t obj_data_flag;
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

int read_secure_object(char * id, uint32_t id_len, char * data, uint32_t * data_len){
    TEE_Result res;
    TEE_ObjectHandle object;
    TEE_ObjectInfo info;
    char * buf;
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

int delete_secure_object(char * id, uint32_t id_len){
    TEE_Result res;
    TEE_ObjectHandle object;
    res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE_RPMB, id, id_len, 
            TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE_META, &object);
    if(res!=TEE_SUCCESS){
        return res;
    }
    TEE_CloseAndDeletePersistentObject(object);
    return res;
}

int at24cxx_read(uint32_t i2c_no,uint32_t i2c_addr, uint32_t read_addr, uint8_t * buf, uint32_t len){
    TEE_Result res;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
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

int at24cxx_write(uint32_t i2c_no,uint32_t i2c_addr, uint32_t write_addr,   uint8_t * buf, uint32_t len){
    TEE_Result res;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
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

int atk301_init(uint32_t i2c_no, uint32_t i2c_addr){
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

int atk301_get_fingerprint_image(uint32_t i2c_no, uint32_t i2c_addr){
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

int atk301_upload_fingerprint_image(uint32_t i2c_no, uint32_t i2c_addr, uint8_t buf[ATK301_FINGERPRINT_IMAGE_DATA_LEN]){
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
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

int native_wait(int ms){
    return TEE_Wait(ms);
}

int gpio_init(int pin, int mode)
{
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    if (pin < 0)
    {
        return TEE_ERROR_BAD_PARAMETERS;
    }
    if (mode < 0)
    {
        return TEE_ERROR_BAD_PARAMETERS;
    }

    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a = (uint32_t)pin;
    params[0].value.b = (uint32_t)mode;
    res = invoke_gpio_pta(PTA_GPIO_CMD_INIT,param_types,params);
    return res;
}

int gpio_set(int pin, int level)
{
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    if (pin < 0)
    {
        return TEE_ERROR_BAD_PARAMETERS;
    }
    // val should >= 0, 0 is low level, >0 is high level 
    if (level < 0)
    {
        return TEE_ERROR_BAD_PARAMETERS;
    }

    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a = (uint32_t)pin;
    params[0].value.b = (uint32_t)level;
    res = invoke_gpio_pta(PTA_GPIO_CMD_SET,param_types,params);
    return res;
}

int gpio_get(int pin)
{
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_Param params[TEE_NUM_PARAMS] = {};
    uint32_t param_types = 0;
    if (pin < 0)
    {
        return TEE_ERROR_BAD_PARAMETERS;
    }
    param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
                                  TEE_PARAM_TYPE_VALUE_OUTPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    params[0].value.a = (uint32_t)pin;
    res = invoke_gpio_pta(PTA_GPIO_CMD_GET,param_types,params);
    if(res) return -1;
    if(params[1].value.a) return 1;
    else return 0;
}