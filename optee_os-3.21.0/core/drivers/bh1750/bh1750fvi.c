/**
 * @brief bh1750fvi driver
 * This source file is based on RIOT bh1750fvi.c
 * https://github.com/RIOT-OS/RIOT/blob/master/drivers/bh1750fvi/bh1750fvi.c
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <drivers/bh1750/bh1750fvi.h>
#include <kernel/delay.h>
#include <trace.h>

/**
 * @brief   Result divisor (1.2 times 65535)
 *
 * The 16-bit RAW results have to be divided by 1.2. We do this by using fixed
 * floating point arithmetic by multiplying divisor and RAW value by 65535 (
 * uint16_t max).
 */
#define RES_DIV                 (78642)

/**
 * @name    Opcodes
 * @{
 */
#define OP_POWER_DOWN           (0x00)
#define OP_POWER_ON             (0x01)
#define OP_RESET                (0x03)
#define OP_CONT_HRES1           (0x10)
#define OP_CONT_HRES2           (0x11)
#define OP_CONT_LRES            (0x13)
#define OP_SINGLE_HRES1         (0x20)
#define OP_SINGLE_HRES2         (0x21)
#define OP_SINGLE_LRES          (0x23)
#define OP_CHANGE_TIME_H_MASK   (0x40)
#define OP_CHANGE_TIME_L_MASK   (0x60)
/** @} */

/**
 * @name    Measurement delays (in us)
 * @{
 */
#define DELAY_HMODE             (120000)    /**< typ. 120ms in H-mode */
#define DELAY_LMODE             (16000)     /**< typ. 16ms in L-mode */
/** @} */

TEE_Result bh1750fvi_init(const bh1750fvi_t *dev)
{
    TEE_Result res;

    DMSG("[bh1750fvi] try acquire i2c %d",dev->i2c);
    res=i2c_acquire(dev->i2c);
    if(res!=TEE_SUCCESS){
        return res;
    }
    // send power down command to make sure we can speak to device
    // i2c_set_frequency(dev->i2c,I2C_SPEED_NORMAL);
    res=i2c_write_byte(dev->i2c,dev->addr,OP_POWER_DOWN,0);
    if(res!=TEE_SUCCESS){
        i2c_release(dev->i2c);
        return TEE_ERROR_ITEM_NOT_FOUND;
    }
    i2c_release(dev->i2c);
    return TEE_SUCCESS;
}

TEE_Result bh1750fvi_sample(const bh1750fvi_t *dev, uint16_t *lux)
{
    TEE_Result res;
    uint32_t tmp;
    uint8_t raw[2]={0};

    /* power on the device and send single H-mode measurement command */
    DMSG("[bh1750fvi] sample: triggering a conversion on i2c %d at %x\n",dev->i2c,dev->addr);
    res=i2c_acquire(dev->i2c);
    if(res!=TEE_SUCCESS){
        return res;
    }
    res=i2c_write_byte(dev->i2c, dev->addr, OP_POWER_ON, 0);
    if(res!=TEE_SUCCESS){
        i2c_release(dev->i2c);
        return res;
    }
    res=i2c_write_byte(dev->i2c, dev->addr, OP_SINGLE_HRES1, 0);
    if(res!=TEE_SUCCESS){
        i2c_release(dev->i2c);
        return res;
    }
    i2c_release(dev->i2c);

    /* wait for measurement to complete */
    udelay(DELAY_HMODE);

    /* read the results */
    DMSG("[bh1750fvi] sample: reading the results\n");
    i2c_acquire(dev->i2c);
    res=i2c_read_bytes(dev->i2c, dev->addr, raw, 2, 0);
    if(res!=TEE_SUCCESS){
        i2c_release(dev->i2c);
        return res;
    }
    i2c_release(dev->i2c);
    DMSG("[bh1750fvi] raw is raw[0] is %x, raw[1] is %x",raw[0],raw[1]);
    /* and finally we calculate the actual LUX value */
    tmp = ((uint32_t)raw[0] << 24) | ((uint32_t)raw[1] << 16);
    tmp /= RES_DIV;
    *lux = (uint16_t)(tmp);
    return TEE_SUCCESS;
}

