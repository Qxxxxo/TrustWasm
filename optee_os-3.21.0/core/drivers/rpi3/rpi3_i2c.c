/**
 * @brief driver for i2c
 * reference to linux driver i2c-bcm2835.c
 * and http://www.airspayce.com/mikem/bcm2835/
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <platform_config.h>
#include <periph/i2c.h>
#include <drivers/rpi3/rpi3_gpio.h>
#include <stdint.h>
#include <kernel/mutex.h>
#include <kernel/tee_time.h>
#include <io.h>
#include <mm/core_memprot.h>
#include <initcall.h>
#include <kernel/delay.h>

#define I2C_NUM 2
#define I2C_START_TIMEOUT_US 1000
#define I2C_READ_TIMEOUT_US_PER_BYTE 1000
#define I2C_WRITE_TIMEOUT_US_PER_BYTE 1000

// #define BSC_C 0x00    // Control
// #define BSC_S 0x04    // Status
// #define BSC_DLEN 0x08 // Data Length
// #define BSC_A 0x0C    // Slave Address
// #define BSC_FIFO 0x10 // Data FIFO
// #define BSC_DIV 0x14  // Clock Divider
// #define BSC_DEL 0x18  // Data Delay
// #define BSC_CLKT 0x1C // Clock Stretch Timeout

// #define BSC_SIZE 0x20 // bsc size

struct BSCRegister
{
    uint32_t BSC_C;    // Control
    uint32_t BSC_S;    // Status
    uint32_t BSC_DLEN; // Data Length
    uint32_t BSC_A;    // Slave Address
    uint32_t BSC_FIFO; // Data FIFO
    uint32_t BSC_DIV;  // Clock Divider
    uint32_t BSC_DEL;  // Data Delay
    uint32_t BSC_CLKT; // Clock Stretch Timeout
};

// BSC Control Flags
#define BSC_C_I2CEN BIT(15) // I2C Enable
#define BSC_C_INTR BIT(10)  // Interrupt on RX
#define BSC_C_INTX BIT(9)   // Interrupt on TX
#define BSC_C_INTD BIT(8)   // Interrupt on DONE
#define BSC_C_ST BIT(7)     // Start Transfer
#define BSC_C_CLEAR BIT(4)  // Clear FIFO, bits 4 & 5 both clear
#define BSC_C_READ BIT(0)   // Read Transfer

// BSC Status Flags
#define BSC_S_CLKT BIT(9)
#define BSC_S_ERR BIT(8)
#define BSC_S_RXF BIT(7)
#define BSC_S_TXE BIT(6)
#define BSC_S_RXD BIT(5)
#define BSC_S_TXD BIT(4)
#define BSC_S_RXR BIT(3)
#define BSC_S_TXW BIT(2)
#define BSC_S_DONE BIT(1)
#define BSC_S_TA BIT(0)

#define BSC_DEL_FEDL_SHIFT 16
#define BSC_DEL_REDL_SHIFT 0

#define BSC_CDIV_MIN 0x0002
#define BSC_CDIV_MAX 0xFFFE // 65534


#define BSC_FIFO_SIZE   16

static struct mutex bsc_locks[I2C_NUM] = {
    MUTEX_INITIALIZER,
    MUTEX_INITIALIZER};

static struct BSCRegister * bsc_registers[I2C_NUM] = {
    (struct BSCRegister *)BSC0_BASE,
    (struct BSCRegister *)BSC1_BASE};

static uint64_t i2c_byte_wait_us[I2C_NUM]={
    0,
    0
};

static inline uint16_t htons(uint16_t x)
{
    return ((x & 0x00ff) << 8) | ((x & 0xff00) >> 8);
}

static inline uint64_t teetime_micro(TEE_Time *t)
{
    return t->seconds * 1000000 + t->micros;
}

static inline vaddr_t BSC_C_(i2c_t dev)
{
    return (vaddr_t)&(bsc_registers[dev]->BSC_C);
}

static inline vaddr_t BSC_S_(i2c_t dev)
{
    return (vaddr_t)&(bsc_registers[dev]->BSC_S);
}

static inline vaddr_t BSC_DLEN_(i2c_t dev)
{
    return (vaddr_t)&(bsc_registers[dev]->BSC_DLEN);
}

static inline vaddr_t BSC_A_(i2c_t dev)
{
    return (vaddr_t)&(bsc_registers[dev]->BSC_A);
}

static inline vaddr_t BSC_FIFO_(i2c_t dev)
{
    return (vaddr_t)&(bsc_registers[dev]->BSC_FIFO);
}

static inline vaddr_t BSC_DIV_(i2c_t dev)
{
    return (vaddr_t)&(bsc_registers[dev]->BSC_DIV);
}

static inline vaddr_t BSC_DEL_(i2c_t dev)
{
    return (vaddr_t)&(bsc_registers[dev]->BSC_DEL);
}

#define max(a, b) ((a) > (b) ? (a) : (b))

static inline void show_bsc_debug_msg(uint16_t remain, uint16_t status){
    DMSG("remain=%u, status=0x%x: %s%s%s%s%s%s%s%s%s%s",
        remain,status,
        status & BSC_S_TA ? "TA ":"",
        status & BSC_S_DONE ? "DONE ":"",
        status & BSC_S_TXW ? "TXW " : "",
		status & BSC_S_RXR ? "RXR " : "",
		status & BSC_S_TXD ? "TXD " : "",
		status & BSC_S_RXD ? "RXD " : "",
		status & BSC_S_TXE ? "TXE " : "",
		status & BSC_S_RXF ? "RXF " : "",
		status & BSC_S_ERR ? "ERR " : "",
		status & BSC_S_CLKT ? "CLKT " : "");
}

/**
 * @brief set bsc divider
 */
