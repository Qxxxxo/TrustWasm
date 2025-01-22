/**
 * @brief i2c funcs
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


__attribute__((import_name("i2c_read_bytes"))) int
i2c_read_bytes(uint32_t i2c_no, uint32_t addr, uint32_t flags, uint32_t dst_buf_addr, uint32_t buf_len);

__attribute__((import_name("i2c_write_bytes"))) int
i2c_write_bytes(uint32_t i2c_no, uint32_t addr, uint32_t flags, uint32_t src_buf_addr, uint32_t buf_len);

__attribute__((import_name("sign_ptr")))
uint32_t sign_ptr(uint32_t original_ptr, uint32_t signature, uint32_t tag);

__attribute__((import_name("native_wait"))) int 
native_wait(int ms);

__attribute__((import_name("record_benchmark_time"))) int record_benchmark_time(uint32_t idx);
__attribute__((import_name("get_benchmark_time"))) int get_benchmark_time(uint32_t idx, uint32_t out_time_addr);

#define I2C_TEST_NO 0
#define I2C_TEST_ADDR 0x55
#define I2C_TEST_FLAG 0
#define I2C_TEST_LEN 4*1024

int main(int argc, char **argv)
{
    int res;
    uint64_t start,end;
    uint8_t * data;
    
    // I2C native funcs
    data = (uint8_t*)malloc(I2C_TEST_LEN);
    record_benchmark_time(0);
    res=i2c_write_bytes(I2C_TEST_NO,I2C_TEST_ADDR,I2C_TEST_FLAG,data,I2C_TEST_LEN);
    record_benchmark_time(1);
    get_benchmark_time(0,(uint32_t)&start);
    get_benchmark_time(1,(uint32_t)&end);
    printf("res %x, i2c_write_bytes (4K data) interval %llu\n",res,end-start);
    free(data);

    data = (uint8_t*)malloc(I2C_TEST_LEN);
    record_benchmark_time(0);
    res=i2c_read_bytes(I2C_TEST_NO,I2C_TEST_ADDR,I2C_TEST_FLAG,data,I2C_TEST_LEN);
    record_benchmark_time(1);
    get_benchmark_time(0,(uint32_t)&start);
    get_benchmark_time(1,(uint32_t)&end);
    printf("res %x, i2c_read_bytes (4K data) interval %llu\n",res,end-start);
    free(data);
}

