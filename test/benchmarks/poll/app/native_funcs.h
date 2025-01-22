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

__attribute__((import_name("at24cxx_read"))) int
at24cxx_read(uint32_t i2c_no,uint32_t i2c_addr, uint32_t read_addr, uint32_t dst_buf_addr, uint32_t len);


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

__attribute__((import_name("at24cxx_read_async_future"))) int
at24cxx_read_async_future(uint32_t i2c_no,uint32_t i2c_addr, uint32_t read_addr, uint32_t read_len);

__attribute__((import_name("at24cxx_read_async"))) int
at24cxx_read_async(uint32_t i2c_no,uint32_t i2c_addr, uint32_t read_addr, uint32_t read_len, uint32_t handler_func_ptr);

__attribute__((import_name("mpu6050_sample_async_future"))) int
mpu6050_sample_async_future(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("sign_ptr")))
uint32_t sign_ptr(uint32_t original_ptr, uint32_t signature, uint32_t tag);

__attribute__((import_name("native_wait"))) int 
native_wait(int ms);

__attribute__((import_name("i2c_read_bytes"))) int
i2c_read_bytes(uint32_t i2c_dev, uint32_t addr, uint32_t flags, uint32_t dst_buf_addr, uint32_t buf_len);

__attribute__((import_name("i2c_read_bytes_async_future"))) int
i2c_read_bytes_async_future(uint32_t i2c_dev, uint32_t addr, uint32_t flags, uint32_t buf_len);

__attribute__((import_name("i2c_read_bytes_async"))) int
i2c_read_bytes_async(uint32_t i2c_dev, uint32_t addr, uint32_t flags, uint32_t buf_len, uint32_t handler_func_ptr);


__attribute__((import_name("i2c_write_bytes"))) int
i2c_write_bytes(uint32_t i2c_no, uint32_t addr, uint32_t flags, uint32_t src_buf_addr, uint32_t buf_len);

__attribute__((import_name("i2c_read_regs"))) int
i2c_read_regs(uint32_t i2c_dev, uint32_t addr, uint32_t flags, uint32_t reg, uint32_t dst_buf_addr, uint32_t buf_len);

__attribute__((import_name("i2c_write_regs"))) int
i2c_write_regs(uint32_t i2c_no, uint32_t addr, uint32_t flags, uint32_t reg, uint32_t src_buf_addr, uint32_t buf_len);

__attribute__((import_name("i2c_read_regs_async_future"))) int 
i2c_read_regs_async_future(uint32_t i2c_no, uint32_t i2c_addr,uint32_t flags, uint32_t reg, uint32_t read_len);

__attribute__((import_name("bh1750fvi_init"))) int
bh1750fvi_init(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("bh1750fvi_sample"))) int
bh1750fvi_sample(uint32_t i2c_no, uint32_t addr, uint32_t dst_lux_addr);

__attribute__((import_name("bh1750fvi_sample_async_future"))) int
bh1750fvi_sample_async_future(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("write_secure_object"))) int
write_secure_object(uint32_t id_addr, uint32_t id_len, uint32_t data_addr, uint32_t data_len);

__attribute__((import_name("dht_init"))) int
dht_init(uint32_t pin, uint32_t type);

__attribute__((import_name("dht_read"))) int
dht_read(uint32_t pin, uint32_t type, uint32_t dst_temp_addr, uint32_t dst_hum_addr);

__attribute__((import_name("dht_read_async_future"))) int dht_read_async_future(uint32_t pin, uint32_t type);



__attribute__((import_name("record_benchmark_time"))) int record_benchmark_time(uint32_t idx);
__attribute__((import_name("get_benchmark_time"))) int get_benchmark_time(uint32_t idx, uint32_t out_time_addr);

__attribute__((import_name("gpio_init"))) int
gpio_init(int pin, int func_code);

__attribute__((import_name("gpio_set"))) int
gpio_set(int pin, int val);

__attribute__((import_name("gpio_get"))) int
gpio_get(int pin);

__attribute__((import_name("record_poll_times"))) int
record_poll_times(uint32_t times);

__attribute__((import_name("get_poll_times"))) int
get_poll_times(uint32_t dst_poll_times_addr);

__attribute__((import_name("get_runtime_poll_times"))) int
get_runtime_poll_times(uint32_t dst_poll_times_addr);

__attribute__((import_name("get_runtime_profile_times"))) int
get_runtime_profile_times(uint32_t dst_profile_times_addr);

__attribute__((import_name("all_async_req_handled"))) int
all_async_req_handled();

__attribute__((import_name("get_async_data"))) int get_async_data(uint32_t signo,
                                                                  uint32_t sub_signo,
                                                                  uint32_t dst_buf_addr,
                                                                  uint32_t dst_buf_len_addr);

__attribute__((import_name("test_ta_get_sys_time_overhead"))) int test_ta_get_sys_time_overhead();


// for test only
#define SIGNATURE 0xE5
#define TAG 0x12

#endif