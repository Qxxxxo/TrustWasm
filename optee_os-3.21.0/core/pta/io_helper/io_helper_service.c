/**
 * @brief pta io helper
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */

#include "pta_io_helper.h"
#include "kernel/pseudo_ta.h"
#include "kernel/mutex.h"
#include "io_helper/io_req.h"
#include "io_helper/io_signal.h"
#include "drivers/dht/dht.h"
#include "drivers/bh1750/bh1750fvi.h"
#include "drivers/mpu6050/mpu6050.h"
#include "drivers/gnss/tel0157.h"
#include "drivers/at24cxx/at24cxx.h"
#include "drivers/atk301/atk301.h"
#include "periph/gpio.h"
#include "periph/i2c.h"
#include "periph/spi.h"
#include "string.h"
#include <platform_config.h>
#include <tee/tee_svc_storage.h>
#include <tee/tee_svc_cryp.h>

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#define IO_HELPER_PTA_NAME  "io_helper.pta"

#ifdef OVERHEAD_BENCHMARK
#include<kernel/tee_time.h>
#include<string.h>
#define IO_HELPER_SERVICE_OVERHEAD_BENCHMARK_SIZE    200
#define IO_HELPER_SERVICE_OVERHEAD_BENCHMARK_POINTS    4
#define IO_HELPER_SERVICE_OVERHEAD_BENCHMARK_TIMES    50
static uint32_t req_idx=0;
static uint32_t get_data_idx=0;
static uint64_t benchmark_time[IO_HELPER_SERVICE_OVERHEAD_BENCHMARK_SIZE]={0};
#define teetime_to_micro(t) \
    (uint64_t)t.seconds * 1000 * 1000 + (uint64_t)t.micros
#endif

// modify use queue mutex
static io_req_queue req_queues[N_PERIPH]={0};
static struct mutex queue_mutexes[N_PERIPH];

// notify not empty to loop
static struct condvar queue_notempty[N_PERIPH];

static inline uint16_t periph_no_to_gpio(uint16_t periph_no){
    // periph no is gpio no start from 0
    return periph_no;
}

static inline uint16_t periph_no_to_i2c(uint16_t periph_no){
    // periph no for i2c after gpio
    return periph_no-AVAILABLE_GPIO_NUM;
}

static inline uint16_t periph_no_to_spi(uint16_t periph_no){
    // periph no for spi after i2c and gpio
    return periph_no-AVAILABLE_GPIO_NUM-AVAILABLE_I2C_NUM;
}

static TEE_Result handle_dht_read(io_req_t * req,uint16_t periph_no){
    TEE_Result res;
    // prepare params
    dht_params_t dev_params={
        .pin=(gpio_t)periph_no_to_gpio(periph_no),
        .type=(dht_type_t)req->params[0].value.a, // dht type,
        .in_mode=GPIO_IN_PU,
    };
    dht_t dev={
        .params=dev_params,
    };
    // malloc mem to save result
    int16_t* results=(int16_t*)malloc(2*sizeof(int16_t));
    if(__builtin_expect(results==NULL,0)){
        // malloc failed
        io_helper_signal_table[periph_no][req->signo].ret=TEE_ERROR_BAD_STATE;
        return TEE_ERROR_BAD_STATE;
    }
    res=dht_read(&dev,&results[0],&results[1]);
    if(res!=TEE_SUCCESS){
        // read failed
        free((void *)results);
        io_helper_signal_table[periph_no][req->signo].ret=res;
        return res;
    }
    // success
    io_helper_signal_table[periph_no][req->signo].ret=2*sizeof(int16_t);
    io_helper_signal_table[periph_no][req->signo].len=2*sizeof(int16_t);
    io_helper_signal_table[periph_no][req->signo].buf=(void*)results;
    return TEE_SUCCESS;
}

static TEE_Result handle_bh1750fvi_sample(io_req_t * req, uint16_t periph_no){
    TEE_Result res;
    bh1750fvi_t dev={
        .i2c=(i2c_t)periph_no_to_i2c(periph_no),
        .addr=(uint8_t)req->params[0].value.a,
    };
    uint16_t * lux=(uint16_t *)malloc(sizeof(uint16_t));
    if(__builtin_expect(lux==NULL,0)){
        io_helper_signal_table[periph_no][req->signo].ret=TEE_ERROR_BAD_STATE;
        return TEE_ERROR_BAD_STATE;
    }
    res=bh1750fvi_sample(&dev,lux);
    if(res!=TEE_SUCCESS){
        free((void *)lux);
        io_helper_signal_table[periph_no][req->signo].ret=res;
        return res;
    }
    io_helper_signal_table[periph_no][req->signo].ret=sizeof(uint16_t);
    io_helper_signal_table[periph_no][req->signo].len=sizeof(uint16_t);
    io_helper_signal_table[periph_no][req->signo].buf=(void *)lux;
    return TEE_SUCCESS;
}

static TEE_Result handle_mpu6050_sample(io_req_t * req, uint16_t periph_no){
    TEE_Result res;
    mpu6050_t dev={
        .i2c=(i2c_t)periph_no_to_i2c(periph_no),
        .addr=(uint8_t)req->params[0].value.a,
    };
    // need 14 bytes
    uint8_t * data = (uint8_t*)malloc(14*sizeof(uint8_t));
    res=mpu_sample(&dev,data);
    if(res!=TEE_SUCCESS){
        free((void *)data);
        io_helper_signal_table[periph_no][req->signo].ret=res;
        return res;
    }
    io_helper_signal_table[periph_no][req->signo].ret=14;
    io_helper_signal_table[periph_no][req->signo].len=14;
    io_helper_signal_table[periph_no][req->signo].buf=(void *)data;
    return TEE_SUCCESS;
}

static TEE_Result handle_write_secure_object(io_req_t * req, uint16_t periph_no){
    TEE_Result res;
    uint32_t obj;
    TEE_ObjectHandle object;
    uint32_t obj_data_flag=TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE | TEE_DATA_FLAG_ACCESS_WRITE_META | TEE_DATA_FLAG_OVERWRITE;
    res = syscall_storage_obj_create(
                TEE_STORAGE_PRIVATE_RPMB,
                req->params[0].memref.buffer,
                req->params[0].memref.size,
                obj_data_flag,
                (unsigned long)TEE_HANDLE_NULL,NULL,0,
                &obj);
    if(res!=TEE_SUCCESS){
        goto handle_write_secure_object_exit;
    }
    object=(TEE_ObjectHandle)(uintptr_t)obj;
    
    res= syscall_storage_obj_write((unsigned long)object,req->params[1].memref.buffer, req->params[1].memref.size);
    if(res!=TEE_SUCCESS){
        syscall_storage_obj_del((unsigned long)object);
    }else{
        syscall_cryp_obj_close((unsigned long)object);
    }
handle_write_secure_object_exit:
    io_helper_signal_table[periph_no][req->signo].ret=res;
    io_helper_signal_table[periph_no][req->signo].len=0;
    io_helper_signal_table[periph_no][req->signo].buf=NULL;
    // free id buf and write data buf
    free(req->params[0].memref.buffer);
    free(req->params[1].memref.buffer);
    return res;
}

