/**
 * @brief driver for spi
 * reference to http://www.airspayce.com/mikem/bcm2835/
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */

#include <periph/spi.h>
#include <platform_config.h>
#include <drivers/rpi3/rpi3_gpio.h>
#include <kernel/tee_time.h>
#include <kernel/mutex.h>
#include <mm/core_memprot.h>
#include <io.h>
#include <initcall.h>

#define SPI_NUM 1

#define SPI_CS_NUM 3

// #define SPI_CS                      0x0000 /*!< SPI Master Control and Status */
// #define SPI_FIFO                    0x0004 /*!< SPI Master TX and RX FIFOs */
// #define SPI_CLK                     0x0008 /*!< SPI Master Clock Divider */
// #define SPI_DLEN                    0x000c /*!< SPI Master Data Length */
// #define SPI_LTOH                    0x0010 /*!< SPI LOSSI mode TOH */
// #define SPI_DC                      0x0014 /*!< SPI DMA DREQ Controls */

struct SPIRegister{
    uint32_t SPI_CS;
    uint32_t SPI_FIFO;
    uint32_t SPI_CLK;
    uint32_t SPI_DLEN;
    uint32_t SPI_LTOH;
    uint32_t SPI_DC;
};

#define SPI_CS_LEN_LONG             0x02000000 /*!< Enable Long data word in Lossi mode if DMA_LEN is set */
#define SPI_CS_DMA_LEN              0x01000000 /*!< Enable DMA mode in Lossi mode */
#define SPI_CS_CSPOL2               0x00800000 /*!< Chip Select 2 Polarity */
#define SPI_CS_CSPOL1               0x00400000 /*!< Chip Select 1 Polarity */
#define SPI_CS_CSPOL0               0x00200000 /*!< Chip Select 0 Polarity */
#define SPI_CS_RXF                  0x00100000 /*!< RXF - RX FIFO Full */
#define SPI_CS_RXR                  0x00080000 /*!< RXR RX FIFO needs Reading (full) */
#define SPI_CS_TXD                  0x00040000 /*!< TXD TX FIFO can accept Data */
#define SPI_CS_RXD                  0x00020000 /*!< RXD RX FIFO contains Data */
#define SPI_CS_DONE                 0x00010000 /*!< Done transfer Done */
#define SPI_CS_TE_EN                0x00008000 /*!< Unused */
#define SPI_CS_LMONO                0x00004000 /*!< Unused */
#define SPI_CS_LEN                  0x00002000 /*!< LEN LoSSI enable */
#define SPI_CS_REN                  0x00001000 /*!< REN Read Enable */
#define SPI_CS_ADCS                 0x00000800 /*!< ADCS Automatically Deassert Chip Select */
#define SPI_CS_INTR                 0x00000400 /*!< INTR Interrupt on RXR */
#define SPI_CS_INTD                 0x00000200 /*!< INTD Interrupt on Done */
#define SPI_CS_DMAEN                0x00000100 /*!< DMAEN DMA Enable */
#define SPI_CS_TA                   0x00000080 /*!< Transfer Active */
#define SPI_CS_CSPOL                0x00000040 /*!< Chip Select Polarity */
#define SPI_CS_CLEAR                0x00000030 /*!< Clear FIFO Clear RX and TX */
#define SPI_CS_CLEAR_RX             0x00000020 /*!< Clear FIFO Clear RX  */
#define SPI_CS_CLEAR_TX             0x00000010 /*!< Clear FIFO Clear TX  */
#define SPI_CS_CPOL                 0x00000008 /*!< Clock Polarity */
#define SPI_CS_CPHA                 0x00000004 /*!< Clock Phase */
#define SPI_CS_CS                   0x00000003 /*!< Chip Select */

#define SPI_MODE_SHIFT(x)   (x<<2)
#define SPI_CSPOL_SHIFT_BASE    21