static TEE_Result i2c_set_divider(i2c_t dev, uint32_t bus_clk_rate)
{
    uint32_t divider, redl, fedl;
    divider = DIV_ROUND_UP(CORE_CLK, bus_clk_rate);
    /*
     * Per the datasheet, the register is always interpreted as an even
     * number, by rounding down. In other words, the LSB is ignored. So,
     * if the LSB is set, increment the divider to avoid any issue.
     */
    if (divider & 1)
        divider++;
    if ((divider < BSC_CDIV_MIN) ||
        (divider > BSC_CDIV_MAX))
    {
        EMSG("Invalid clock frequency\n");
        return TEE_ERROR_BAD_PARAMETERS;
    }
    io_write16(BSC_DIV_(dev), divider);

    /*
     * Number of core clocks to wait after falling edge before
     * outputting the next data bit.  Note that both FEDL and REDL
     * can't be greater than CDIV/2.
     */
    fedl = max(divider / 16, 1u);

    /*
     * Number of core clocks to wait after rising edge before
     * sampling the next incoming data bit.
     */
    redl = max(divider / 4, 1u);
    io_write16(BSC_DEL_(dev), (fedl << BSC_DEL_FEDL_SHIFT) | (redl << BSC_DEL_REDL_SHIFT));
    io_write32(BSC_DIV_(dev), (fedl << BSC_DEL_FEDL_SHIFT) | (redl << BSC_DEL_REDL_SHIFT));

    return TEE_SUCCESS;
}

// static void bsc_fill_txfifo(i2c_t dev, const void * data, uint16_t* sent, uint16_t * remain){
//     uint16_t status;
//     while(*remain){
//         status = io_read16(BSC_S_(dev));
//         if(!(status & BSC_S_TXD))
//             break;
//         io_write8(BSC_FIFO_(dev),((uint8_t *)data)[*sent]);
//         (*sent)++;
//         (*remain)--;
//     }
// }

// static void bsc_drain_rxfifo(i2c_t dev, void * data, uint16_t* received, uint16_t * remain){
//     uint16_t status;
//     while(*remain){
//         status = io_read16(BSC_S_(dev));
//         if(!(status & BSC_S_RXD))
//             break;
//         ((uint8_t*)data)[*received]=io_read8(BSC_FIFO_(dev));
//         (*received)++;
//         (*remain)--;
//     }
// }