static TEE_Result handle_read_secure_object(io_req_t * req, uint16_t periph_no){
    TEE_Result res;
    uint32_t obj;
    TEE_ObjectHandle object;
    struct utee_object_info info;
    uint64_t cnt64;
    uint32_t read_bytes=0;
    uint8_t * buf;

    res=syscall_storage_obj_open(TEE_STORAGE_PRIVATE_RPMB, req->params[0].memref.buffer, req->params[0].memref.size, 
            TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_SHARE_READ, &obj);
    if(res!=TEE_SUCCESS){
        io_helper_signal_table[periph_no][req->signo].ret=res;
        goto handle_read_secure_object_exit;
    }
    object=(TEE_ObjectHandle)(uintptr_t)obj;

    res=syscall_cryp_obj_get_info((unsigned long)object,&info);
    if(res!=TEE_SUCCESS){
        io_helper_signal_table[periph_no][req->signo].ret=res;
        goto handle_read_secure_object_exit;
    }
    // malloc info data size
    buf=(uint8_t *)malloc(info.data_size);
    if(__builtin_expect(buf==NULL,0)){
        res=TEE_ERROR_OUT_OF_MEMORY;
        io_helper_signal_table[periph_no][req->signo].ret=res;
        syscall_cryp_obj_close((unsigned long)object);
        goto handle_read_secure_object_exit;
    }
    
    cnt64 = read_bytes;
    res = syscall_storage_obj_read((unsigned long)object,buf,info.data_size,&cnt64);
    read_bytes=(uint32_t)cnt64;

    if(res!=TEE_SUCCESS){
        io_helper_signal_table[periph_no][req->signo].ret=res;
        syscall_cryp_obj_close((unsigned long)object);
        free(buf);
        goto handle_read_secure_object_exit;
    }
    if(info.data_size!=read_bytes){
        res=TEE_ERROR_BAD_STATE;
        io_helper_signal_table[periph_no][req->signo].ret=res;
        syscall_cryp_obj_close((unsigned long)object);
        free(buf);
        goto handle_read_secure_object_exit;
    }
    // success
    io_helper_signal_table[periph_no][req->signo].ret=info.data_size;
    io_helper_signal_table[periph_no][req->signo].len=info.data_size;
    io_helper_signal_table[periph_no][req->signo].buf=(void *)buf;
handle_read_secure_object_exit:
    // free id buf
    free(req->params[0].memref.buffer);
    return res;
}

static TEE_Result handle_i2c_write_bytes(io_req_t * req, uint16_t periph_no){
    TEE_Result res;
    i2c_t i2c=(i2c_t)periph_no_to_i2c(periph_no);
    res=i2c_acquire(i2c);
    if(res!=TEE_SUCCESS){
        io_helper_signal_table[periph_no][req->signo].ret=res;
        return res;
    }
    res=i2c_write_bytes(i2c,
                        (uint16_t)req->params[1].value.a, // i2c addr
                        req->params[0].memref.buffer,
                        req->params[0].memref.size,
                        (uint8_t)req->params[1].value.b); // i2c flag
    i2c_release(i2c);
    io_helper_signal_table[periph_no][req->signo].ret=res;
    io_helper_signal_table[periph_no][req->signo].len=0;
    io_helper_signal_table[periph_no][req->signo].buf=NULL;
    // free write buf
    free(req->params[0].memref.buffer);
    return res;
}

static TEE_Result handle_i2c_read_bytes(io_req_t * req, uint16_t periph_no){
    TEE_Result res;
    i2c_t i2c=(i2c_t)periph_no_to_i2c(periph_no);
    uint8_t * buf=(uint8_t *)malloc(req->params[0].value.b);// read len
    if(__builtin_expect(buf==NULL,0)){
        io_helper_signal_table[periph_no][req->signo].ret=TEE_ERROR_OUT_OF_MEMORY;
        return TEE_ERROR_OUT_OF_MEMORY;
    }
    res=i2c_acquire(i2c);
    if(res!=TEE_SUCCESS){
        free(buf);
        io_helper_signal_table[periph_no][req->signo].ret=res;
        return res;
    }
    res=i2c_read_bytes( i2c,
                        (uint16_t)req->params[0].value.a, // i2c addr
                        buf,
                        req->params[0].value.b, // read len
                        (uint8_t)req->params[1].value.a); // i2c flag
    i2c_release(i2c);
    if(res!=TEE_SUCCESS){
        free(buf);
        io_helper_signal_table[periph_no][req->signo].ret=res;
        return res;
    }
    // success
    io_helper_signal_table[periph_no][req->signo].ret=req->params[0].value.b;
    io_helper_signal_table[periph_no][req->signo].len=req->params[0].value.b;
    io_helper_signal_table[periph_no][req->signo].buf=(void *)buf;
    return TEE_SUCCESS;
}

static TEE_Result handle_i2c_read_regs(io_req_t * req, uint16_t periph_no){
    TEE_Result res;
    i2c_t i2c=(i2c_t)periph_no_to_i2c(periph_no);
    uint8_t * buf=(uint8_t *)malloc(req->params[1].value.a);// read len
    if(__builtin_expect(buf==NULL,0)){
        io_helper_signal_table[periph_no][req->signo].ret=TEE_ERROR_OUT_OF_MEMORY;
        return TEE_ERROR_OUT_OF_MEMORY;
    }
    res=i2c_acquire(i2c);
    if(res!=TEE_SUCCESS){
        free(buf);
        io_helper_signal_table[periph_no][req->signo].ret=res;
        return res;
    }
    res=i2c_read_regs( i2c,
                        (uint16_t)req->params[0].value.a, // i2c addr
                        (uint16_t)req->params[0].value.b, // reg
                        buf,
                        req->params[1].value.a, // read len
                        (uint8_t)req->params[1].value.b); // i2c flag
    i2c_release(i2c);
    if(res!=TEE_SUCCESS){
        free(buf);
        io_helper_signal_table[periph_no][req->signo].ret=res;
        return res;
    }
    // success
    io_helper_signal_table[periph_no][req->signo].ret=req->params[1].value.a;
    io_helper_signal_table[periph_no][req->signo].len=req->params[1].value.a;
    io_helper_signal_table[periph_no][req->signo].buf=(void *)buf;
    return TEE_SUCCESS;
}