#define SPI_WAIT_TXD_US 10000
#define SPI_WAIT_RXD_US 10000
#define SPI_TRANSFER_WAIT_DONE_PERBYTE_US 1000


static struct SPIRegister * spi_registers[SPI_NUM] = {
    (struct SPIRegister *) SPI0_BASE
};

static void spi_setbits(vaddr_t addr, uint32_t value, uint32_t mask){
    uint32_t v = io_read32(addr);
    v = (v & ~mask) | (value & mask);
    io_write32(addr,v);
}

static inline uint64_t teetime_micro(TEE_Time *t)
{
    return t->seconds * 1000000 + t->micros;
}


/*! Rpi3SPIBitOrder SPI Bit order
  Specifies the SPI data bit ordering for bcm2835_spi_setBitOrder()
*/
typedef enum
{
    RPI3_SPI_BIT_ORDER_LSBFIRST = 0,  /*!< LSB First */
    RPI3_SPI_BIT_ORDER_MSBFIRST = 1   /*!< MSB First */
}Rpi3SPIBitOrder;


/* SPI bit order. BCM2835 SPI0 only supports MSBFIRST, so we instead 
 * have a software based bit reversal, based on a contribution by Damiano Benedetti
 */
static uint8_t rpi3_spi_bit_order = RPI3_SPI_BIT_ORDER_MSBFIRST;
static uint8_t rpi3_byte_reverse_table[] = 
{
    0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
    0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
    0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
    0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
    0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
    0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
    0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
    0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
    0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
    0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
    0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
    0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
    0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
    0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
    0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
    0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
    0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
    0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
    0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
    0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
    0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
    0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
    0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
    0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
    0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
    0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
    0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
    0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
    0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
    0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
    0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
    0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};


static inline uint8_t rpi3_correct_order(uint8_t v){
    if(rpi3_spi_bit_order == RPI3_SPI_BIT_ORDER_LSBFIRST)
        return rpi3_byte_reverse_table[v];
    else
        return v;
}

static struct mutex spi_locks[SPI_NUM] = {
    MUTEX_INITIALIZER
};

static inline vaddr_t SPI_CS_(spi_t dev)
{
    return (vaddr_t)&(spi_registers[dev]->SPI_CS);
}

static inline vaddr_t SPI_FIFO_(spi_t dev)
{
    return (vaddr_t)&(spi_registers[dev]->SPI_FIFO);
}

static inline vaddr_t SPI_CLK_(spi_t dev)
{
    return (vaddr_t)&(spi_registers[dev]->SPI_CLK);
}

static inline vaddr_t SPI_DLEN_(spi_t dev)
{
    return (vaddr_t)&(spi_registers[dev]->SPI_DLEN);
}

static inline vaddr_t SPI_LTOH_(spi_t dev)
{
    return (vaddr_t)&(spi_registers[dev]->SPI_LTOH);
}

static inline vaddr_t SPI_DC_(spi_t dev)
{
    return (vaddr_t)&(spi_registers[dev]->SPI_DC);
}

static inline void spi_set_speed_clk(spi_t bus, spi_clk_t clk){
    uint32_t speed_hz;
    uint16_t divider;
    switch(clk){
        case SPI_CLK_100KHZ:
            speed_hz=100000;
            break;
        case SPI_CLK_400KHZ:
            speed_hz=400000;
            break;
        case SPI_CLK_1MHZ:
            speed_hz=1000000;
            break;
        case SPI_CLK_5MHZ:
            speed_hz=5000000;
            break;
        case SPI_CLK_10MHZ:
            speed_hz=10000000;
            break;
        case SPI_CLK_40MHZ:
            speed_hz=40000000;
            break;
        default:
            DMSG("spi clk not supported, use default 100KHZ");
            speed_hz=4000000;
    }
    divider=(uint16_t)((uint32_t)CORE_CLK/speed_hz);
    divider&=0xFFFE; // must be power of 2, rouded down
    io_write32(SPI_CLK_(bus),divider);
}

