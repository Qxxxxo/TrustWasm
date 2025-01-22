/**
 * @brief pta i2c service
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#ifndef __PTA_I2C_H
#define __PTA_I2C_H
// cf96209a-2890-41d3-aaab-e5437bd54f13
#define PTA_I2C_SERVICE_UUID                               \
    {                                                      \
        0xcf96209a, 0x2890, 0x41d3,                        \
        {                                                  \
            0xaa, 0xab, 0xe5, 0x43, 0x7b, 0xd5, 0x4f, 0x13 \
        }                                                  \
    }

/**
 * I2C read byte
 * [in]     params[0].value.a: i2c number
 * [in]     params[0].value.b: i2c device address
 * [in]     params[1].value.a: i2c flags
 * [out]    params[0].value.a: read byte
 */
#define PTA_I2C_READ_BYTE 0

/**
 * I2C read bytes
 * [in]     params[0].value.a: i2c number
 * [in]     params[0].value.b: i2c device address
 * [in]     params[1].value.a: i2c flags
 * [in/out] params[2].memref.buf
 * [in/out] params[2].memref.len
 */
#define PTA_I2C_READ_BYTES 1

/**
 * I2C write byte
 * [in]     params[0].value.a: i2c number
 * [in]     params[0].value.b: i2c device address
 * [in]     params[1].value.a: i2c flags
 * [in]     params[1].value.b: write byte
 */
#define PTA_I2C_WRITE_BYTE 2

/**
 * I2C write bytes
 * [in]     params[0].value.a: i2c number
 * [in]     params[0].value.b: i2c device address
 * [in]     params[1].value.a: i2c flags
 * [in]     params[2].memref.buf
 * [in]     params[2].memref.len
 */
#define PTA_I2C_WRITE_BYTES 3

/**
 * I2C read reg
 * [in]     params[0].value.a: i2c number
 * [in]     params[0].value.b: i2c device address
 * [in]     params[1].value.a: i2c flags
 * [in]     params[1].value.b: reg
 * [out]    params[0].value.a: read byte
 */
#define PTA_I2C_READ_REG 4

/**
 * I2C read regs
 * [in]     params[0].value.a: i2c number
 * [in]     params[0].value.b: i2c device address
 * [in]     params[1].value.a: i2c flags
 * [in]     params[1].value.b: reg
 * [in/out] params[2].memref.buf
 * [in/out] params[2].memref.len
 */
#define PTA_I2C_READ_REGS 5

/**
 * I2C write reg
 * [in]     params[0].value.a: i2c number
 * [in]     params[0].value.b: i2c device address
 * [in]     params[1].value.a: i2c flags
 * [in]     params[1].value.b: reg
 * [in]     params[2].value.a: write byte
 */
#define PTA_I2C_WRITE_REG 6

/**
 * I2C write regs
 * [in]     params[0].value.a: i2c number
 * [in]     params[0].value.b: i2c device address
 * [in]     params[1].value.a: i2c flags
 * [in]     params[1].value.b: reg
 * [in]     params[2].memref.buf
 * [in]     params[2].memref.len
 */
#define PTA_I2C_WRITE_REGS 7

/**
 * I2C set frequency
 * [in]     params[0].value.a:  i2c number
 * [in]     params[0].value.b:  i2c speed option
 */
#define PTA_I2C_SET_FREQ 8


#define PTA_I2C_GET_OVERHEAD_TIMESTAMP  9

#endif