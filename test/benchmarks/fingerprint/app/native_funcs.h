#ifndef NATIVE_FUNCS_H  
#define NATIVE_FUNCS_H

#include <stdint.h>

__attribute__((import_name("atk301_init"))) int
atk301_init(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("atk301_get_fingerprint_image"))) int
atk301_get_fingerprint_image(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("atk301_upload_fingerprint_image"))) int
atk301_upload_fingerprint_image(uint32_t i2c_no, uint32_t addr, uint32_t dst_buf_addr);

__attribute__((import_name("atk301_get_fingerprint_image_async_future"))) int
atk301_get_fingerprint_image_async_future(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("atk301_upload_fingerprint_image_async_future"))) int
atk301_upload_fingerprint_image_async_future(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("at24cxx_write"))) int
at24cxx_write(uint32_t i2c_no,uint32_t i2c_addr, uint32_t write_addr, uint32_t src_buf_addr, uint32_t len);

__attribute__((import_name("at24cxx_write_async_future"))) int
at24cxx_write_async_future(uint32_t i2c_no,uint32_t i2c_addr, uint32_t write_addr, uint32_t src_buf_addr, uint32_t len);

__attribute__((import_name("record_benchmark_time"))) int record_benchmark_time(uint32_t idx);
__attribute__((import_name("get_benchmark_time"))) int get_benchmark_time(uint32_t idx, uint32_t out_time_addr);

__attribute__((import_name("sign_ptr")))
uint32_t sign_ptr(uint32_t original_ptr, uint32_t signature, uint32_t tag);

__attribute__((import_name("native_wait"))) int 
native_wait(int ms);

__attribute__((import_name("get_async_data_future"))) int get_async_data_future(uint32_t signo,
                                                                                uint32_t sub_signo,
                                                                                uint32_t dst_buf_addr,
                                                                                uint32_t dst_buf_len_addr);

__attribute__((import_name("all_async_req_handled"))) int
all_async_req_handled();

__attribute__((import_name("get_async_data"))) int get_async_data(uint32_t signo,
                                                                  uint32_t sub_signo,
                                                                  uint32_t dst_buf_addr,
                                                                  uint32_t dst_buf_len_addr);

__attribute__((import_name("atk301_get_fingerprint_image_async"))) int
atk301_get_fingerprint_image_async(uint32_t i2c_no, uint32_t addr, uint32_t handler_func_ptr);

__attribute__((import_name("atk301_upload_fingerprint_image_async"))) int
atk301_upload_fingerprint_image_async(uint32_t i2c_no, uint32_t addr, uint32_t handler_func_ptr);

__attribute__((import_name("at24cxx_write_async"))) int
at24cxx_write_async(uint32_t i2c_no,uint32_t i2c_addr, uint32_t write_addr, uint32_t src_buf_addr, uint32_t len, uint32_t handler_func_ptr);

// for test only
#define SIGNATURE 0xE5
#define TAG 0x12

#endif