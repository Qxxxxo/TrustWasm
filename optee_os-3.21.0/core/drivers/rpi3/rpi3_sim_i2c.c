/**
 * @brief gpio simulate i2c
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <platform_config.h>
#include <periph/i2c.h>
#include <periph/gpio.h>
#include <stdint.h>
#include <kernel/mutex.h>
#include <kernel/tee_time.h>
#include <kernel/delay.h>
#include <io.h>
#include <mm/core_memprot.h>
#include <initcall.h>

#define I2C_NUM 2
#define I2C_NORMAL_SPEED_DELAY_US 126
#define I2C_LOW_SPEED_DELAY_US 500
#define I2C_FAST_SPEED_DELAY_US 50
#define I2C_FAST_PLUS_SPEED_DELAY_US 20
#define I2C_HIGH_SPEED_DELAY_US 5

#define I2C_NACK GPIO_HIGH_LEVEL
#define I2C_ACK GPIO_LOW_LEVEL

struct i2c_dev{
    gpio_t sda;
    gpio_t scl;
    uint64_t delay_us;
};

static struct i2c_dev i2c_devs[I2C_NUM];

static struct mutex i2c_locks[I2C_NUM] = {
    MUTEX_INITIALIZER,
    MUTEX_INITIALIZER};

static inline uint16_t htons(uint16_t x)
{
    return ((x & 0x00ff) << 8) | ((x & 0xff00) >> 8);
}

static inline uint64_t teetime_micro(TEE_Time *t)
{
    return t->seconds * 1000000 + t->micros;
}


static inline void i2c_delay(i2c_t dev){
    // udelay(i2c_devs[dev].delay_us);
    for (volatile uint64_t i = 0; i < i2c_devs[dev].delay_us; i++)
    {
        __asm__ __volatile__ ("nop");
    }
}

static inline void i2c_init(i2c_t dev){

    // scl output push pull
    gpio_init(i2c_devs[dev].scl,GPIO_OUT);
    // sda output push pull
    gpio_init(i2c_devs[dev].sda,GPIO_OUT);
    // both high
    gpio_set(i2c_devs[dev].scl,GPIO_HIGH_LEVEL);
    gpio_set(i2c_devs[dev].sda,GPIO_HIGH_LEVEL);
    TEE_Time start,end;
    tee_time_get_sys_time(&start);
    i2c_delay(dev);
    tee_time_get_sys_time(&end);
    DMSG("test delay %d",teetime_micro(&end)-teetime_micro(&start));
}

static inline void i2c_set_sda_output(i2c_t dev){
    gpio_init(i2c_devs[dev].sda,GPIO_OUT); // push pull
}

static inline void i2c_set_sda_input(i2c_t dev){
    gpio_init(i2c_devs[dev].sda,GPIO_IN); // without pull
}

static inline void i2c_set_sda(i2c_t dev, gpio_level_t level){
    i2c_set_sda_output(dev);
    gpio_set(i2c_devs[dev].sda,level);
}

static inline int i2c_read_sda(i2c_t dev){
    i2c_set_sda_input(dev);
    return gpio_get(i2c_devs[dev].sda);
}

static inline void i2c_set_scl(i2c_t dev, gpio_level_t level){
    gpio_set(i2c_devs[dev].scl,level);
}

static inline void i2c_start(i2c_t dev){
    i2c_set_sda(dev,GPIO_HIGH_LEVEL);
    i2c_delay(dev);
    i2c_set_scl(dev,GPIO_HIGH_LEVEL);
    i2c_delay(dev);
    i2c_set_sda(dev,GPIO_LOW_LEVEL);
    i2c_delay(dev);
    i2c_set_sda(dev,GPIO_LOW_LEVEL);
    i2c_delay(dev);
}

static inline void i2c_stop(i2c_t dev){
    i2c_set_sda(dev,GPIO_LOW_LEVEL);
    i2c_delay(dev);
    i2c_set_scl(dev,GPIO_HIGH_LEVEL);
    i2c_delay(dev);
    i2c_set_sda(dev,GPIO_HIGH_LEVEL);
    i2c_delay(dev);
}

static TEE_Result _i2c_write_byte(i2c_t dev,uint8_t byte){
    int ack;
    i2c_set_sda_output(dev);
    for(int8_t i=7;i>=0;i--){
        gpio_level_t level=(byte & (1<<i))?GPIO_HIGH_LEVEL:GPIO_LOW_LEVEL;
        i2c_set_sda(dev,level);
        i2c_delay(dev);
        i2c_set_scl(dev,GPIO_HIGH_LEVEL);
        i2c_delay(dev);
        i2c_set_scl(dev,GPIO_LOW_LEVEL);
        i2c_delay(dev);
    }
    // receive ack
    i2c_set_sda_input(dev);
    i2c_delay(dev);
    i2c_set_scl(dev,GPIO_HIGH_LEVEL);
    i2c_delay(dev);
    ack=i2c_read_sda(dev);
    i2c_set_scl(dev,GPIO_LOW_LEVEL);
    i2c_delay(dev);
    DMSG("ack %d",ack);
    return (ack>0)?TEE_ERROR_TIMEOUT:TEE_SUCCESS;
}

static uint8_t _i2c_read_byte(i2c_t dev, gpio_level_t ack){
    uint8_t byte=0;
    i2c_set_sda_input(dev);
    i2c_delay(dev);
    for(int8_t i=7;i>=0;i--){
        i2c_set_scl(dev,GPIO_HIGH_LEVEL);
        i2c_delay(dev);
        if(i2c_read_sda(dev)==GPIO_HIGH_LEVEL){
            byte |= (1<<i);
        }
        i2c_set_scl(dev,GPIO_LOW_LEVEL);
        i2c_delay(dev);
    }
    // send ack
    i2c_set_sda_output(dev);
    i2c_set_sda(dev,ack);
    i2c_delay(dev);
    i2c_set_scl(dev,GPIO_HIGH_LEVEL);
    i2c_delay(dev);
    i2c_set_scl(dev,GPIO_LOW_LEVEL);
    i2c_delay(dev);
    return byte;
} 



TEE_Result i2c_acquire(i2c_t dev)
{
    if (__builtin_expect(dev >= I2C_NUM, 0))
        return TEE_ERROR_BAD_PARAMETERS;
    mutex_lock(&i2c_locks[dev]);
    return TEE_SUCCESS;
}

TEE_Result i2c_release(i2c_t dev)
{
    if (__builtin_expect(dev > I2C_NUM, 0))
        return TEE_ERROR_BAD_PARAMETERS;
    mutex_unlock(&i2c_locks[dev]);
    return TEE_SUCCESS;
}

TEE_Result i2c_read_byte(i2c_t dev, uint16_t addr, void *data, uint8_t flags)
{
    return i2c_read_bytes(dev, addr, data, 1, flags);
}

TEE_Result i2c_read_bytes(i2c_t dev, uint16_t addr,
                          void *data, uint16_t len, uint8_t flags)
{
    if (__builtin_expect(dev >= I2C_NUM || len == 0 || data == NULL, 0))
        return TEE_ERROR_BAD_PARAMETERS;
    i2c_init(dev);
    i2c_start(dev);
    if(_i2c_write_byte(dev,((uint8_t)addr)<<1)!=TEE_SUCCESS){
        i2c_stop(dev);
        return TEE_ERROR_ITEM_NOT_FOUND;
    }
    i2c_start(dev);
    if(_i2c_write_byte(dev,((uint8_t)addr)<<1|1)!=TEE_SUCCESS){
        i2c_stop(dev);
        return TEE_ERROR_ITEM_NOT_FOUND;
    }
    for(uint16_t i=0;i<len;i++){
        ((uint8_t*)data)[i]=_i2c_read_byte(dev,i==len-1?GPIO_HIGH_LEVEL:GPIO_LOW_LEVEL);
    }
    i2c_stop(dev);
    return TEE_SUCCESS;
}

TEE_Result i2c_write_byte(i2c_t dev, uint16_t addr, uint8_t data, uint8_t flags)
{
    DMSG("[i2c] write byte call write bytes");
    return i2c_write_bytes(dev, addr, &data, 1, flags);
}

TEE_Result i2c_write_bytes(i2c_t dev, uint16_t addr, const void *data,
                           uint16_t len, uint8_t flags)
{
    if (__builtin_expect(dev > I2C_NUM || len == 0 || data == NULL, 0))
        return TEE_ERROR_BAD_PARAMETERS;
    if(flags&I2C_ADDR10) return TEE_ERROR_NOT_IMPLEMENTED;

    i2c_init(dev);
    i2c_start(dev);

    if(_i2c_write_byte(dev,((uint8_t)addr)<<1)!=TEE_SUCCESS){
        DMSG("write addr error");
        i2c_stop(dev);
        return TEE_ERROR_ITEM_NOT_FOUND;
    }
    for(uint16_t i=0;i<len;i++){
        if(_i2c_write_byte(dev,((uint8_t *)data)[i])!=TEE_SUCCESS){
            i2c_stop(dev);
            return TEE_ERROR_TIMEOUT;
        }
    }
    i2c_stop(dev);
    return TEE_SUCCESS;
}

TEE_Result i2c_read_reg(i2c_t dev, uint16_t addr, uint16_t reg,
                        void *data, uint8_t flags)
{
    return i2c_read_regs(dev, addr, reg, data, 1, flags);
}

TEE_Result i2c_read_regs(i2c_t dev, uint16_t addr, uint16_t reg,
                         void *data, uint16_t len, uint8_t flags)
{
    TEE_Result res;
    // htons
    reg = htons(reg);
    // write reg addr to i2c dev
    res = i2c_write_bytes(dev, addr,
                          (flags & I2C_REG16) ? ((uint8_t *)&reg) : ((uint8_t *)&reg) + 1,
                          (flags & I2C_REG16) ? 2 : 1,
                          flags);
    if (res != TEE_SUCCESS)
        return res;

    res = i2c_read_bytes(dev, addr, data, len, flags);
    return res;
}

TEE_Result i2c_write_reg(i2c_t dev, uint16_t addr, uint16_t reg,
                         uint8_t data, uint8_t flags)
{
    return i2c_write_regs(dev, addr, reg, &data, 1, flags);
}

TEE_Result i2c_write_regs(i2c_t dev, uint16_t addr, uint16_t reg,
                          const void *data, uint16_t len, uint8_t flags)
{
    TEE_Result res;
    reg = htons(reg);
    res = i2c_write_bytes(dev, addr,
                          (flags & I2C_REG16) ? ((uint8_t *)&reg) : ((uint8_t *)&reg) + 1,
                          (flags & I2C_REG16) ? 2 : 1,
                          flags);
    if (res != TEE_SUCCESS)
        return res;
    return i2c_write_bytes(dev, addr, data, len, flags);
}

TEE_Result i2c_set_frequency(i2c_t dev, i2c_speed_t freq)
{
    return TEE_ERROR_NOT_SUPPORTED;
}

static TEE_Result rpi3_sim_i2c_init(void)
{
    i2c_devs[0].sda=0;
    i2c_devs[0].scl=1;
    i2c_devs[0].delay_us=I2C_NORMAL_SPEED_DELAY_US;
    i2c_devs[1].sda=2;
    i2c_devs[1].scl=3;
    i2c_devs[1].delay_us=I2C_NORMAL_SPEED_DELAY_US;
    return TEE_SUCCESS;
}

driver_init(rpi3_sim_i2c_init);