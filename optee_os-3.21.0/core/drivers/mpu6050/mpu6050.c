/**
 * @brief mpu6050 driver
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <drivers/mpu6050/mpu6050.h>
#include <periph/i2c.h>
#include <trace.h>
#include <kernel/delay.h>

#define MPU6050_I2C_ADDR (0x68)

#define PWR_MGMT_1 (0x6B)
#define PWR_MGMT_2 (0x6C)
#define PWR_MGMT_1_CMD_ON   (0x00)
#define PWR_MGMT_1_CMD_X   (0x01)
#define PWR_MGMT_1_CMD_OFF  (0x40)
#define PWR_MGMT_1_CMD_RESET  (0x80)


#define SMPLRT_DIV   (0x19)
#define CONFIG       (0x1A)
#define GYRO_CONFIG  (0x1B)
#define ACCEL_CONFIG (0x1C)
#define FIFO_EN_REG  (0x23)
#define INTBP_CFG_REG (0x37)
#define INT_EN_REG	 (0x38)
#define ACCEL_XOUT_H (0x3B)
#define ACCEL_XOUT_L (0x3C)
#define ACCEL_YOUT_H (0x3D)
#define ACCEL_YOUT_L (0x3E)
#define ACCEL_ZOUT_H (0x3F)
#define ACCEL_ZOUT_L (0x40)
#define TEMP_OUT_H (0x41)
#define TEMP_OUT_L (0x42)
#define GYRO_XOUT_H (0x43)
#define GYRO_XOUT_L (0x44)
#define GYRO_YOUT_H (0x45)
#define GYRO_YOUT_L (0x46)
#define GYRO_ZOUT_H (0x47)
#define GYRO_ZOUT_L (0x48)
#define SIGNAL_PATH_RESET   (0x68)
#define USER_CTRL_REG		(0x6A)
#define DEVICE_ID_REG		(0x75)

#define RESET_DELAY_US  100000

TEE_Result mpu_init(const mpu6050_t* dev){
    TEE_Result res;
    uint8_t id;
    res=i2c_acquire(dev->i2c);
    if(res!=TEE_SUCCESS) {
        return res;
    };
    // reset
    res=i2c_write_reg(dev->i2c,dev->addr,PWR_MGMT_1,PWR_MGMT_1_CMD_RESET,0);
    if(res!=TEE_SUCCESS) {
        goto init_release_exit;
    };
    udelay(RESET_DELAY_US);
    res=i2c_write_reg(dev->i2c,dev->addr,SIGNAL_PATH_RESET,0x07,0);
    if(res!=TEE_SUCCESS) {
        goto init_release_exit;
    };
    udelay(RESET_DELAY_US);
    // sample rate to 1kHz, may support config later
    res=i2c_write_reg(dev->i2c,dev->addr,SMPLRT_DIV,0x00,0);
    if(res!=TEE_SUCCESS){
        DMSG("Failed to set sample rate");
        goto init_release_exit;
    }
    // set dlpf bandwidth to 44Hz, may support config later
    res=i2c_write_reg(dev->i2c,dev->addr,CONFIG,0x07,0);
    if(res!=TEE_SUCCESS){
        DMSG("Failed to set dlpf bandwidth");
        goto init_release_exit;
    }
    // set gyro full scale range to ±250°/s, may support config later
    res=i2c_write_reg(dev->i2c,dev->addr,GYRO_CONFIG,0x00,0);
    if(res!=TEE_SUCCESS){
        DMSG("Faield to set gyro full scale range");
        goto init_release_exit;
    }
    // set accel full scale range to ±2g, may support config later
    res=i2c_write_reg(dev->i2c,dev->addr,ACCEL_CONFIG,0x00,0);
    if(res!=TEE_SUCCESS){
        DMSG("Faield to set accel full scale range");
        goto init_release_exit;
    }
    
    // read device id
    res=i2c_read_reg(dev->i2c,dev->addr,DEVICE_ID_REG,&id,0);
    if(res!=TEE_SUCCESS){
        DMSG("Faield to device id");
        goto init_release_exit;
    }
    DMSG("mpu6050 get id %x",id);
    if(id!=MPU6050_I2C_ADDR){
        i2c_release(dev->i2c);
        return TEE_ERROR_ITEM_NOT_FOUND;
    }

    res=i2c_write_reg(dev->i2c,dev->addr,PWR_MGMT_1,PWR_MGMT_1_CMD_X,0);
    if(res!=TEE_SUCCESS) {
        DMSG("Failed to set clksel");
        goto init_release_exit;
    };
    udelay(RESET_DELAY_US);
    
    DMSG("mpu6050 init success");
init_release_exit:
    i2c_release(dev->i2c);
    return res;
}

static int print_mpu6050_data(uint8_t data[14]){
    int16_t ax=(data[0]<<8)|data[1];
    int16_t ay=(data[2]<<8)|data[3];
    int16_t az=(data[4]<<8)|data[5];
    int16_t raw_temp = (data[6]<<8)|data[7];
    int16_t gx=(data[8]<<8)|data[9];
    int16_t gy=(data[10]<<8)|data[11];
    int16_t gz=(data[12]<<8)|data[13];
    
    // float temp=(float)raw_temp/340.0 + 36.53;
    DMSG("sample data ax: %d, ay: %d, az: %d, gx: %d, gy: %d, gz: %d, raw_temp: %d",
            ax,ay,az,gx,gy,gz,raw_temp);
    return 0;
}

// a[6] g[6] t[2]
TEE_Result mpu_sample(const mpu6050_t * dev, uint8_t data[14]){
    TEE_Result res;
    res=i2c_acquire(dev->i2c);
    if(res!=TEE_SUCCESS){
        return res;
    }
    for(int i=0;i<14;i++){
        res = i2c_read_reg(dev->i2c,dev->addr,ACCEL_XOUT_H+i,&(data[i]),0);
        if (res != TEE_SUCCESS){
            goto sample_release_exit;
        }
    }
    // // Convert raw data to int16_t
    // for (int i = 0; i < 3; i++) {
    //     a[i] = (data[i*2] << 8) | data[i*2 + 1];
    //     g[i] = (data[i*2 + 8] << 8) | data[i*2 + 9];
    // }
    // // raw temp
    // t[0] = data[6];
    // t[1] = data[7];
    // DMSG("mpu6050 sample success %d",print_mpu6050_data(data));
sample_release_exit:
    i2c_release(dev->i2c);
    return res;
}

TEE_Result mpu_deinit(const mpu6050_t* dev){
    TEE_Result res;
    res=i2c_acquire(dev->i2c);
    if(res!=TEE_SUCCESS){
        return res;
    }
    res=i2c_write_reg(dev->i2c,dev->addr,PWR_MGMT_1,PWR_MGMT_1_CMD_OFF,0);
    if(res!=TEE_SUCCESS){
        DMSG("Failded to power off mpu6050");
    }
    i2c_release(dev->i2c);
    return res;
}