TEE_Result i2c_acquire(i2c_t dev)
{
    uint16_t cdiv;
    if (__builtin_expect(dev >= I2C_NUM, 0))
        return TEE_ERROR_BAD_PARAMETERS;
    mutex_lock(&bsc_locks[dev]);
    // i2c begin
    // set gpio fsel
    if(dev==1){
        gpio_fsel((gpio_t)2, GPIO_FSEL_ALT0); // SDA
        gpio_fsel((gpio_t)3, GPIO_FSEL_ALT0); // SCL
    }else{
        gpio_fsel((gpio_t)0, GPIO_FSEL_ALT0); // SDA
        gpio_fsel((gpio_t)1, GPIO_FSEL_ALT0); // SCL
    }
    cdiv=io_read16(BSC_DIV_(dev));
    /* Calculate time for transmitting one byte
    // 1000000 = micros seconds in a second
    // 9 = Clocks per byte : 8 bits + ACK
    */
    i2c_byte_wait_us[dev] = (uint64_t)((uint64_t)cdiv * 1000000 * 9) / CORE_CLK;
    return TEE_SUCCESS;
}

TEE_Result i2c_release(i2c_t dev)
{
    if (__builtin_expect(dev >= I2C_NUM, 0))
        return TEE_ERROR_BAD_PARAMETERS;
    if(dev==1){
        gpio_fsel((gpio_t)2, GPIO_FSEL_INPT); // SDA
        gpio_fsel((gpio_t)3, GPIO_FSEL_INPT); // SCL
    }else{
        gpio_fsel((gpio_t)0, GPIO_FSEL_INPT); // SDA
        gpio_fsel((gpio_t)1, GPIO_FSEL_INPT); // SCL
    }
    mutex_unlock(&bsc_locks[dev]);
    return TEE_SUCCESS;
}

TEE_Result i2c_read_byte(i2c_t dev, uint16_t addr, void *data, uint8_t flags)
{
    return i2c_read_bytes(dev, addr, data, 1, flags);
}

TEE_Result i2c_read_bytes(i2c_t dev, uint16_t addr,
                          void *data, uint16_t len, uint8_t flags)
{
    TEE_Result res = TEE_SUCCESS;
    uint32_t control= BSC_C_ST | BSC_C_I2CEN;
    uint16_t transfer_active = 0;
    uint64_t timeout=I2C_START_TIMEOUT_US;
    uint16_t remain = len;
    uint16_t received = 0;
    TEE_Time start, end;

    if (__builtin_expect(dev >= I2C_NUM || len == 0 || data == NULL, 0))
        return TEE_ERROR_BAD_PARAMETERS;

    if(!(flags&I2C_CONT)){
        io_setbits32(BSC_C_(dev),BSC_C_CLEAR); // clear fifo
        io_write32(BSC_S_(dev),BSC_S_CLKT | BSC_S_ERR | BSC_S_DONE); // clear status
    }
    

    if (flags & I2C_ADDR10)
    {
        // 10 bits device address
        // set data len one
        io_write16(BSC_DLEN_(dev), 1);
        // send slave address he least 8 bits
        io_write8(BSC_FIFO_(dev), (uint8_t)(addr & 0xFF));
        // write slave address reg the most 2 bits
        // 11110xx ,xx is most 2 bits
        io_write8(BSC_A_(dev), (uint8_t)((addr >> 8) & 0x03) | 0x78);
        
        // first write
        io_write32(BSC_C_(dev), control);

        // poll I2CS.TA, wait transfer start
        tee_time_get_sys_time(&start);
        tee_time_get_sys_time(&end);
        do
        {
            transfer_active = io_read16(BSC_S_(dev)) & BSC_S_TA;
            tee_time_get_sys_time(&end); // update end time
        } while (!transfer_active && teetime_micro(&end) - teetime_micro(&start) <= timeout);

        if (teetime_micro(&end) - teetime_micro(&start) > timeout)
        {
            DMSG("addr send timeout");
            res=TEE_ERROR_TIMEOUT;
            io_write32(BSC_S_(dev),BSC_S_CLKT | BSC_S_ERR | BSC_S_DONE);
            return res;
        }
        if(transfer_active){
            // write len
            io_write16(BSC_DLEN_(dev), len);
        }else{
            DMSG("no slave ack");
            res=TEE_ERROR_TIMEOUT;
            io_write32(BSC_S_(dev),BSC_S_CLKT | BSC_S_ERR | BSC_S_DONE);
            return res;
        }
    }
    else
    {
        // 7 bits device address
        // set data len zero
        io_write32(BSC_DLEN_(dev), len);
        io_write32(BSC_A_(dev), (uint8_t)addr);
    }

    // read start
    io_write32(BSC_C_(dev), control | BSC_C_READ);

    // wait for transfer to complete
    while(!(io_read16(BSC_S_(dev)) & BSC_S_DONE)){
        // we must empty the fifo as it is populated and not use any delay
        while (remain && (io_read16(BSC_S_(dev)) & BSC_S_RXD))
        {
            ((uint8_t *)data)[received] = (uint8_t)io_read32(BSC_FIFO_(dev));
            received++;
            remain--;
        }
    }

    // transfer has finished - grab any remainning stuff in fifo
    while (remain && (io_read16(BSC_S_(dev)) & BSC_S_RXD))
    {
        ((uint8_t *)data)[received] = (uint8_t)io_read32(BSC_FIFO_(dev));
        received++;
        remain--;
    }

    if(io_read16(BSC_S_(dev)) & BSC_S_ERR){
        // nack
        DMSG("read error nack");
        show_bsc_debug_msg(remain,io_read16(BSC_S_(dev)));
        res=TEE_ERROR_NO_DATA;
    }
    else if(io_read16(BSC_S_(dev)) & BSC_S_CLKT){
        // clock stretch timeout
        DMSG("read error clock stretch timeout");
        show_bsc_debug_msg(remain,io_read16(BSC_S_(dev)));
        res=TEE_ERROR_TIMEOUT;
    }
    else if(remain){
        DMSG("still remain");
        res=TEE_ERROR_BAD_STATE;
    }
    io_write32(BSC_S_(dev),BSC_S_CLKT | BSC_S_ERR | BSC_S_DONE);
    return res;
}