static TEE_Result handle_spi_transfer_bytes(io_req_t * req, uint16_t periph_no){
    TEE_Result res;
    spi_cs_t spi_cs=(spi_cs_t)req->params[1].value.a; // spi cs
    spi_t spi=(spi_t)periph_no_to_spi(periph_no); // spi
    bool cont = (bool)req->params[1].value.b; // spi cont
    spi_mode_t mode=(spi_mode_t)((req->params[2].value.a>>16)&0xFFFF); // H 16
    spi_clk_t freq=(spi_clk_t)(req->params[2].value.a&0xFFFF); // L 16
    uint8_t * buf=NULL;
    if(req->params[2].value.b>0){
        //  malloc read buf
        buf=(uint8_t *)malloc(req->params[2].value.b);// read len
        if(__builtin_expect(buf==NULL,0)){
            io_helper_signal_table[periph_no][req->signo].ret=TEE_ERROR_OUT_OF_MEMORY;
            return TEE_ERROR_OUT_OF_MEMORY;
        }
    }
    res=spi_acquire(spi,spi_cs,mode,freq);
    if(res!=TEE_SUCCESS){
        io_helper_signal_table[periph_no][req->signo].ret=res;
        free(buf);
        return res;
    }
    if(req->params[2].value.b==0){
        // read len = 0, only write
        res=spi_transfer_bytes(spi,spi_cs,cont,req->params[0].memref.buffer,NULL,req->params[0].memref.size);
    }else if(req->params[0].memref.size==0){
        // write len = 0, only read
        res=spi_transfer_bytes(spi,spi_cs,cont,NULL,buf,req->params[2].value.b);
    }else{
        // exchange same len
        res=spi_transfer_bytes(spi,spi_cs,cont,req->params[0].memref.buffer,buf,req->params[0].memref.size);
    }
    spi_release(spi);
    if(res!=TEE_SUCCESS){
        // free write buf
        free(req->params[0].memref.buffer);
        free(buf);
        io_helper_signal_table[periph_no][req->signo].ret=res;
        return res;
    }
    // success
    io_helper_signal_table[periph_no][req->signo].ret=req->params[2].value.b;
    io_helper_signal_table[periph_no][req->signo].len=req->params[2].value.b;
    io_helper_signal_table[periph_no][req->signo].buf=(void *)buf;
    // free write buf
    free(req->params[0].memref.buffer);
    return TEE_SUCCESS;
}

static TEE_Result handle_at24cxx_read(io_req_t * req, uint16_t periph_no){
    TEE_Result res;
    at24cxx_t dev={
        .dev=(i2c_t)periph_no_to_i2c(periph_no),
        .addr=(uint8_t)req->params[0].value.a, // i2c addr
    };
    uint8_t * buf=(uint8_t *)malloc(req->params[1].value.a);// read len
    if(__builtin_expect(buf==NULL,0)){
        io_helper_signal_table[periph_no][req->signo].ret=TEE_ERROR_OUT_OF_MEMORY;
        return TEE_ERROR_OUT_OF_MEMORY;
    }
    res=at24cxx_read(&dev,
                    (uint16_t)req->params[0].value.b, // read addr
                    buf,
                    req->params[1].value.a); // read len
    if(res!=TEE_SUCCESS){
        free(buf);
        io_helper_signal_table[periph_no][req->signo].ret=res;
        return res;
    }
    // success
    io_helper_signal_table[periph_no][req->signo].ret=req->params[1].value.a;
    io_helper_signal_table[periph_no][req->signo].len=req->params[1].value.a;
    io_helper_signal_table[periph_no][req->signo].buf=(void *)buf;
    return TEE_SUCCESS;
}

static TEE_Result handle_at24cxx_write(io_req_t * req, uint16_t periph_no){
    TEE_Result res;
    at24cxx_t dev={
        .dev=(i2c_t)periph_no_to_i2c(periph_no),
        .addr=(uint8_t)req->params[1].value.a, // i2c addr
    };
    res=at24cxx_write(&dev,
                    (uint16_t)req->params[1].value.b, // write addr
                    req->params[0].memref.buffer,
                    req->params[0].memref.size); // write len
    io_helper_signal_table[periph_no][req->signo].ret=res;
    io_helper_signal_table[periph_no][req->signo].len=0;
    io_helper_signal_table[periph_no][req->signo].buf=NULL;
    // free write buf
    free(req->params[0].memref.buffer);
    return res;
}

static TEE_Result handle_tel0157_get_gnss_len(io_req_t * req, uint16_t periph_no){
    TEE_Result res;
    tel0157_t dev={
        .i2c=(i2c_t)periph_no_to_i2c(periph_no),
        .addr=(uint8_t)req->params[0].value.a, // i2c addr
    };
    uint16_t * gnss_len = (uint16_t *)malloc(sizeof(uint16_t));
    if(__builtin_expect(gnss_len==NULL,0)){
        io_helper_signal_table[periph_no][req->signo].ret=TEE_ERROR_OUT_OF_MEMORY;
        return TEE_ERROR_OUT_OF_MEMORY;
    }
    res=tel0157_get_gnss_len(&dev,gnss_len);
    if(res!=TEE_SUCCESS){
        free((void *)gnss_len);
        io_helper_signal_table[periph_no][req->signo].ret=res;
        return res;
    }
    io_helper_signal_table[periph_no][req->signo].ret=sizeof(uint16_t);
    io_helper_signal_table[periph_no][req->signo].len=sizeof(uint16_t);
    io_helper_signal_table[periph_no][req->signo].buf=(void *)gnss_len;
    return TEE_SUCCESS;
}

static TEE_Result handle_tel0157_get_all_gnss(io_req_t * req, uint16_t periph_no){
    TEE_Result res;
    tel0157_t dev={
        .i2c=(i2c_t)periph_no_to_i2c(periph_no),
        .addr=(uint8_t)req->params[0].value.a, // i2c addr
    };
    uint8_t * gnss_buf = (uint8_t *)malloc(req->params[0].value.b); // gnss len
    if(__builtin_expect(gnss_buf==NULL,0)){
        io_helper_signal_table[periph_no][req->signo].ret=TEE_ERROR_OUT_OF_MEMORY;
        return TEE_ERROR_OUT_OF_MEMORY;
    }
    res=tel0157_get_all_gnss(&dev,gnss_buf,(uint16_t)req->params[0].value.b);
    if(res!=TEE_SUCCESS){
        free((void*)gnss_buf);
        io_helper_signal_table[periph_no][req->signo].ret=res;
        return res;
    }
    io_helper_signal_table[periph_no][req->signo].ret=req->params[0].value.b;
    io_helper_signal_table[periph_no][req->signo].len=req->params[0].value.b;
    io_helper_signal_table[periph_no][req->signo].buf=(void*)gnss_buf;
    return TEE_SUCCESS;
}

