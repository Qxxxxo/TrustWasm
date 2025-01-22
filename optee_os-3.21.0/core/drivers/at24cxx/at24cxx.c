/**
 * @brief at24cxx driver
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */

#include <drivers/at24cxx/at24cxx.h>

TEE_Result at24cxx_write(const at24cxx_t *dev, uint16_t addr, const uint8_t *buf, uint32_t len)
{
    int res;
    uint8_t addr_buf[2];
    addr_buf[0]=(uint8_t)(addr>>8);
    addr_buf[1]=(uint8_t)(addr&0xFF);
    // DMSG("write to %04x",addr);
    i2c_acquire(dev->dev);
    res = i2c_write_with_prefix(dev->dev, dev->addr, addr_buf, 2, buf, len, 0);
    i2c_release(dev->dev);
    return res;
}

TEE_Result at24cxx_read(const at24cxx_t *dev, uint16_t addr, uint8_t *buf, uint32_t len)
{
    int res;
    uint8_t addr_buf[2];
    addr_buf[0]=(uint8_t)(addr>>8);
    addr_buf[1]=(uint8_t)(addr&0xFF);
    i2c_acquire(dev->dev);
    res = i2c_write_bytes(dev->dev, dev->addr, addr_buf, 2, 0);
    if(res!=TEE_SUCCESS){
        i2c_release(dev->dev);
        return res;
    }
    res = i2c_read_bytes(dev->dev, dev->addr, buf, len,I2C_CONT);
    i2c_release(dev->dev);
    return res;
}