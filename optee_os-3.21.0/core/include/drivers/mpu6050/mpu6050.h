/**
 * @brief mpu6050 driver
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#ifndef _MPU6050_H
#define _MPU6050_H

#include "periph/i2c.h"

typedef struct {
    i2c_t i2c;
    uint8_t addr;
    uint8_t smplrt_div;
    uint8_t config;
    uint8_t gyro_config;
    uint8_t accel_config;
}mpu6050_t;

TEE_Result mpu_init(const mpu6050_t * dev);
TEE_Result mpu_sample(const mpu6050_t * dev, uint8_t data[14]);
TEE_Result mpu_deinit(const mpu6050_t* dev);

#endif