static inline void spi_set_mode(spi_t bus, spi_mode_t mode){
    io_clrsetbits32(SPI_CS_(bus),SPI_CS_CPOL|SPI_CS_CPHA,SPI_MODE_SHIFT(mode));
}

static inline void spi_chip_select(spi_t bus, spi_cs_t cs){
    io_clrsetbits32(SPI_CS_(bus),SPI_CS_CS,cs);
}

// active 0 or 1
static inline void spi_set_chip_select_polarity(spi_t bus, spi_cs_t cs, uint8_t active){
    uint8_t shift = SPI_CSPOL_SHIFT_BASE + cs;
    io_clrsetbits32(SPI_CS_(bus),1<<shift,active<<shift);
}

// set bit order
static inline void spi_set_bit_order(uint8_t order){
    rpi3_spi_bit_order = order;
}

TEE_Result spi_acquire(spi_t bus, spi_cs_t cs, spi_mode_t mode, spi_clk_t clk){
    if(__builtin_expect(bus>=SPI_NUM||cs>=SPI_CS_NUM,0))
        return TEE_ERROR_BAD_PARAMETERS;

    mutex_lock(&spi_locks[bus]);
    if(bus==0){
        gpio_fsel((gpio_t)7,GPIO_FSEL_ALT0); // CE1
        gpio_fsel((gpio_t)8,GPIO_FSEL_ALT0); // CE0
        gpio_fsel((gpio_t)9,GPIO_FSEL_ALT0); // MISO
        gpio_fsel((gpio_t)10,GPIO_FSEL_ALT0); // MOSI
        gpio_fsel((gpio_t)11,GPIO_FSEL_ALT0); // SCLK
    }

    // clear CS
    io_write32(SPI_CS_(bus),0);
    // clear FIFO TX RX
    io_write32(SPI_CS_(bus),SPI_CS_CLEAR);

    // spi chip select
    spi_chip_select(bus,cs);
    // set clk
    spi_set_speed_clk(bus,clk);
    // set mode
    spi_set_mode(bus,mode);
    

    return TEE_SUCCESS;
}

TEE_Result spi_release(spi_t bus){
    if(__builtin_expect(bus>=SPI_NUM,0))
        return TEE_ERROR_BAD_PARAMETERS;
    if(bus==0){
        gpio_fsel((gpio_t)7,GPIO_FSEL_INPT); // CE1
        gpio_fsel((gpio_t)8,GPIO_FSEL_INPT); // CE0
        gpio_fsel((gpio_t)9,GPIO_FSEL_INPT); // MISO
        gpio_fsel((gpio_t)10,GPIO_FSEL_INPT); // MOSI
        gpio_fsel((gpio_t)11,GPIO_FSEL_INPT); // CLK
    }
    mutex_unlock(&spi_locks[bus]);
    return TEE_SUCCESS;
}

