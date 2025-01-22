/**
 * @brief pta mpu6050 service
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#ifndef __PTA_MPU6050_H
#define __PTA_MPU6050_H
// d8389f18-99c2-4605-b6dd-7ee5f103e061
#define PTA_MPU6050_SERVICE_UUID    \
    {   \
        0xd8389f18, 0x99c2, 0x4605,  \
        {   \
            0xb6,0xdd,0x7e, 0xe5, 0xf1, 0x03, 0xe0, 0x61  \
        }   \
    }
/**
 * MPU6050 INIT
 * [in] params[0].value.a: i2c number
 * [in] params[0].value.b: i2c addr
 */
#define PTA_CMD_MPU6050_INIT    0

/**
 * MPU6050 DEINIT
 * [in] params[0].value.a: i2c number
 * [in] params[0].value.b: i2c addr
 */
#define PTA_CMD_MPU6050_DEINIT  1

/**
 * MPU6050 SAMPLE
 * [in] params[0].value.a: i2c number
 * [in] params[0].value.b: i2c addr
 * [in/out] params[1].memref.buffer: data
 * [in/out] params[1].memref.size: len, expect 14
 */
#define PTA_CMD_MPU6050_SAMPLE  2

#endif