TEE_Result i2c_write_byte(i2c_t dev, uint16_t addr, uint8_t data, uint8_t flags)
{
    return i2c_write_bytes(dev, addr, &data, 1, flags);
}

TEE_Result i2c_write_bytes(i2c_t dev, uint16_t addr, const void *data,
                           uint16_t len, uint8_t flags)
{
    TEE_Result res = TEE_SUCCESS;
    uint32_t control= BSC_C_I2CEN | BSC_C_ST;
    uint16_t remain = len;
    uint16_t sent = 0;
    uint64_t fail_safe = len * 10000;
    int safe_timeout=0;
    if (__builtin_expect(dev >= I2C_NUM || len == 0 || data == NULL || (len==UINT16_MAX&&flags&I2C_ADDR10), 0))
        return TEE_ERROR_BAD_PARAMETERS;
    // clear fifo
    // DMSG("[i2c] clear fifo for %d", dev);
    if(!(flags&I2C_CONT)){
        io_setbits32(BSC_C_(dev),BSC_C_CLEAR); // clear fifo
        io_write32(BSC_S_(dev),BSC_S_CLKT | BSC_S_ERR | BSC_S_DONE); // clear status
    }
    

    if (flags & I2C_ADDR10)
    {
        // 10 bits addr
        io_write32(BSC_DLEN_(dev), (uint32_t)(len + 1)); // data len
        // send slave address he least 8 bits
        io_write32(BSC_FIFO_(dev), (uint32_t)(addr & 0xFF)); // send slave addr least 8 bits
        sent++;
        io_write32(BSC_FIFO_(dev), (uint32_t)((addr >> 8) & 0x03) | 0x78); // send slave addr most 2 bits
    }
    else
    {
        // 7 bits addr
        io_write32(BSC_DLEN_(dev), (uint32_t)len); // data len
        io_write32(BSC_A_(dev), (uint8_t)addr);
    }
    // pre populate fifo with max buffer
    while(remain&&(sent<BSC_FIFO_SIZE)){
        io_write32(BSC_FIFO_(dev),((uint8_t *)data)[sent]);
        sent++;
        remain--;
    }
    
    // write start 
    io_write32(BSC_C_(dev), control);
    
    // transfer is over when done is set
    while (!safe_timeout && !(io_read32(BSC_S_(dev)) & BSC_S_DONE))
    {
        while(!safe_timeout && remain &&(io_read32(BSC_S_(dev)) & BSC_S_TXD)){
            io_write32(BSC_FIFO_(dev),((uint8_t *)data)[sent]); // write data
            sent++;
            remain--;
            if(--fail_safe==0){
                // timeout
                safe_timeout=1;
                break;
            }
        }
        if(--fail_safe==0){
            // timeout
            safe_timeout=1;
            break;
        }
    }
    if(safe_timeout){
        DMSG("write timeout");
        res=TEE_ERROR_TIMEOUT;
    }
    else if(io_read32(BSC_S_(dev)) & BSC_S_ERR){
        // nack
        DMSG("write error nack");
        show_bsc_debug_msg(remain,io_read32(BSC_S_(dev)));
        res=TEE_ERROR_NO_DATA;
    }
    else if(io_read32(BSC_S_(dev)) & BSC_S_CLKT){
        // clock stretch timeout
        DMSG("write error clock stretch timeout");
        show_bsc_debug_msg(remain,io_read32(BSC_S_(dev)));
        res=TEE_ERROR_TIMEOUT;
    }
    else if(remain){
        DMSG("still remain");
        res=TEE_ERROR_BAD_STATE;
    }
    io_setbits32(BSC_S_(dev),BSC_S_CLKT | BSC_S_ERR | BSC_S_DONE);
    return res;
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
    if(flags & I2C_REG16){
        return TEE_ERROR_NOT_IMPLEMENTED;
    }
    for(int i=0;i<len;i++){
        res = i2c_write_byte(dev, addr,(uint8_t)reg+i,
                          flags);
        if (res != TEE_SUCCESS)
            return res;
        flags|=I2C_CONT;
        res = i2c_read_byte(dev, addr, &(((uint8_t*)data)[i]), flags);
        if (res != TEE_SUCCESS)
            return res;
    }
    return TEE_SUCCESS;
}