TEE_Result spi_transfer_byte(spi_t bus, spi_cs_t cs, bool cont, uint8_t out, uint8_t * in){
    TEE_Time start,end;
    uint32_t status;
    uint32_t ret;
    if(__builtin_expect(bus>=SPI_NUM||cs>=SPI_CS_NUM,0))
        return TEE_ERROR_BAD_PARAMETERS;

    // spi chip select
    spi_chip_select(bus,cs);
    if(!cont){
        // clear TX RX fifos
        spi_setbits(SPI_CS_(bus),SPI_CS_CLEAR,SPI_CS_CLEAR);
    }// continue last transfer, don't clear
    
    // set TA 1
    spi_setbits(SPI_CS_(bus),SPI_CS_TA,SPI_CS_TA);

    // maybe wait for TXD
    tee_time_get_sys_time(&start);
    do{
        status=io_read32(SPI_CS_(bus));
        tee_time_get_sys_time(&end); // update
    }while(!(status & SPI_CS_TXD)&&teetime_micro(&end)-teetime_micro(&start)<=SPI_WAIT_TXD_US);
    if(teetime_micro(&end)-teetime_micro(&start)>SPI_WAIT_TXD_US){
        spi_setbits(SPI_CS_(bus),0,SPI_CS_TA);
        return TEE_ERROR_TIMEOUT;
    }

    // write fifo
    io_write32(SPI_FIFO_(bus),rpi3_correct_order(out));

    // wait for DONE
    tee_time_get_sys_time(&start);
    do{
        status=io_read32(SPI_CS_(bus));
        tee_time_get_sys_time(&end); // update
    }while(!(status & SPI_CS_DONE)&&teetime_micro(&end)-teetime_micro(&start)<=SPI_TRANSFER_WAIT_DONE_PERBYTE_US);
    if(teetime_micro(&end)-teetime_micro(&start)>SPI_TRANSFER_WAIT_DONE_PERBYTE_US){
        spi_setbits(SPI_CS_(bus),0,SPI_CS_TA);
        return TEE_ERROR_TIMEOUT;
    }

    // need read
    if(in!=NULL){
        // read a byte
        ret = rpi3_correct_order((uint8_t)io_read32(SPI_FIFO_(bus)));
        *in=(uint8_t)ret;
    }
    spi_setbits(SPI_CS_(bus),0,SPI_CS_TA);
    return TEE_SUCCESS;
}

