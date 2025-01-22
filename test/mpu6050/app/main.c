/**
 * @brief wasm use native mpu6050 
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// for test only
#define SIGNATURE 0xE5
#define TAG 0x12

__attribute__((import_name("mpu6050_init"))) int
mpu6050_init(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("mpu6050_deinit"))) int
mpu6050_deinit(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("mpu6050_sample"))) int
mpu6050_sample(uint32_t i2c_no, uint32_t addr, uint32_t dst_addr);

__attribute__((import_name("mpu6050_sample_async_future"))) int
mpu6050_sample_async_future(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("get_async_data_future"))) int get_async_data_future(uint32_t signo,
                                                                                uint32_t sub_signo,
                                                                                uint32_t dst_buf_addr,
                                                                                uint32_t dst_buf_len_addr);

__attribute__((import_name("sign_ptr")))
uint32_t sign_ptr(uint32_t original_ptr, uint32_t signature, uint32_t tag);

__attribute__((import_name("native_wait"))) int 
native_wait(int ms);

void print_mpu6050_data(uint8_t data[14]){
    int16_t ax=(data[0]<<8)|data[1];
    int16_t ay=(data[2]<<8)|data[3];
    int16_t az=(data[4]<<8)|data[5];
    int16_t raw_temp = (data[6]<<8)|data[7];
    int16_t gx=(data[8]<<8)|data[9];
    int16_t gy=(data[10]<<8)|data[11];
    int16_t gz=(data[12]<<8)|data[13];
    
    float temp=(float)raw_temp/340.0 + 36.53;
    float ax_g = (float)ax / 1638.4;
    float ay_g = (float)ay / 1638.4;
    float az_g = (float)az / 1638.4;
    float gx_dps = (float)gx / 131.0; 
    float gy_dps = (float)gy / 131.0;
    float gz_dps = (float)gz / 131.0;
    printf("sample data ax: %.2f, ay: %.2f, az: %.2f, gx: %.2f, gy: %.2f, gz: %.2f, temp: %.2f\n",
            ax_g,ay_g,az_g,gx_dps,gy_dps,gz_dps,temp);
}

int main(int argc, char **argv)
{
    int future;
    int ret;
    uint32_t len=14;
    uint8_t data[14]={0};
    uint32_t signed_data=sign_ptr(data,SIGNATURE,TAG);
    uint32_t signed_len=sign_ptr(&len,SIGNATURE,TAG);
    printf("origin: %x, signed %x\n",data,signed_data);
    printf("origin: %x, signed %x\n",&len,signed_len);
    if (mpu6050_init(1, 0x68) == 0)
    {
        // int i=0;
        // native_wait(100);
        // ret=mpu6050_sample(1, 0x68, signed_data);
        // // read from mpu6050
        // if(ret==0){
        //     print_mpu6050_data(data);
        // }else{
        //     printf("get sync data failed with %x\n",ret);
        // }
        // native_sleep(5000);
        
        // async 
        future=mpu6050_sample_async_future(1,0x68);
        if(future>=0){
            ret=get_async_data_future(29,future,signed_data,signed_len);
            if(ret>0){
                print_mpu6050_data(data);
            }else{
                printf("get async data failed with %x\n",ret);
            }
        }else{
            printf("failed to send async future with %x\n",future);
        }

        future=mpu6050_sample_async_future(1,0x68);
        if(future>=0){
            ret=get_async_data_future(29,future,signed_data,signed_len);
            if(ret>0){
                print_mpu6050_data(data);
            }else{
                printf("get async data failed with %x\n",ret);
            }
        }else{
            printf("failed to send async future with %x\n",future);
        }
        
        mpu6050_deinit(1,0x68);
    }
    return 0;
}