static TEE_Result handle_tel0157_get_utc_time(io_req_t * req, uint16_t periph_no){
    TEE_Result res;
    tel0157_t dev={
        .i2c=(i2c_t)periph_no_to_i2c(periph_no),
        .addr=(uint8_t)req->params[0].value.a, // i2c addr
    };
    tel0157_time_t * buf=(tel0157_time_t *)malloc(sizeof(tel0157_time_t));
    if(__builtin_expect(buf==NULL,0)){
        io_helper_signal_table[periph_no][req->signo].ret=TEE_ERROR_OUT_OF_MEMORY;
        return TEE_ERROR_OUT_OF_MEMORY;
    }
    res=tel0157_get_utc_time(&dev,buf);
    if(res!=TEE_SUCCESS){
        free((void*)buf);
        io_helper_signal_table[periph_no][req->signo].ret=res;
        return res;
    }
    io_helper_signal_table[periph_no][req->signo].ret=sizeof(tel0157_time_t);
    io_helper_signal_table[periph_no][req->signo].len=sizeof(tel0157_time_t);
    io_helper_signal_table[periph_no][req->signo].buf=(void*)buf;
    return TEE_SUCCESS;
}

static TEE_Result handle_tel0157_get_gnss_mode(io_req_t * req, uint16_t periph_no){
    TEE_Result res;
    tel0157_t dev={
        .i2c=(i2c_t)periph_no_to_i2c(periph_no),
        .addr=(uint8_t)req->params[0].value.a, // i2c addr
    };
    uint8_t * gnss_mode = (uint8_t *)malloc(sizeof(uint8_t));
    if(__builtin_expect(gnss_mode==NULL,0)){
        io_helper_signal_table[periph_no][req->signo].ret=TEE_ERROR_OUT_OF_MEMORY;
        return TEE_ERROR_OUT_OF_MEMORY;
    }
    res=tel0157_get_gnss_mode(&dev,gnss_mode);
    if(res!=TEE_SUCCESS){
        free((void *)gnss_mode);
        io_helper_signal_table[periph_no][req->signo].ret=res;
        return res;
    }
    io_helper_signal_table[periph_no][req->signo].ret=sizeof(uint8_t);
    io_helper_signal_table[periph_no][req->signo].len=sizeof(uint8_t);
    io_helper_signal_table[periph_no][req->signo].buf=(void *)gnss_mode;
    return TEE_SUCCESS;
}

static TEE_Result handle_tel0157_get_num_sat_used(io_req_t * req, uint16_t periph_no){
    TEE_Result res;
    tel0157_t dev={
        .i2c=(i2c_t)periph_no_to_i2c(periph_no),
        .addr=(uint8_t)req->params[0].value.a, // i2c addr
    };
    uint8_t * num_sat_used = (uint8_t *)malloc(sizeof(uint8_t));
    if(__builtin_expect(num_sat_used==NULL,0)){
        io_helper_signal_table[periph_no][req->signo].ret=TEE_ERROR_OUT_OF_MEMORY;
        return TEE_ERROR_OUT_OF_MEMORY;
    }
    res=tel0157_get_num_sat_used(&dev,num_sat_used);
    if(res!=TEE_SUCCESS){
        free((void *)num_sat_used);
        io_helper_signal_table[periph_no][req->signo].ret=res;
        return res;
    }
    io_helper_signal_table[periph_no][req->signo].ret=sizeof(uint8_t);
    io_helper_signal_table[periph_no][req->signo].len=sizeof(uint8_t);
    io_helper_signal_table[periph_no][req->signo].buf=(void *)num_sat_used;
    return TEE_SUCCESS;
}

static TEE_Result handle_tel0157_get_lon(io_req_t * req, uint16_t periph_no){
    TEE_Result res;
    tel0157_t dev={
        .i2c=(i2c_t)periph_no_to_i2c(periph_no),
        .addr=(uint8_t)req->params[0].value.a, // i2c addr
    };
    tel0157_lon_lat_t * lon_lat = (tel0157_lon_lat_t *)malloc(sizeof(tel0157_lon_lat_t));
    if(__builtin_expect(lon_lat==NULL,0)){
        io_helper_signal_table[periph_no][req->signo].ret=TEE_ERROR_OUT_OF_MEMORY;
        return TEE_ERROR_OUT_OF_MEMORY;
    }
    res=tel0157_get_lon(&dev,lon_lat);
    if(res!=TEE_SUCCESS){
        free((void *)lon_lat);
        io_helper_signal_table[periph_no][req->signo].ret=res;
        return res;
    }
    io_helper_signal_table[periph_no][req->signo].ret=sizeof(tel0157_lon_lat_t);
    io_helper_signal_table[periph_no][req->signo].len=sizeof(tel0157_lon_lat_t);
    io_helper_signal_table[periph_no][req->signo].buf=(void *)lon_lat;
    return TEE_SUCCESS;
}

static TEE_Result handle_tel0157_get_lat(io_req_t * req, uint16_t periph_no){
    TEE_Result res;
    tel0157_t dev={
        .i2c=(i2c_t)periph_no_to_i2c(periph_no),
        .addr=(uint8_t)req->params[0].value.a, // i2c addr
    };
    tel0157_lon_lat_t * lon_lat = (tel0157_lon_lat_t *)malloc(sizeof(tel0157_lon_lat_t));
    if(__builtin_expect(lon_lat==NULL,0)){
        io_helper_signal_table[periph_no][req->signo].ret=TEE_ERROR_OUT_OF_MEMORY;
        return TEE_ERROR_OUT_OF_MEMORY;
    }
    res=tel0157_get_lat(&dev,lon_lat);
    if(res!=TEE_SUCCESS){
        free((void *)lon_lat);
        io_helper_signal_table[periph_no][req->signo].ret=res;
        return res;
    }
    io_helper_signal_table[periph_no][req->signo].ret=sizeof(tel0157_lon_lat_t);
    io_helper_signal_table[periph_no][req->signo].len=sizeof(tel0157_lon_lat_t);
    io_helper_signal_table[periph_no][req->signo].buf=(void *)lon_lat;
    return TEE_SUCCESS;
}