TEE_Result spi_transfer_bytes(spi_t bus, spi_cs_t cs, bool cont,
                        const void *out, void *in, size_t len){
    uint32_t status=0;
    uint32_t sent=0;
    uint32_t received=0;
    TEE_Time start,end;
    // spi chip select
    spi_chip_select(bus,cs);

    if(!cont){
        // clear TX RX fifos
        spi_setbits(SPI_CS_(bus),SPI_CS_CLEAR,SPI_CS_CLEAR);
    }// continue last transfer, don't clear

    // set TA = 1
    spi_setbits(SPI_CS_(bus),SPI_CS_TA,SPI_CS_TA);

    if(out!=NULL&&in!=NULL){
        tee_time_get_sys_time(&start);
        tee_time_get_sys_time(&end);
        while(((sent<len)||(received<len))&&teetime_micro(&end)-teetime_micro(&start)<=SPI_WAIT_TXD_US){
            // TX fifo not full
            while(((io_read32(SPI_CS_(bus))&SPI_CS_TXD))&&(sent<len)){
                io_write32(SPI_FIFO_(bus),rpi3_correct_order(((uint8_t *)out)[sent]));
                sent++;
                tee_time_get_sys_time(&start); // update start
            }
            // RX fifo not empty
            while(((io_read32(SPI_CS_(bus))&SPI_CS_RXD))&&(received<len)){
                ((uint8_t *)in)[received] = rpi3_correct_order((uint8_t)io_read32(SPI_FIFO_(bus)));
                received++;
                tee_time_get_sys_time(&start); // update start
            }
            tee_time_get_sys_time(&end); // update end
        }
        if(teetime_micro(&end)-teetime_micro(&start)>SPI_WAIT_TXD_US){
            spi_setbits(SPI_CS_(bus),0,SPI_CS_TA);
            return TEE_ERROR_TIMEOUT;
        }
        // wait for DONE
        tee_time_get_sys_time(&start);
        do{
            status=io_read32(SPI_CS_(bus));
            tee_time_get_sys_time(&end); // update
        }while(!(status & SPI_CS_DONE)&&teetime_micro(&end)-teetime_micro(&start)<=SPI_TRANSFER_WAIT_DONE_PERBYTE_US*len);
        if(teetime_micro(&end)-teetime_micro(&start)>SPI_TRANSFER_WAIT_DONE_PERBYTE_US*len){
            spi_setbits(SPI_CS_(bus),0,SPI_CS_TA);
            return TEE_ERROR_TIMEOUT;
        }

    }else if(out!=NULL){
        DMSG("spi out");
        tee_time_get_sys_time(&start);
        tee_time_get_sys_time(&end);
        while(sent<len&&teetime_micro(&end)-teetime_micro(&start)<=SPI_WAIT_TXD_US){
            // TX fifo not full
            while(((io_read32(SPI_CS_(bus))&SPI_CS_TXD))&&(sent<len)){
                io_write32(SPI_FIFO_(bus),rpi3_correct_order(((uint8_t *)out)[sent]));
                sent++;
                tee_time_get_sys_time(&start); // update start
            }
            tee_time_get_sys_time(&end); // update end
        }
        if(teetime_micro(&end)-teetime_micro(&start)>SPI_WAIT_TXD_US){
            spi_setbits(SPI_CS_(bus),0,SPI_CS_TA);
            return TEE_ERROR_TIMEOUT;
        }
        // wait for DONE
        tee_time_get_sys_time(&start);
        do{
            status=io_read32(SPI_CS_(bus));
            tee_time_get_sys_time(&end); // update
        }while(!(status & SPI_CS_DONE)&&teetime_micro(&end)-teetime_micro(&start)<=SPI_TRANSFER_WAIT_DONE_PERBYTE_US*len);
        if(teetime_micro(&end)-teetime_micro(&start)>SPI_TRANSFER_WAIT_DONE_PERBYTE_US*len){
            spi_setbits(SPI_CS_(bus),0,SPI_CS_TA);
            return TEE_ERROR_TIMEOUT;
        }
    }else if(in!=NULL){
        DMSG("spi in");
        tee_time_get_sys_time(&start);
        tee_time_get_sys_time(&end);
        while(received<len&&teetime_micro(&end)-teetime_micro(&start)<=SPI_WAIT_RXD_US){
            // RX fifo not empty
            while(((io_read32(SPI_CS_(bus))&SPI_CS_RXD))&&(received<len)){
                ((uint8_t *)in)[received] = rpi3_correct_order((uint8_t)io_read32(SPI_FIFO_(bus)));
                received++;
                tee_time_get_sys_time(&start); // update start
            }
            tee_time_get_sys_time(&end); // update end
        }
        if(teetime_micro(&end)-teetime_micro(&start)>SPI_WAIT_RXD_US){
            spi_setbits(SPI_CS_(bus),0,SPI_CS_TA);
            return TEE_ERROR_TIMEOUT;
        }
        // wait for DONE
        tee_time_get_sys_time(&start);
        do{
            status=io_read32(SPI_CS_(bus));
            tee_time_get_sys_time(&end); // update
        }while(!(status & SPI_CS_DONE)&&teetime_micro(&end)-teetime_micro(&start)<=SPI_TRANSFER_WAIT_DONE_PERBYTE_US*len);
        if(teetime_micro(&end)-teetime_micro(&start)>SPI_TRANSFER_WAIT_DONE_PERBYTE_US*len){
            spi_setbits(SPI_CS_(bus),0,SPI_CS_TA);
            return TEE_ERROR_TIMEOUT;
        }
    }

    // set TA = 0
    spi_setbits(SPI_CS_(bus),0,SPI_CS_TA);
    return TEE_SUCCESS;
}

