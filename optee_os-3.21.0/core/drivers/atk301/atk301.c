/**
 * @brief atk301 driver
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */

#include "periph/i2c.h"
#include "drivers/atk301/atk301.h"
#include "kernel/delay.h"
#include <trace.h>

#define ATK301_I2C_CMD_HANDSHAKE 0x01
#define ATK301_I2C_CMD_GET_FP_IMAGE 0x02
#define ATK301_I2C_CMD_UPLOAD_FP_IMAGE 0x03
#define ATK301_I2C_RETURN_SUCCESS 0x01
#define ATK301_I2C_RETURN_SUCCESS_WITH_DATA 0x02
#define ATK301_I2C_RETURN_FAILED 0x03

#define UPLOAD_FP_IMAGE_BLOCK_SIZE 160

#define HANDSHAKE_WAIT_TIME_US 1100000 //1050ms
#define GET_FP_IMAGE_WAIT_TIME_US 1100000 //1050ms
#define UPLOAD_FP_IMAGE_WAIT_TIME_US 3500000 //3400ms

TEE_Result atk301_init(const atk301_t *dev)
{
    TEE_Result res;
    uint8_t head[3];
    res=i2c_acquire(dev->i2c);
    if(res!=TEE_SUCCESS){
        return res;
    }
    res = i2c_write_byte(dev->i2c, dev->addr, ATK301_I2C_CMD_HANDSHAKE, 0);
    if (res != TEE_SUCCESS)
    {
        i2c_release(dev->i2c);
        return res;
    }
    i2c_release(dev->i2c);
    
    udelay(HANDSHAKE_WAIT_TIME_US);

    i2c_acquire(dev->i2c);
    i2c_read_bytes(dev->i2c, dev->addr, head, 1, 0); // trigger write res
    res = i2c_read_bytes(dev->i2c, dev->addr, head, 3, 0);
    i2c_release(dev->i2c);
    if (res != TEE_SUCCESS)
    {
        return res;
    }
    if (head[0] != ATK301_I2C_RETURN_SUCCESS)
    {
        res=TEE_ERROR_ITEM_NOT_FOUND; // handshake failed
    }
    // handshake success
    return res;
}

TEE_Result atk301_get_fingerprint_image(const atk301_t * dev){
    TEE_Result res;
    uint8_t head[3];
    res=i2c_acquire(dev->i2c);
    if(res!=TEE_SUCCESS){
        return res;
    }
    res = i2c_write_byte(dev->i2c, dev->addr, ATK301_I2C_CMD_GET_FP_IMAGE, 0);
    if (res != TEE_SUCCESS)
    {
        i2c_release(dev->i2c);
        return res;
    }
    i2c_release(dev->i2c);
    udelay(GET_FP_IMAGE_WAIT_TIME_US);

    i2c_acquire(dev->i2c);
    i2c_read_bytes(dev->i2c, dev->addr, head, 1, 0); // trigger write res
    res = i2c_read_bytes(dev->i2c, dev->addr, head, 3, 0);
    i2c_release(dev->i2c);
    if (res != TEE_SUCCESS)
    {
        return res;
    }
    if (head[0] != ATK301_I2C_RETURN_SUCCESS)
    {
        res=TEE_ERROR_ITEM_NOT_FOUND; // get fingerprint image failed
    }
    // get fingerprint image success
    return res;
}

TEE_Result atk301_upload_fingerprint_image(const atk301_t * dev, uint8_t data[ATK301_FINGERPRINT_IMAGE_DATA_LEN]){
    TEE_Result res;
    uint8_t head[3];
    uint16_t len=0;
    res=i2c_acquire(dev->i2c);
    if(res!=TEE_SUCCESS){
        return res;
    }
    res = i2c_write_byte(dev->i2c, dev->addr, ATK301_I2C_CMD_UPLOAD_FP_IMAGE, 0);
    if (res != TEE_SUCCESS)
    {
        i2c_release(dev->i2c);
        return res;
    }
    i2c_release(dev->i2c);
    udelay(UPLOAD_FP_IMAGE_WAIT_TIME_US);

    i2c_acquire(dev->i2c);
    i2c_read_bytes(dev->i2c, dev->addr, head, 1, 0); // trigger write res
    res = i2c_read_bytes(dev->i2c, dev->addr, head, 3, 0);
    i2c_release(dev->i2c);
    if (res != TEE_SUCCESS)
    {
        return res;
    }
    if (head[0] != ATK301_I2C_RETURN_SUCCESS_WITH_DATA)
    {
        res=TEE_ERROR_NO_DATA;
        return res;
    }
    len |= (((uint16_t)head[1])<<8);
    len |= (uint16_t)head[2];
    // verify len
    if(len!=ATK301_FINGERPRINT_IMAGE_DATA_LEN){
        DMSG("len not match");
        return TEE_ERROR_NO_DATA;
    }
    // data ready
    // request for fp image data
    i2c_acquire(dev->i2c);
    for(int i=0;i<ATK301_FINGERPRINT_IMAGE_DATA_LEN/UPLOAD_FP_IMAGE_BLOCK_SIZE;i++){
        res = i2c_read_bytes(dev->i2c, dev->addr, &(data[i*UPLOAD_FP_IMAGE_BLOCK_SIZE]),UPLOAD_FP_IMAGE_BLOCK_SIZE, 0);
        if(res!=TEE_SUCCESS){
            i2c_release(dev->i2c);
            return res;
        }
    }
    i2c_release(dev->i2c);
    return res;
}