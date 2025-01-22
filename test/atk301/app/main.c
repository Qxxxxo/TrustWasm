/**
 * @brief wasm use native atk301 fingerprint sensor
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

__attribute__((import_name("atk301_init"))) int
atk301_init(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("atk301_get_fingerprint_image"))) int
atk301_get_fingerprint_image(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("atk301_upload_fingerprint_image"))) int
atk301_upload_fingerprint_image(uint32_t i2c_no, uint32_t addr, uint32_t dst_buf_addr);

__attribute__((import_name("i2c_read_bytes"))) int
i2c_read_bytes(uint32_t i2c_no, uint32_t addr, uint32_t flags, uint32_t dst_buf_addr, uint32_t buf_len);

__attribute__((import_name("i2c_write_bytes"))) int
i2c_write_bytes(uint32_t i2c_no, uint32_t addr, uint32_t flags, uint32_t src_buf_addr, uint32_t buf_len);

__attribute__((import_name("native_sleep"))) int 
native_sleep(int ms);

void print_fingerprint_image(uint8_t * buf, uint32_t len){
    for(int i=0;i<len;i++){
        printf("0x%02x ",buf[i]);
        if(i%32==31){
            printf("\n");
        }
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    uint8_t data[12800]={0};
    uint8_t head[3]={0};
    uint8_t cmd;
    if(atk301_init(0, 0x27)==0){
        if(atk301_get_fingerprint_image(0, 0x27)==0){
            if(atk301_upload_fingerprint_image(0, 0x27, (uint32_t)data)==0){
                print_fingerprint_image(data,12800);
            }
        }else{
            printf("get fingerprint image failed\n");
        }
    }else{
        printf("init failed\n");
    }
    
    // cmd=0x01;
    // i2c_write_bytes(0, 0x27, 0, &cmd, 1);
    // native_sleep(1050);
    
    // i2c_read_bytes(0,0x27,0,(uint32_t)head,1);
    // printf("handshake respond\n");
    // print_fingerprint_image(head,3);
    // i2c_read_bytes(0,0x27,0,(uint32_t)head,3);
    // printf("handshake respond\n");
    // print_fingerprint_image(head,3);
    
    // if(head[0]==0x01){
    //     head[0]=0x00;
    //     cmd=0x02;
    //     i2c_write_bytes(0,0x27,0,&cmd,1);
    //     native_sleep(1100);
    //     i2c_read_bytes(0,0x27,0,(uint32_t)head,1);
    //     printf("get fingerprint image\n");
    //     print_fingerprint_image(head,3);
    //     i2c_read_bytes(0,0x27,0,(uint32_t)head,3);
    //     printf("get fingerprint image\n");
    //     print_fingerprint_image(head,3);
    //     if(head[0]==0x01){
    //         printf("get fingerprint image success\n");
    //         print_fingerprint_image(head,3);
    //         head[0]=0x00;
    //         cmd=0x03;
    //         i2c_write_bytes(0,0x27,0,&cmd,1);
    //         native_sleep(3450);
    //         i2c_read_bytes(0,0x27,0,(uint32_t)head,1);
    //         printf("upload fingerprint image\n");
    //         print_fingerprint_image(head,3);
    //         i2c_read_bytes(0,0x27,0,(uint32_t)head,3);
    //         printf("upload fingerprint image\n");
    //         print_fingerprint_image(head,3);
    //         if(head[0]==0x02){
    //             printf("upload success\n");
    //             for(int i=0;i<80;i++){
    //                 i2c_read_bytes(0,0x27,0,(uint32_t)&(data[i*160]),160);
    //             }
    //             print_fingerprint_image(data,12800);
    //         }else{
    //             printf("upload fingerprint image failed\n");
    //             print_fingerprint_image(head,3);
    //         }
    //     }else{
    //         printf("get fingerprint image failed\n");
    //         print_fingerprint_image(head,3);
    //     }
    // }else{
    //     printf("handshake failed\n");
    //     print_fingerprint_image(head,3);
    // }
    
}

