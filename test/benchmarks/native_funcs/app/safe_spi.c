/**
 * @brief spi native func
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// for test only
#define SIGNATURE 0xE5
#define TAG 0x12

__attribute__((import_name("spi_transfer_bytes"))) int
spi_transfer_bytes(uint32_t spi_no,uint32_t spi_cs_no, uint32_t cont,
                             uint32_t mode, uint32_t freq,  uint32_t src_len, uint32_t dst_len,
                              uint32_t src_buf_addr, uint32_t dst_buf_addr);

__attribute__((import_name("sign_ptr")))
uint32_t sign_ptr(uint32_t original_ptr, uint32_t signature, uint32_t tag);

__attribute__((import_name("native_wait"))) int 
native_wait(int ms);

__attribute__((import_name("record_benchmark_time"))) int record_benchmark_time(uint32_t idx);
__attribute__((import_name("get_benchmark_time"))) int get_benchmark_time(uint32_t idx, uint32_t out_time_addr);

#define SPI_TEST_NO 0
#define SPI_TEST_CS_NO 1
#define SPI_TEST_CONT 0
#define SPI_TEST_MODE 0
#define SPI_TEST_FREQ 0
#define SPI_TEST_LEN 4*1024

int main(int argc, char **argv)
{
    int res;
    uint64_t start,end;
    uint8_t * src_data;
    uint8_t * dst_data;
    uint32_t signed_src_data;
    uint32_t signed_dst_data;

    // I2C native funcs
    src_data = (uint8_t*)malloc(SPI_TEST_LEN);
    dst_data = (uint8_t*)malloc(SPI_TEST_LEN);
    record_benchmark_time(0);
    signed_src_data=sign_ptr(src_data,SIGNATURE,TAG);
    signed_dst_data=sign_ptr(dst_data,SIGNATURE,TAG);
    res=spi_transfer_bytes(SPI_TEST_NO,SPI_TEST_CS_NO,SPI_TEST_CONT,
                            SPI_TEST_MODE,SPI_TEST_FREQ,SPI_TEST_LEN,SPI_TEST_LEN,
                            signed_src_data,signed_dst_data);
    record_benchmark_time(1);
    get_benchmark_time(0,(uint32_t)&start);
    get_benchmark_time(1,(uint32_t)&end);
    printf("res %x, spi_transfer_bytes (4K data) interval %llu\n",res,end-start);
    free(src_data);
    free(dst_data);
}

