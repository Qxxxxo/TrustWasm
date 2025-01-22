#ifndef _NATIVE_FUNCS_H
#define _NATIVE_FUNCS_H
#include <stdint.h>

typedef struct{
    uint16_t year; 
    uint8_t month;
    uint8_t date;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
}tel0157_time_t;

typedef struct{
    uint32_t lonMMMMM;
    uint32_t latMMMMM;
    uint8_t lonDD;
    uint8_t lonMM;
    uint8_t latDD;
    uint8_t latMM;
    char lonDirection;
    char latDirection;
}tel0157_lon_lat_t;

__attribute__((import_name("mpu6050_init"))) int
mpu6050_init(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("mpu6050_deinit"))) int
mpu6050_deinit(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("mpu6050_sample"))) int
mpu6050_sample(uint32_t i2c_no, uint32_t addr, uint32_t dst_addr);

__attribute__((import_name("at24cxx_write"))) int
at24cxx_write(uint32_t i2c_no,uint32_t i2c_addr, uint32_t write_addr, uint32_t src_buf_addr, uint32_t len);

__attribute__((import_name("tel0157_init"))) int
tel0157_init(uint32_t i2c_no, uint32_t addr, uint32_t gnss_mode, uint32_t rgb);

__attribute__((import_name("tel0157_deinit"))) int
tel0157_deinit(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("tel0157_get_gnss_len"))) int
tel0157_get_gnss_len(uint32_t i2c_no, uint32_t addr, uint32_t len_dst_addr);

__attribute__((import_name("tel0157_get_all_gnss"))) int
tel0157_get_all_gnss(uint32_t i2c_no, uint32_t addr, uint32_t dst_buf_addr,uint32_t len);

__attribute__((import_name("tel0157_get_utc_time"))) int
tel0157_get_utc_time(uint32_t i2c_no, uint32_t addr, uint32_t utc_dst_addr);

__attribute__((import_name("tel0157_get_gnss_mode"))) int
tel0157_get_gnss_mode(uint32_t i2c_no, uint32_t addr, uint32_t mode_dst_addr);

__attribute__((import_name("tel0157_get_num_sat_used"))) int
tel0157_get_num_sat_used(uint32_t i2c_no, uint32_t addr, uint32_t num_dst_addr);

__attribute__((import_name("tel0157_get_lon"))) int
tel0157_get_lon(uint32_t i2c_no, uint32_t addr, uint32_t lon_dst_addr);

__attribute__((import_name("tel0157_get_lat"))) int
tel0157_get_lat(uint32_t i2c_no, uint32_t addr, uint32_t lat_dst_addr);

__attribute__((import_name("tel0157_get_alt"))) int
tel0157_get_alt(uint32_t i2c_no, uint32_t addr, uint32_t alt_dst_addr);

__attribute__((import_name("tel0157_get_sog"))) int
tel0157_get_sog(uint32_t i2c_no, uint32_t addr, uint32_t sog_dst_addr);

__attribute__((import_name("tel0157_get_cog"))) int
tel0157_get_cog(uint32_t i2c_no, uint32_t addr, uint32_t cog_dst_addr);

__attribute__((import_name("tel0157_get_gnss_len_async_future"))) int
tel0157_get_gnss_len_async_future(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("tel0157_get_all_gnss_async_future"))) int
tel0157_get_all_gnss_async_future(uint32_t i2c_no, uint32_t addr, uint32_t len);

__attribute__((import_name("tel0157_get_utc_time_async_future"))) int
tel0157_get_utc_time_async_future(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("tel0157_get_gnss_mode_async_future"))) int
tel0157_get_gnss_mode_async_future(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("tel0157_get_num_sat_used_async_future"))) int
tel0157_get_num_sat_used_async_future(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("tel0157_get_lon_async_future"))) int
tel0157_get_lon_async_future(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("tel0157_get_lat_async_future"))) int
tel0157_get_lat_async_future(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("tel0157_get_alt_async_future"))) int
tel0157_get_alt_async_future(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("tel0157_get_sog_async_future"))) int
tel0157_get_sog_async_future(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("tel0157_get_cog_async_future"))) int
tel0157_get_cog_async_future(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("get_async_data_future"))) int get_async_data_future(uint32_t signo,
                                                                                uint32_t sub_signo,
                                                                                uint32_t dst_buf_addr,
                                                                                uint32_t dst_buf_len_addr);

__attribute__((import_name("at24cxx_write_async_future"))) int
at24cxx_write_async_future(uint32_t i2c_no,uint32_t i2c_addr, uint32_t write_addr, uint32_t src_buf_addr, uint32_t len);


__attribute__((import_name("mpu6050_sample_async_future"))) int
mpu6050_sample_async_future(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("sign_ptr")))
uint32_t sign_ptr(uint32_t original_ptr, uint32_t signature, uint32_t tag);

__attribute__((import_name("native_wait"))) int 
native_wait(int ms);

__attribute__((import_name("record_benchmark_time"))) int record_benchmark_time(uint32_t idx);
__attribute__((import_name("get_benchmark_time"))) int get_benchmark_time(uint32_t idx, uint32_t out_time_addr);

__attribute__((import_name("all_async_req_handled"))) int
all_async_req_handled();

__attribute__((import_name("get_async_data"))) int get_async_data(uint32_t signo,
                                                                  uint32_t sub_signo,
                                                                  uint32_t dst_buf_addr,
                                                                  uint32_t dst_buf_len_addr);

__attribute__((import_name("mpu6050_sample_async"))) int
mpu6050_sample_async(uint32_t i2c_no, uint32_t addr, uint32_t handler_func_ptr);

__attribute__((import_name("tel0157_get_utc_time_async"))) int
tel0157_get_utc_time_async(uint32_t i2c_no, uint32_t addr, uint32_t handler_func_ptr);

__attribute__((import_name("tel0157_get_lon_async"))) int
tel0157_get_lon_async(uint32_t i2c_no, uint32_t addr, uint32_t handler_func_ptr);

__attribute__((import_name("tel0157_get_lat_async"))) int
tel0157_get_lat_async(uint32_t i2c_no, uint32_t addr, uint32_t handler_func_ptr);

__attribute__((import_name("tel0157_get_alt_async"))) int
tel0157_get_alt_async(uint32_t i2c_no, uint32_t addr, uint32_t handler_func_ptr);

__attribute__((import_name("at24cxx_write_async"))) int
at24cxx_write_async(uint32_t i2c_no,uint32_t i2c_addr, uint32_t write_addr, uint32_t src_buf_addr, uint32_t len, uint32_t handler_func_ptr);

// for test only
#define SIGNATURE 0xE5
#define TAG 0x12

#endif