static TEE_Result handle_tel0157_get_alt(io_req_t * req, uint16_t periph_no){
    TEE_Result res;
    tel0157_t dev={
        .i2c=(i2c_t)periph_no_to_i2c(periph_no),
        .addr=(uint8_t)req->params[0].value.a, // i2c addr
    };
    uint8_t * buf = (uint8_t *)malloc(3*sizeof(uint8_t));
    if(__builtin_expect(buf==NULL,0)){
        io_helper_signal_table[periph_no][req->signo].ret=TEE_ERROR_OUT_OF_MEMORY;
        return TEE_ERROR_OUT_OF_MEMORY;
    }
    res=tel0157_get_alt(&dev,buf);
    if(res!=TEE_SUCCESS){
        free((void *)buf);
        io_helper_signal_table[periph_no][req->signo].ret=res;
        return res;
    }
    io_helper_signal_table[periph_no][req->signo].ret=3*sizeof(uint8_t);
    io_helper_signal_table[periph_no][req->signo].len=3*sizeof(uint8_t);
    io_helper_signal_table[periph_no][req->signo].buf=(void *)buf;
    return TEE_SUCCESS;
}

static TEE_Result handle_tel0157_get_sog(io_req_t * req, uint16_t periph_no){
    TEE_Result res;
    tel0157_t dev={
        .i2c=(i2c_t)periph_no_to_i2c(periph_no),
        .addr=(uint8_t)req->params[0].value.a, // i2c addr
    };
    uint8_t * buf = (uint8_t *)malloc(3*sizeof(uint8_t));
    if(__builtin_expect(buf==NULL,0)){
        io_helper_signal_table[periph_no][req->signo].ret=TEE_ERROR_OUT_OF_MEMORY;
        return TEE_ERROR_OUT_OF_MEMORY;
    }
    res=tel0157_get_sog(&dev,buf);
    if(res!=TEE_SUCCESS){
        free((void *)buf);
        io_helper_signal_table[periph_no][req->signo].ret=res;
        return res;
    }
    io_helper_signal_table[periph_no][req->signo].ret=3*sizeof(uint8_t);
    io_helper_signal_table[periph_no][req->signo].len=3*sizeof(uint8_t);
    io_helper_signal_table[periph_no][req->signo].buf=(void *)buf;
    return TEE_SUCCESS;
}

static TEE_Result handle_tel0157_get_cog(io_req_t * req, uint16_t periph_no){
    TEE_Result res;
    tel0157_t dev={
        .i2c=(i2c_t)periph_no_to_i2c(periph_no),
        .addr=(uint8_t)req->params[0].value.a, // i2c addr
    };
    uint8_t * buf = (uint8_t *)malloc(3*sizeof(uint8_t));
    if(__builtin_expect(buf==NULL,0)){
        io_helper_signal_table[periph_no][req->signo].ret=TEE_ERROR_OUT_OF_MEMORY;
        return TEE_ERROR_OUT_OF_MEMORY;
    }
    res=tel0157_get_cog(&dev,buf);
    if(res!=TEE_SUCCESS){
        free((void *)buf);
        io_helper_signal_table[periph_no][req->signo].ret=res;
        return res;
    }
    io_helper_signal_table[periph_no][req->signo].ret=3*sizeof(uint8_t);
    io_helper_signal_table[periph_no][req->signo].len=3*sizeof(uint8_t);
    io_helper_signal_table[periph_no][req->signo].buf=(void *)buf;
    return TEE_SUCCESS;
}

static TEE_Result handle_atk301_get_fingerprint_image(io_req_t * req, uint16_t periph_no){
    TEE_Result res;
    atk301_t dev={
        .i2c=(i2c_t)periph_no_to_i2c(periph_no),
        .addr=(uint8_t)req->params[0].value.a, // i2c addr
    };
    res=atk301_get_fingerprint_image(&dev);
    io_helper_signal_table[periph_no][req->signo].ret=res;
    io_helper_signal_table[periph_no][req->signo].len=0;
    io_helper_signal_table[periph_no][req->signo].buf=NULL;
    return TEE_SUCCESS;
}

static TEE_Result handle_atk301_upload_fingerprint_image(io_req_t * req, uint16_t periph_no){
    TEE_Result res;
    atk301_t dev={
        .i2c=(i2c_t)periph_no_to_i2c(periph_no),
        .addr=(uint8_t)req->params[0].value.a, // i2c addr
    };
    uint8_t * buf = (uint8_t *)malloc(ATK301_FINGERPRINT_IMAGE_DATA_LEN);
    if(__builtin_expect(buf==NULL,0)){
        io_helper_signal_table[periph_no][req->signo].ret=TEE_ERROR_OUT_OF_MEMORY;
        return TEE_ERROR_OUT_OF_MEMORY;
    }
    res=atk301_upload_fingerprint_image(&dev,buf);
    if(res!=TEE_SUCCESS){
        free((void*)buf);
        io_helper_signal_table[periph_no][req->signo].ret=res;
        return res;
    }
    io_helper_signal_table[periph_no][req->signo].ret=ATK301_FINGERPRINT_IMAGE_DATA_LEN;
    io_helper_signal_table[periph_no][req->signo].len=ATK301_FINGERPRINT_IMAGE_DATA_LEN;
    io_helper_signal_table[periph_no][req->signo].buf=buf;
    return TEE_SUCCESS;
}