TEE_Result i2c_write_reg(i2c_t dev, uint16_t addr, uint16_t reg,
                         uint8_t data, uint8_t flags)
{
    // put reg and data into one array
    uint8_t all[2];
    if(flags & I2C_REG16){
        return TEE_ERROR_NOT_IMPLEMENTED;
    }
    all[0]=(uint8_t) reg;
    all[1]=data;
    return i2c_write_bytes(dev, addr, all, 2, flags);
}

TEE_Result i2c_write_regs(i2c_t dev, uint16_t addr, uint16_t reg,
                          const void *data, uint16_t len, uint8_t flags)
{
    TEE_Result res;
    uint8_t real_reg=(uint8_t) reg;
    if(flags & I2C_REG16){
        return TEE_ERROR_NOT_IMPLEMENTED;
    }
    // DMSG("write to reg %x",real_reg);
    // reg = htons(reg);
    res = i2c_write_bytes(dev, addr,&real_reg,1,
                          flags);
    if (res != TEE_SUCCESS)
        return res;
    flags|=I2C_CONT;
    return i2c_write_bytes(dev, addr, data, len, flags);
}

TEE_Result i2c_write_with_prefix(i2c_t dev, uint16_t addr, const void *prefix, uint16_t prefix_len, const void *data, uint16_t len, uint8_t flags)
{
    TEE_Result res = TEE_SUCCESS;
    uint32_t control= BSC_C_I2CEN | BSC_C_ST;
    uint16_t remain = prefix_len+len;
    uint16_t sent = 0;
    uint64_t fail_safe = (prefix_len+len) * 10000;
    int safe_timeout=0;
    if (__builtin_expect(dev >= I2C_NUM || len == 0 || prefix==NULL || data == NULL || (len==UINT16_MAX&&flags&I2C_ADDR10), 0))
        return TEE_ERROR_BAD_PARAMETERS;
    if (flags&I2C_ADDR10){
        return TEE_ERROR_NOT_IMPLEMENTED;
    }
    // clear fifo
    // DMSG("[i2c] clear fifo for %d", dev);
    if(!(flags&I2C_CONT)){
        io_setbits32(BSC_C_(dev),BSC_C_CLEAR); // clear fifo
        io_write32(BSC_S_(dev),BSC_S_CLKT | BSC_S_ERR | BSC_S_DONE); // clear status
    }

    // 7 bits addr
    io_write32(BSC_DLEN_(dev), (uint32_t)prefix_len+len); // data len
    io_write32(BSC_A_(dev), (uint8_t)addr);
    
    // pre populate fifo with max buffer
    while(remain&&(sent<BSC_FIFO_SIZE)){
        if(sent<prefix_len){
            io_write32(BSC_FIFO_(dev),((uint8_t *)prefix)[sent]);
        }else{
            io_write32(BSC_FIFO_(dev),((uint8_t *)data)[sent-prefix_len]);
        }
        sent++;
        remain--;
    }
    
    // write start 
    io_write32(BSC_C_(dev), control);
    
    // transfer is over when done is set
    while (!safe_timeout && !(io_read32(BSC_S_(dev)) & BSC_S_DONE))
    {
        while(!safe_timeout && remain &&(io_read32(BSC_S_(dev)) & BSC_S_TXD)){
            if(sent<prefix_len){
                io_write32(BSC_FIFO_(dev),((uint8_t *)prefix)[sent]);
            }else{
                io_write32(BSC_FIFO_(dev),((uint8_t *)data)[sent-prefix_len]);
            }
            sent++;
            remain--;
            if(--fail_safe==0){
                // timeout
                safe_timeout=1;
                break;
            }
        }
        if(--fail_safe==0){
            // timeout
            safe_timeout=1;
            break;
        }
    }
    if(safe_timeout){
        DMSG("write timeout");
        res=TEE_ERROR_TIMEOUT;
    }
    else if(io_read32(BSC_S_(dev)) & BSC_S_ERR){
        // nack
        DMSG("write error nack");
        show_bsc_debug_msg(remain,io_read32(BSC_S_(dev)));
        res=TEE_ERROR_NO_DATA;
    }
    else if(io_read32(BSC_S_(dev)) & BSC_S_CLKT){
        // clock stretch timeout
        DMSG("write error clock stretch timeout");
        show_bsc_debug_msg(remain,io_read32(BSC_S_(dev)));
        res=TEE_ERROR_TIMEOUT;
    }
    else if(remain){
        DMSG("still remain");
        res=TEE_ERROR_BAD_STATE;
    }
    io_setbits32(BSC_S_(dev),BSC_S_CLKT | BSC_S_ERR | BSC_S_DONE);
    return res;
}

