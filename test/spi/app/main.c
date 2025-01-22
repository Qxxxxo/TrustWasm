/**
 * @brief wasm use spi native
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

__attribute__((import_name("spi_transfer_diff_bytes"))) int
spi_transfer_diff_bytes(uint32_t spi_no,uint32_t spi_cs_no, uint32_t cont,
                             uint32_t mode, uint32_t freq,  uint32_t src_len, uint32_t dst_len,
                              uint32_t src_buf_addr, uint32_t dst_buf_addr);

__attribute__((import_name("spi_transfer_byte"))) int
spi_transfer_byte(uint32_t spi_no,uint32_t spi_cs_no, uint32_t cont,
                             uint32_t mode, uint32_t freq,  uint32_t need_byte_out, uint32_t need_byte_in,
                              uint32_t byte_out, uint32_t dst_byte_addr);

__attribute__((import_name("sign_ptr")))
uint32_t sign_ptr(uint32_t original_ptr, uint32_t signature, uint32_t tag);

__attribute__((import_name("native_sleep"))) int 
native_sleep(int ms);

void print_array(uint8_t * buf, uint32_t len){
    for(uint32_t i=0;i<len;i++){
        printf("%x ",buf[i]);
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    char * src_buf="Hello, SPI!";
    char dst_buf[11]={0};
    uint32_t signed_src=sign_ptr(src_buf,SIGNATURE,TAG);
    uint32_t signed_dst=sign_ptr(dst_buf,SIGNATURE,TAG);
    printf("%x\n",spi_transfer_bytes(0,1,0,0,0,11,11,signed_src,signed_dst));
    // printf("%x\n",spi_transfer_diff_bytes(0,1,0,0,1,11,11,signed_src,signed_dst));
    print_array(dst_buf,11);
    printf("%x\n",spi_transfer_bytes(0,1,0,0,0,11,11,signed_src,signed_dst));
    print_array(dst_buf,11);
}

