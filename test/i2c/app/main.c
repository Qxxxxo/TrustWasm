/**
 * @brief wasm use i2c native
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

__attribute__((import_name("i2c_read_bytes"))) int
i2c_read_bytes(uint32_t i2c_no, uint32_t addr, uint32_t flags, uint32_t dst_buf_addr, uint32_t buf_len);

__attribute__((import_name("i2c_write_bytes"))) int
i2c_write_bytes(uint32_t i2c_no, uint32_t addr, uint32_t flags, uint32_t src_buf_addr, uint32_t buf_len);

__attribute__((import_name("i2c_read_regs"))) int
i2c_read_regs(uint32_t i2c_no, uint32_t addr, uint32_t flags, uint32_t reg, uint32_t dst_buf_addr, uint32_t buf_len);

__attribute__((import_name("i2c_write_regs"))) int
i2c_write_regs(uint32_t i2c_no, uint32_t addr, uint32_t flags, uint32_t reg, uint32_t src_buf_addr, uint32_t buf_len);

__attribute__((import_name("mpu6050_sample"))) int
mpu6050_sample(uint32_t i2c_no, uint32_t addr, uint32_t dst_addr);

__attribute__((import_name("native_sleep"))) int 
native_sleep(int ms);

void print_array(uint8_t * buf, uint32_t len){
    for(uint32_t i=0;i<len;i++){
        printf("%x ",buf[i]);
    }
}

int main(int argc, char **argv)
{
    uint32_t len=2;
    uint8_t * buf = (uint8_t *)malloc(len);
    uint8_t op_power_on=0x01;
    uint8_t op_sigle_hres1=0x20;
    uint8_t tmp;
    uint8_t raw_data[14]={0};
    uint8_t mpu6050_addr=0x68;
    uint8_t mpu6050_config[2];
    
    // bh1750fvi sample
    printf("%x\n",i2c_write_bytes(1,0x23,0,&op_power_on,1));
    printf("%x\n",i2c_write_bytes(1,0x23,0,&op_sigle_hres1,1));
    printf("%x\n",i2c_read_bytes(1,0x23,0,(uint32_t)buf,len));
    print_array(buf,len);
    printf("\n");

    // mpu6050 sample
    uint8_t mpu_id=0x00;
    printf("%x\n",i2c_read_regs(1,mpu6050_addr,0,0x75,&mpu_id,1));
    printf("%x\n",mpu_id);
    mpu6050_config[0]=0x6B;
    mpu6050_config[1]=0x00;
    printf("%x\n",i2c_write_bytes(1,mpu6050_addr,0,&mpu6050_config,2));
    mpu6050_config[0]=0x1C;
    mpu6050_config[1]=0x00;
    printf("%x\n",i2c_write_bytes(1,mpu6050_addr,0,&mpu6050_config,2));
    mpu6050_config[0]=0x1B;
    mpu6050_config[1]=0x00;
    printf("%x\n",i2c_write_bytes(1,mpu6050_addr,0,&mpu6050_config,2));
    mpu6050_config[0]=0x10;
    mpu6050_config[1]=0x07;
    printf("%x\n",i2c_write_bytes(1,mpu6050_addr,0,&mpu6050_config,2));
    native_sleep(100);
    printf("%x\n",mpu6050_sample(1,mpu6050_addr,(uint32_t)raw_data));
    print_array(raw_data,14);
    return 0;
}