static TEE_Result handle_req(io_req_t * req,uint16_t periph_no){
	switch(req->type){
		case DHT_READ:
			return handle_dht_read(req,periph_no);
        case BH1750FVI_SAMPLE:
            return handle_bh1750fvi_sample(req,periph_no);
        case MPU6050_SAMPLE:
            return handle_mpu6050_sample(req,periph_no);
        case WRITE_SECURE_OBJECT:
            return handle_write_secure_object(req,periph_no);
        case READ_SECURE_OBJECT:
            return handle_read_secure_object(req,periph_no);
        case I2C_WRITE_BYTES:
            return handle_i2c_write_bytes(req,periph_no);
        case I2C_READ_BYTES:
            return handle_i2c_read_bytes(req,periph_no);
        case I2C_READ_REGS:
            return handle_i2c_read_regs(req,periph_no);
        case SPI_TRANSFER_BYTES:
            return handle_spi_transfer_bytes(req,periph_no);
        case AT24CXX_READ:
            return handle_at24cxx_read(req,periph_no);
        case AT24CXX_WRITE:
            return handle_at24cxx_write(req,periph_no);
        case TEL0157_GET_GNSS_LEN:
            return handle_tel0157_get_gnss_len(req,periph_no);
        case TEL0157_GET_ALL_GNSS:
            return handle_tel0157_get_all_gnss(req,periph_no);
        case TEL0157_GET_UTC_TIME:
            return handle_tel0157_get_utc_time(req,periph_no);
        case TEL0157_GET_GNSS_MODE:
            return handle_tel0157_get_gnss_mode(req,periph_no);
        case TEL0157_GET_NUM_SAT_USED:
            return handle_tel0157_get_num_sat_used(req,periph_no);
        case TEL0157_GET_LON:
            return handle_tel0157_get_lon(req,periph_no);
        case TEL0157_GET_LAT:
            return handle_tel0157_get_lat(req,periph_no);
        case TEL0157_GET_ALT:
            return handle_tel0157_get_alt(req,periph_no);
        case TEL0157_GET_SOG:
            return handle_tel0157_get_sog(req,periph_no);
        case TEL0157_GET_COG:
            return handle_tel0157_get_cog(req,periph_no);
        case ATK301_GET_FINGERPRINT_IMAGE:
            return handle_atk301_get_fingerprint_image(req,periph_no);
        case ATK301_UPLOAD_FINGERPRINT_IMAGE:
            return handle_atk301_upload_fingerprint_image(req,periph_no);
		default:
            // not supported
            io_helper_signal_table[periph_no][req->signo].ret=TEE_ERROR_NOT_SUPPORTED;
			return TEE_ERROR_NOT_SUPPORTED;
	}
}

static TEE_Result create(void)
{
    DMSG("has been called");
    return TEE_SUCCESS;
}

static TEE_Result start_loop(uint32_t param_types,
                             TEE_Param params[4])
{
    io_req_t req;
    uint16_t periph_no=0;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE);
    if(exp_param_types!=param_types){
        EMSG("Wrong param types");
        return TEE_ERROR_BAD_PARAMETERS;
    }
    // get periph number
    periph_no=params[0].value.a;
    if(periph_no>=N_PERIPH){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    // init mutexes & condvar
    mutex_init(&sigpending_mutexes[periph_no]);
    mutex_init(&sigusing_mutexes[periph_no]);
    mutex_init(&queue_mutexes[periph_no]);
    condvar_init(&queue_notempty[periph_no]);
    
    while(true){
        // lock mutexs
        mutex_lock(&queue_mutexes[periph_no]);
        // condvar
        while(is_empty(&req_queues[periph_no])){
            // wait not empty
            DMSG("condvar wait %d",periph_no);
            condvar_wait(&queue_notempty[periph_no],&queue_mutexes[periph_no]);
        }
        DMSG("check loop %d wake",periph_no);
        // dequeue & handle req
        dequeue(&req_queues[periph_no],&req);
        // unlock
        mutex_unlock(&queue_mutexes[periph_no]);
        // handle req
        handle_req(&req,periph_no);
        // set signal ready
        set_ready_signal(periph_no,req.signo);
    }
    return TEE_SUCCESS;
}