TEE_Result spi_transfer_diff_bytes(spi_t bus, spi_cs_t cs, bool cont,
                        const void *out, void *in, size_t out_len,size_t in_len){
    uint32_t status=0;
    uint32_t sent=0;
    uint32_t received=0;
    TEE_Time start,end;
    // spi chip select
    spi_chip_select(bus,cs);

    if(!cont){
        // clear TX RX fifos
        spi_setbits(SPI_CS_(bus),SPI_CS_CLEAR,SPI_CS_CLEAR);
    }// continue last transfer, don't clear

    // set TA = 1
    spi_setbits(SPI_CS_(bus),SPI_CS_TA,SPI_CS_TA);

    if(out!=NULL&&in!=NULL){
        tee_time_get_sys_time(&start);
        tee_time_get_sys_time(&end);
        while(((sent<out_len)||(received<in_len))&&teetime_micro(&end)-teetime_micro(&start)<=SPI_WAIT_TXD_US){
            // TX fifo not full
            while(((io_read32(SPI_CS_(bus))&SPI_CS_TXD))&&(sent<out_len)){
                io_write32(SPI_FIFO_(bus),rpi3_correct_order(((uint8_t *)out)[sent]));
                sent++;
                tee_time_get_sys_time(&start); // update start
            }
            // RX fifo not empty
            while(((io_read32(SPI_CS_(bus))&SPI_CS_RXD))&&(received<in_len)){
                ((uint8_t *)in)[received] = rpi3_correct_order((uint8_t)io_read32(SPI_FIFO_(bus)));
                received++;
                tee_time_get_sys_time(&start); // update start
            }
            tee_time_get_sys_time(&end); // update end
        }
        if(teetime_micro(&end)-teetime_micro(&start)>SPI_WAIT_TXD_US){
            spi_setbits(SPI_CS_(bus),0,SPI_CS_TA);
            return TEE_ERROR_TIMEOUT;
        }
        // wait for DONE
        tee_time_get_sys_time(&start);
        do{
            status=io_read32(SPI_CS_(bus));
            tee_time_get_sys_time(&end); // update
        }while(!(status & SPI_CS_DONE)&&teetime_micro(&end)-teetime_micro(&start)<=SPI_TRANSFER_WAIT_DONE_PERBYTE_US*(out_len>in_len?out_len:in_len));
        if(teetime_micro(&end)-teetime_micro(&start)>SPI_TRANSFER_WAIT_DONE_PERBYTE_US*(out_len>in_len?out_len:in_len)){
            spi_setbits(SPI_CS_(bus),0,SPI_CS_TA);
            return TEE_ERROR_TIMEOUT;
        }

    }else if(out!=NULL){
        DMSG("spi out");
        tee_time_get_sys_time(&start);
        tee_time_get_sys_time(&end);
        while(sent<out_len&&teetime_micro(&end)-teetime_micro(&start)<=SPI_WAIT_TXD_US){
            // TX fifo not full
            while(((io_read32(SPI_CS_(bus))&SPI_CS_TXD))&&(sent<out_len)){
                io_write32(SPI_FIFO_(bus),rpi3_correct_order(((uint8_t *)out)[sent]));
                sent++;
                tee_time_get_sys_time(&start); // update start
            }
            tee_time_get_sys_time(&end); // update end
        }
        if(teetime_micro(&end)-teetime_micro(&start)>SPI_WAIT_TXD_US){
            spi_setbits(SPI_CS_(bus),0,SPI_CS_TA);
            return TEE_ERROR_TIMEOUT;
        }
        // wait for DONE
        tee_time_get_sys_time(&start);
        do{
            status=io_read32(SPI_CS_(bus));
            tee_time_get_sys_time(&end); // update
        }while(!(status & SPI_CS_DONE)&&teetime_micro(&end)-teetime_micro(&start)<=SPI_TRANSFER_WAIT_DONE_PERBYTE_US*out_len);
        if(teetime_micro(&end)-teetime_micro(&start)>SPI_TRANSFER_WAIT_DONE_PERBYTE_US*out_len){
            spi_setbits(SPI_CS_(bus),0,SPI_CS_TA);
            return TEE_ERROR_TIMEOUT;
        }
    }else if(in!=NULL){
        DMSG("spi in");
        tee_time_get_sys_time(&start);
        tee_time_get_sys_time(&end);
        while(received<in_len&&teetime_micro(&end)-teetime_micro(&start)<=SPI_WAIT_RXD_US){
            // RX fifo not empty
            while(((io_read32(SPI_CS_(bus))&SPI_CS_RXD))&&(received<in_len)){
                ((uint8_t *)in)[received] = rpi3_correct_order((uint8_t)io_read32(SPI_FIFO_(bus)));
                received++;
                tee_time_get_sys_time(&start); // update start
            }
            tee_time_get_sys_time(&end); // update end
        }
        if(teetime_micro(&end)-teetime_micro(&start)>SPI_WAIT_RXD_US){
            spi_setbits(SPI_CS_(bus),0,SPI_CS_TA);
            return TEE_ERROR_TIMEOUT;
        }
        // wait for DONE
        tee_time_get_sys_time(&start);
        do{
            status=io_read32(SPI_CS_(bus));
            tee_time_get_sys_time(&end); // update
        }while(!(status & SPI_CS_DONE)&&teetime_micro(&end)-teetime_micro(&start)<=SPI_TRANSFER_WAIT_DONE_PERBYTE_US*in_len);
        if(teetime_micro(&end)-teetime_micro(&start)>SPI_TRANSFER_WAIT_DONE_PERBYTE_US*in_len){
            spi_setbits(SPI_CS_(bus),0,SPI_CS_TA);
            return TEE_ERROR_TIMEOUT;
        }
    }

    // set TA = 0
    spi_setbits(SPI_CS_(bus),0,SPI_CS_TA);
    return TEE_SUCCESS;
}