TEE_Result i2c_set_frequency(i2c_t dev, i2c_speed_t freq)
{
    switch (freq)
    {
    case I2C_SPEED_LOW:
        return i2c_set_divider(dev, 10000);
    case I2C_SPEED_NORMAL:
        return i2c_set_divider(dev, 100000);
    case I2C_SPEED_FAST:
        return i2c_set_divider(dev, 400000);
    case I2C_SPEED_FAST_PLUS:
        return i2c_set_divider(dev, 1000000);
    case I2C_SPEED_HIGH:
        return i2c_set_divider(dev, 3400000);
    default:
        return TEE_ERROR_NOT_SUPPORTED;
    }
}

static TEE_Result rpi3_i2c_init(void)
{
    if (cpu_mmu_enabled())
    {
        bsc_registers[0] = (struct BSCRegister *)phys_to_virt((paddr_t)BSC0_BASE,MEM_AREA_IO_NSEC, sizeof(struct BSCRegister));
        bsc_registers[1] = (struct BSCRegister *)phys_to_virt((paddr_t)BSC1_BASE,MEM_AREA_IO_NSEC, sizeof(struct BSCRegister));
        for(int i=0;i<I2C_NUM;i++){
            if(bsc_registers[i]==NULL){
                DMSG("init i2c%d virt addr base failed",i);
            }else{
                DMSG("init i2c%d virt addr base %p",i,bsc_registers[i]);
            }
        }
    }
    return TEE_SUCCESS;
}

driver_init(rpi3_i2c_init);