static TEE_Result read_req(uint32_t param_types,
                           TEE_Param params[4]){
#ifdef OVERHEAD_BENCHMARK
    TEE_Time t1;
    tee_time_get_sys_time(&t1);
    benchmark_time[req_idx*IO_HELPER_SERVICE_OVERHEAD_BENCHMARK_POINTS]=teetime_to_micro(t1);
#endif
    uint16_t periph_no;
    io_req_t req;
    int res_signo;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
											   TEE_PARAM_TYPE_VALUE_INPUT,
											   TEE_PARAM_TYPE_VALUE_INPUT,
											   TEE_PARAM_TYPE_VALUE_INPUT);
    if(param_types!=exp_param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    if(params[0].value.a>=N_PERIPH){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    // allocate a signo
    periph_no=(uint16_t)params[0].value.a;
    res_signo=allocate_signo(periph_no);
    if(res_signo<0){
        DMSG("io helper %d busy",periph_no);
        return TEE_ERROR_BUSY;
    }
    DMSG("periph %d, allocated sub signo %d for read req",periph_no,res_signo);
    // get req type
    req.type=(io_req_type_t)params[0].value.b;
    // output signo
    params[0].value.a=res_signo;
    // set req signo
    req.signo=res_signo;
    req.params[0]=params[1];
    req.params[1]=params[2];
    req.params[2]=params[3];

    mutex_lock(&queue_mutexes[periph_no]);
    // enqueue
    enqueue(&req_queues[periph_no],req);
    // signal to check loop, while holding the mutex
    condvar_signal(&queue_notempty[periph_no]);
    mutex_unlock(&queue_mutexes[periph_no]);
#ifdef OVERHEAD_BENCHMARK
    TEE_Time t2;
    tee_time_get_sys_time(&t2);
    benchmark_time[req_idx*IO_HELPER_SERVICE_OVERHEAD_BENCHMARK_POINTS+1]=teetime_to_micro(t2);
    if(req_idx<IO_HELPER_SERVICE_OVERHEAD_BENCHMARK_TIMES-1){
        req_idx++;
    }
#endif
    return TEE_SUCCESS;
}

// add a buffer in, compare to read_req
static TEE_Result read_req_with_buffer(uint32_t param_types,
                                       TEE_Param params[4]){
    uint16_t periph_no;
    io_req_t req;
    int res_signo;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                               TEE_PARAM_TYPE_MEMREF_INPUT,
                                               TEE_PARAM_TYPE_VALUE_INPUT,
                                               TEE_PARAM_TYPE_VALUE_INPUT);
    if(param_types!=exp_param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    if(params[0].value.a>=N_PERIPH){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    // allocate a signo
    periph_no=(uint16_t)params[0].value.a;
    // get req type
    req.type=(io_req_type_t)params[0].value.b;
    // try malloc mem for input buffer
    req.params[0].memref.buffer=(void*)malloc(params[1].memref.size);
    if(__builtin_expect(req.params[0].memref.buffer==NULL,0)){
        // malloc failed
        return TEE_ERROR_OUT_OF_MEMORY;
    }
    req.params[0].memref.size=params[1].memref.size;

    res_signo=allocate_signo(periph_no);
    if(res_signo<0){
        DMSG("io helper %d busy",periph_no);
        return TEE_ERROR_BUSY;
    }
    DMSG("periph %d, allocated sub signo %d for read req",periph_no,res_signo);
    // output signo
    params[0].value.a=res_signo;
    // set req signo
    req.signo=res_signo;
    // memcpy input buffer to req
    memcpy(req.params[0].memref.buffer,params[1].memref.buffer,req.params[0].memref.size);
    req.params[1]=params[2];
    req.params[2]=params[3];

    mutex_lock(&queue_mutexes[periph_no]);
    // enqueue
    enqueue(&req_queues[periph_no],req);
    // signal to check loop, while holding the mutex
    condvar_signal(&queue_notempty[periph_no]);
    mutex_unlock(&queue_mutexes[periph_no]);
    return TEE_SUCCESS;
}

// same pattern as read_req_with_buffer
static TEE_Result write_req(uint32_t param_types,
                            TEE_Param params[4]){
    uint16_t periph_no;
    io_req_t req;
    int res_signo;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                               TEE_PARAM_TYPE_MEMREF_INPUT,
                                               TEE_PARAM_TYPE_VALUE_INPUT,
                                               TEE_PARAM_TYPE_VALUE_INPUT);
    if(param_types!=exp_param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    if(params[0].value.a>=N_PERIPH){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    // allocate a signo
    periph_no=(uint16_t)params[0].value.a;
    // get req type
    req.type=(io_req_type_t)params[0].value.b;
    // try malloc mem for write buffer
    if(params[1].memref.size!=0){
        // write but buffer size = 0
        // special for transfer_bytes interface when write buffer is empty
        req.params[0].memref.buffer=(void*)malloc(params[1].memref.size);
        if(__builtin_expect(req.params[0].memref.buffer==NULL,0)){
            // malloc failed
            return TEE_ERROR_OUT_OF_MEMORY;
        }
    }
    req.params[0].memref.size=params[1].memref.size;

    res_signo=allocate_signo(periph_no);
    if(res_signo<0){
        DMSG("io helper %d busy",periph_no);
        return TEE_ERROR_BUSY;
    }
    DMSG("periph %d, allocated sub signo %d for read req",periph_no,res_signo);
    // output signo
    params[0].value.a=res_signo;
    // set req signo
    req.signo=res_signo;
    // memcpy input buffer to req
    if(params[1].memref.size!=0){
        // write but buffer size = 0
        // special case for spi_transfer_bytes when write buffer is empty
        memcpy(req.params[0].memref.buffer,params[1].memref.buffer,req.params[0].memref.size);
    }
    req.params[1]=params[2];
    req.params[2]=params[3];

    mutex_lock(&queue_mutexes[periph_no]);
    // enqueue
    enqueue(&req_queues[periph_no],req);
    // signal to check loop, while holding the mutex
    condvar_signal(&queue_notempty[periph_no]);
    mutex_unlock(&queue_mutexes[periph_no]);
    return TEE_SUCCESS;
}

// add a buffer in compare to write req
static TEE_Result write_req_with_buffer(uint32_t param_types,
                            TEE_Param params[4]){
    uint16_t periph_no;
    io_req_t req;
    int res_signo;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                               TEE_PARAM_TYPE_MEMREF_INPUT,
                                               TEE_PARAM_TYPE_MEMREF_INPUT,
                                               TEE_PARAM_TYPE_VALUE_INPUT);
    if(param_types!=exp_param_types){
        DMSG("error for wrong parameter types");
        return TEE_ERROR_BAD_PARAMETERS;
    }
    if(params[0].value.a>=N_PERIPH){
        DMSG("error for wrong periph no %d",params[0].value.a);
        return TEE_ERROR_BAD_PARAMETERS;
    }
    // allocate a signo
    periph_no=(uint16_t)params[0].value.a;
    // get req type
    req.type=(io_req_type_t)params[0].value.b;
    // try malloc mem for input buffer
    req.params[0].memref.buffer=(void*)malloc(params[1].memref.size);
    if(__builtin_expect(req.params[0].memref.buffer==NULL,0)){
        // malloc failed
        return TEE_ERROR_OUT_OF_MEMORY;
    }
    req.params[0].memref.size=params[1].memref.size;
    // try malloc mem for write buffer
    req.params[1].memref.buffer=(void*)malloc(params[2].memref.size);
    if(__builtin_expect(req.params[1].memref.buffer==NULL,0)){
        // malloc failed
        return TEE_ERROR_OUT_OF_MEMORY;
    }
    req.params[1].memref.size=params[2].memref.size;

    res_signo=allocate_signo(periph_no);
    if(res_signo<0){
        DMSG("io helper %d busy",periph_no);
        return TEE_ERROR_BUSY;
    }
    DMSG("periph %d, allocated sub signo %d for read req",periph_no,res_signo);
    // output signo
    params[0].value.a=res_signo;
    // set req signo
    req.signo=res_signo;
    // memcpy input and write buffer to req
    memcpy(req.params[0].memref.buffer,params[1].memref.buffer,req.params[0].memref.size);
    memcpy(req.params[1].memref.buffer,params[2].memref.buffer,req.params[1].memref.size);

    req.params[2]=params[3];

    mutex_lock(&queue_mutexes[periph_no]);
    // enqueue
    enqueue(&req_queues[periph_no],req);
    // signal to check loop, while holding the mutex
    condvar_signal(&queue_notempty[periph_no]);
    mutex_unlock(&queue_mutexes[periph_no]);
    return TEE_SUCCESS;
}

static TEE_Result poll(uint32_t param_types, TEE_Param params[4]){
    int signo;
    uint16_t periph_no;
    uint32_t req_num;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE);
    if(param_types!=exp_param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    // get periph no
    periph_no=(uint16_t)params[0].value.a;
    if(periph_no>=N_PERIPH){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    signo=get_ready_signal(periph_no);
    // DMSG("get_ready_signal return %d",signo);
    if(signo<0){
        req_num=get_req_num(periph_no);
		if(req_num>0){
			// a>0 means still have processing req
			params[0].value.a=1;
			// b=0 means no ready signal
			params[0].value.b=0xFFFFFFFF;
		}else{
			// req_num = 0
			params[0].value.a=0;
		}
	}else{
		// signo >= 0, means we have req done
		params[0].value.a=1;
		params[0].value.b=(uint32_t)signo;
	}
	return TEE_SUCCESS;
}

static TEE_Result poll_subsignal(uint32_t param_types, TEE_Param params[4]){
    uint16_t periph_no;
    uint32_t sub_signo;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE);
    if(param_types!=exp_param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    // get periph no & sub_signo
    periph_no=(uint16_t)params[0].value.a;
    sub_signo=params[0].value.b;
    if(periph_no>=N_PERIPH||sub_signo>=N_SUB_SIG){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    // check subsigno is using
    if(!is_using_signo(periph_no,sub_signo)){
        DMSG("signo not using");
        return TEE_ERROR_BAD_STATE;
    }

    // for test, count poll times
    io_helper_signal_table[periph_no][sub_signo].poll_times++;

    if(is_ready_signal(periph_no,sub_signo)){
        // signal ready, return 1
        params[0].value.a=1;
    }else{
        // not ready, return 0
        params[0].value.a=0;
    }
    return TEE_SUCCESS;             
}

static TEE_Result get_data(uint32_t param_types, TEE_Param params[4]){
#ifdef OVERHEAD_BENCHMARK
    TEE_Time t1;
    tee_time_get_sys_time(&t1);
    benchmark_time[get_data_idx*IO_HELPER_SERVICE_OVERHEAD_BENCHMARK_POINTS+2]=teetime_to_micro(t1);
#endif
    uint16_t periph_no;
    uint32_t sub_signo;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
											   TEE_PARAM_TYPE_MEMREF_OUTPUT,
											   TEE_PARAM_TYPE_NONE,
											    TEE_PARAM_TYPE_NONE);
    if(param_types!=exp_param_types){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    periph_no=(uint16_t)params[0].value.a;
    sub_signo=params[0].value.b;
	if(sub_signo>N_SUB_SIG){
		DMSG("Wrong sub signo");
		return TEE_ERROR_BAD_PARAMETERS;
	}
    // check sub_signo ready
    if(!is_ready_signal(periph_no,sub_signo)){
		DMSG("periph %d sub signal %d is not ready",periph_no,sub_signo);
		return TEE_ERROR_NO_DATA;
	}
    // ret < 0, req success
	if(io_helper_signal_table[periph_no][sub_signo].ret<0){
		// io req failed
		// DMSG("ret < 0");
		params[0].value.a=io_helper_signal_table[periph_no][sub_signo].ret;
		// reduce request num, release signo
		release_signo(periph_no,sub_signo);
		return TEE_SUCCESS;
	}
    // first, may request data len
	if(params[1].memref.size<io_helper_signal_table[periph_no][sub_signo].len){
		params[1].memref.size=io_helper_signal_table[periph_no][sub_signo].len;
		// may try again, so don't release signo
		return TEE_ERROR_SHORT_BUFFER;
	}

    if(io_helper_signal_table[periph_no][sub_signo].len!=0&&io_helper_signal_table[periph_no][sub_signo].buf==NULL){
        // something wrong, we don't have data buf
        DMSG("Data buf is NULL");
		release_signo(periph_no,sub_signo);
		return TEE_ERROR_NO_DATA;
	}else if(io_helper_signal_table[periph_no][sub_signo].len==0){
        // return no data and req success
        params[0].value.a=io_helper_signal_table[periph_no][sub_signo].ret;
        params[1].memref.size=0; 
        release_signo(periph_no,sub_signo);
        return TEE_SUCCESS;
    }

    // move sigtable buf data to params out
    memcpy(params[1].memref.buffer,io_helper_signal_table[periph_no][sub_signo].buf,io_helper_signal_table[periph_no][sub_signo].len);
	// free signal buf
	free(io_helper_signal_table[periph_no][sub_signo].buf);
	
	params[0].value.a=io_helper_signal_table[periph_no][sub_signo].ret;
    // for test, return poll times
    params[0].value.b=io_helper_signal_table[periph_no][sub_signo].poll_times;
	params[1].memref.size=io_helper_signal_table[periph_no][sub_signo].len;
	// reset len and ret value
	io_helper_signal_table[periph_no][sub_signo].len=0;
	io_helper_signal_table[periph_no][sub_signo].ret=0;

    // for test, reset poll times
    io_helper_signal_table[periph_no][sub_signo].poll_times=0;

	// reduce request num
	// release signal using & pending
	release_signo(periph_no,sub_signo);
	// DMSG("reduce req_num to %d",req_num);
#ifdef OVERHEAD_BENCHMARK
    TEE_Time t2;
    tee_time_get_sys_time(&t2);
    benchmark_time[get_data_idx*IO_HELPER_SERVICE_OVERHEAD_BENCHMARK_POINTS+3]=teetime_to_micro(t2);
    if(get_data_idx<IO_HELPER_SERVICE_OVERHEAD_BENCHMARK_TIMES-1){
        get_data_idx++;
    }
#endif
	return TEE_SUCCESS;
}

static TEE_Result get_overhead_timestamp(uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS]){
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
    req_idx=0;
    get_data_idx=0;
    return TEE_SUCCESS;
#else
    return TEE_ERROR_NOT_IMPLEMENTED;
#endif
}

static TEE_Result invoke_command(void __maybe_unused * session_context,
                uint32_t cmd_id,
                uint32_t param_types,
                TEE_Param params[TEE_NUM_PARAMS]){
    TEE_Result res = TEE_SUCCESS;
    switch (cmd_id)
    {
    case PTA_IO_HELPER_CMD_START_LOOP:
        res=start_loop(param_types,params);
        break;
    case PTA_IO_HELPER_CMD_READ_REQ:
        res=read_req(param_types,params);
        break;
    case PTA_IO_HELPER_CMD_POLL:
        res=poll(param_types,params);
        break;
    case PTA_IO_HELPER_CMD_GET_DATA:
        res=get_data(param_types,params);
        break;
    case PTA_IO_HELPER_CMD_POLL_SUBSIGNAL:
        res=poll_subsignal(param_types,params);
        break;
    case PTA_IO_HELPER_CMD_READ_REQ_WITH_BUFFER:
        res=read_req_with_buffer(param_types,params);
        break;
    case PTA_IO_HELPER_CMD_WRITE_REQ:
        res=write_req(param_types,params);
        break;
    case PTA_IO_HELPER_CMD_WRITE_REQ_WITH_BUFFER:
        res=write_req_with_buffer(param_types,params);
        break;
    case PTA_IO_HELPER_CMD_GET_OVERHEAD_TIMESTAMP:
        res=get_overhead_timestamp(param_types,params);
        break;
    default:
        EMSG("cmd: %d not supported %s",cmd_id,IO_HELPER_PTA_NAME);
        res=TEE_ERROR_NOT_SUPPORTED;
        break;
    }
    return res;
}

pseudo_ta_register(.uuid=PTA_IO_HELPER_UUID,
                .name = IO_HELPER_PTA_NAME,
                .flags = PTA_DEFAULT_FLAGS | TA_FLAG_CONCURRENT,
                .create_entry_point = create,
                .invoke_command_entry_point = invoke_command);