TEE_Result spi_transfer_reg(spi_t bus, spi_cs_t cs, uint8_t reg, uint8_t out, uint8_t * in){
    return spi_transfer_regs(bus,cs,reg,&out,in,1);
}

TEE_Result spi_transfer_regs(spi_t bus, spi_cs_t cs, uint8_t reg,
                       const void *out, void *in, size_t len){
    uint32_t status=0;
    uint32_t sent=0;
    uint32_t received=0;
    TEE_Time start,end;
    // spi chip select
    spi_chip_select(bus,cs);

    
    // clear TX RX fifos
    spi_setbits(SPI_CS_(bus),SPI_CS_CLEAR,SPI_CS_CLEAR);
    

    // set TA = 1
    spi_setbits(SPI_CS_(bus),SPI_CS_TA,SPI_CS_TA);

    // send reg
    io_write32(SPI_FIFO_(bus),rpi3_correct_order(reg));

    if(out!=NULL&&in!=NULL){
        tee_time_get_sys_time(&start);
        tee_time_get_sys_time(&end);
        while(((sent<len)||(received<len))&&teetime_micro(&end)-teetime_micro(&start)<=SPI_WAIT_TXD_US){
            // TX fifo not full
            while(((io_read32(SPI_CS_(bus))&SPI_CS_TXD))&&(sent<len)){
                io_write32(SPI_FIFO_(bus),rpi3_correct_order(((uint8_t *)out)[sent]));
                sent++;
                tee_time_get_sys_time(&start); // update start
            }
            // RX fifo not empty
            while(((io_read32(SPI_CS_(bus))&SPI_CS_RXD))&&(received<len)){
                ((uint8_t *)in)[received] = rpi3_correct_order((uint8_t)io_read32(SPI_FIFO_(bus)));
                received++;
                tee_time_get_sys_time(&start); // update start
            }
            tee_time_get_sys_time(&end); // update end
        }
        if(teetime_micro(&end)-teetime_micro(&start)>SPI_WAIT_TXD_US){
            spi_setbits(SPI_CS_(bus),0,SPI_CS_TA);
            return TEE_ERROR_TIMEOUT;
        }
        // wait for DONE
        tee_time_get_sys_time(&start);
        do{
            status=io_read32(SPI_CS_(bus));
            tee_time_get_sys_time(&end); // update
        }while(!(status & SPI_CS_DONE)&&teetime_micro(&end)-teetime_micro(&start)<=SPI_TRANSFER_WAIT_DONE_PERBYTE_US*len);
        if(teetime_micro(&end)-teetime_micro(&start)>SPI_TRANSFER_WAIT_DONE_PERBYTE_US*len){
            spi_setbits(SPI_CS_(bus),0,SPI_CS_TA);
            return TEE_ERROR_TIMEOUT;
        }

    }else if(out!=NULL){
        // DMSG("spi out");
        tee_time_get_sys_time(&start);
        tee_time_get_sys_time(&end);
        while(sent<len&&teetime_micro(&end)-teetime_micro(&start)<=SPI_WAIT_TXD_US){
            // TX fifo not full
            while(((io_read32(SPI_CS_(bus))&SPI_CS_TXD))&&(sent<len)){
                io_write32(SPI_FIFO_(bus),rpi3_correct_order(((uint8_t *)out)[sent]));
                sent++;
                tee_time_get_sys_time(&start); // update start
            }
            tee_time_get_sys_time(&end); // update end
        }
        if(teetime_micro(&end)-teetime_micro(&start)>SPI_WAIT_TXD_US){
            spi_setbits(SPI_CS_(bus),0,SPI_CS_TA);
            return TEE_ERROR_TIMEOUT;
        }
        // wait for DONE
        tee_time_get_sys_time(&start);
        do{
            status=io_read32(SPI_CS_(bus));
            tee_time_get_sys_time(&end); // update
        }while(!(status & SPI_CS_DONE)&&teetime_micro(&end)-teetime_micro(&start)<=SPI_TRANSFER_WAIT_DONE_PERBYTE_US*len);
        if(teetime_micro(&end)-teetime_micro(&start)>SPI_TRANSFER_WAIT_DONE_PERBYTE_US*len){
            spi_setbits(SPI_CS_(bus),0,SPI_CS_TA);
            return TEE_ERROR_TIMEOUT;
        }
    }else if(in!=NULL){
        // DMSG("spi in");
        tee_time_get_sys_time(&start);
        tee_time_get_sys_time(&end);
        while(received<len&&teetime_micro(&end)-teetime_micro(&start)<=SPI_WAIT_RXD_US){
            // RX fifo not empty
            while(((io_read32(SPI_CS_(bus))&SPI_CS_RXD))&&(received<len)){
                ((uint8_t *)in)[received] = rpi3_correct_order((uint8_t)io_read32(SPI_FIFO_(bus)));
                received++;
                tee_time_get_sys_time(&start); // update start
            }
            tee_time_get_sys_time(&end); // update end
        }
        if(teetime_micro(&end)-teetime_micro(&start)>SPI_WAIT_RXD_US){
            spi_setbits(SPI_CS_(bus),0,SPI_CS_TA);
            return TEE_ERROR_TIMEOUT;
        }
        // wait for DONE
        tee_time_get_sys_time(&start);
        do{
            status=io_read32(SPI_CS_(bus));
            tee_time_get_sys_time(&end); // update
        }while(!(status & SPI_CS_DONE)&&teetime_micro(&end)-teetime_micro(&start)<=SPI_TRANSFER_WAIT_DONE_PERBYTE_US*len);
        if(teetime_micro(&end)-teetime_micro(&start)>SPI_TRANSFER_WAIT_DONE_PERBYTE_US*len){
            spi_setbits(SPI_CS_(bus),0,SPI_CS_TA);
            return TEE_ERROR_TIMEOUT;
        }
    }

    // set TA = 0
    spi_setbits(SPI_CS_(bus),0,SPI_CS_TA);
    return TEE_SUCCESS;
}


static TEE_Result rpi3_spi_init(void)
{
    if(cpu_mmu_enabled()){
        spi_registers[0] = (struct SPIRegister *)phys_to_virt((paddr_t)SPI0_BASE,MEM_AREA_IO_NSEC, sizeof(struct SPIRegister));
         for(int i=0;i<SPI_NUM;i++){
            if(spi_registers[i]==NULL){
                DMSG("init spi%d virt addr base failed",i);
            }else{
                DMSG("init spi%d virt addr base %p",i,spi_registers[i]);
            }
        }   
    }
    return TEE_SUCCESS;
}

driver_init(rpi3_spi_init);