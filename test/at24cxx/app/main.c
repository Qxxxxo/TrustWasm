/**
 * @brief wasm use native at24cxx
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define PERIPH_NO 28

__attribute__((import_name("i2c_read_bytes"))) int
i2c_read_bytes(uint32_t i2c_dev, uint32_t addr, uint32_t flags, uint32_t dst_buf_addr, uint32_t buf_len);

__attribute__((import_name("i2c_write_bytes"))) int
i2c_write_bytes(uint32_t i2c_no, uint32_t addr, uint32_t flags, uint32_t src_buf_addr, uint32_t buf_len);

__attribute__((import_name("i2c_read_regs"))) int
i2c_read_regs(uint32_t i2c_dev, uint32_t addr, uint32_t flags, uint32_t reg, uint32_t dst_buf_addr, uint32_t buf_len);

__attribute__((import_name("i2c_write_regs"))) int
i2c_write_regs(uint32_t i2c_no, uint32_t addr, uint32_t flags, uint32_t reg, uint32_t src_buf_addr, uint32_t buf_len);

__attribute__((import_name("at24cxx_read"))) int
at24cxx_read(uint32_t i2c_no,uint32_t i2c_addr, uint32_t read_addr, uint32_t dst_buf_addr, uint32_t len);

__attribute__((import_name("at24cxx_write"))) int
at24cxx_write(uint32_t i2c_no,uint32_t i2c_addr, uint32_t write_addr, uint32_t src_buf_addr, uint32_t len);

__attribute__((import_name("at24cxx_read_async_future"))) int
at24cxx_read_async_future(uint32_t i2c_no,uint32_t i2c_addr, uint32_t read_addr, uint32_t read_len);

__attribute__((import_name("at24cxx_write_async_future"))) int
at24cxx_write_async_future(uint32_t i2c_no,uint32_t i2c_addr, uint32_t write_addr,uint32_t src_buf_addr, uint32_t src_len);

__attribute__((import_name("get_async_data_future"))) int get_async_data_future(uint32_t signo,
                                                                                uint32_t sub_signo,
                                                                                uint32_t dst_buf_addr,
                                                                                uint32_t dst_buf_len_addr);

__attribute__((import_name("native_wait"))) int 
native_wait(int ms);

#define AT24CXX_ADDR 0x50
#define AT24C256_PAGE_SIZE 64

void print_array(uint8_t * buf, uint32_t len){
    for(uint32_t i=0;i<len;i++){
        printf("%x ",buf[i]);
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    uint8_t write_data[10] = {0x00,0x00,0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    uint8_t read_data[8] = {0};
    uint16_t mem_addr = 0x00;  // Memory address to read/write
    uint8_t mem_addr_data[2]={0x00,0x00};

    // use i2c api

    // Write test data
    printf("Writing data to AT24CXX:\n");
    printf("%x\n", i2c_write_bytes(0, AT24CXX_ADDR, 0, write_data, 10)); 
    print_array(write_data, 10);
    // Wait for write completion
    native_wait(10);
    // Read back data
    printf("Reading data from AT24CXX:\n");
    printf("%x\n", i2c_write_bytes(0, AT24CXX_ADDR, 0,mem_addr_data,2));
    printf("%x\n", i2c_read_bytes(0, AT24CXX_ADDR, 0,read_data,8));
    print_array(read_data, 8);
    // Verify data
    printf("Verification:\n");
    for(int i = 0; i < 8; i++) {
        if(write_data[i+2] != read_data[i]) {
            printf("Data mismatch at index %d\n", i);
            return 0;
        }
    }
    printf("Data verified successfully\n");

    // use at24cxx api
    uint8_t another_write_data[8] = {0x16, 0x17, 0x18, 0x19, 0x55, 0x66, 0x77, 0x88};
    mem_addr=0x02;
    
    at24cxx_write(0, AT24CXX_ADDR, mem_addr, another_write_data, 8);
    at24cxx_read(0, AT24CXX_ADDR, mem_addr, read_data, 8);
    print_array(read_data, 8);

    int future;
    int res;
    mem_addr=0x00;
    // async
    future=at24cxx_write_async_future(0,AT24CXX_ADDR, mem_addr,another_write_data,8);
    if(future<0){
        printf("send async write failed with %x\n",future);
    }else{
        res=get_async_data_future(PERIPH_NO,future,NULL,0);
        printf("async write result %x\n",res);
    }
    int len = 8;
    future=at24cxx_read_async_future(0,AT24CXX_ADDR, mem_addr,8);
    if(future<0){
        printf("read async write failed with %x\n",future);
    }else{
        res=get_async_data_future(PERIPH_NO,future,read_data,&len);
        printf("read async result %x, len %d\n",res,len);
        if(res>0){
           print_array(read_data, 8); 
        }
    }
    return 0;
}

