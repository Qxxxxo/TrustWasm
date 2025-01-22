/**
 * @brief rpi3 gpio driver
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <periph/gpio.h>
#include <drivers/rpi3/rpi3_gpio.h>
#include <io.h>
#include <mm/core_memprot.h>
#include <platform_config.h>
#include <kernel/delay.h>
#include <stdint.h>
#include <initcall.h>

#define RPI3_MAX_PIN_NUM 53

typedef enum {
	GPPUD_OFF = 0x00,
	GPPUD_ENABLE_PULL_DOWN_CTRL=0x01,
	GPPUD_ENABLE_PULL_UP_CTRL=0x02,
} Rpi3_GPPUD_Value;

typedef enum {
	NO_EFFECT     =  0x0,
	ASSERT_CLK    =  0x1
} Rpi3_GPPUDCLK_Value;

#define NBITS_PER_FSEL_GPIO 3
#define NGPIOS_PER_FSEL     10
#define NGPIOS_PER_BANK      32 
#define NFSELS   6
#define NBANKS  2

#define RPI3_GPIO_FSEL_BANK(pin)     ((pin)/NGPIOS_PER_FSEL)
#define RPI3_GPIO_FSEL_SHIFT(pin)   (((pin)%NGPIOS_PER_FSEL)*NBITS_PER_FSEL_GPIO)
#define RPI3_GPIO_BANK(pin)           ((pin) / NGPIOS_PER_BANK)
#define RPI3_GPIO_SHIFT(pin)      ((pin) % NGPIOS_PER_BANK)


struct GpioRegister {
    uint32_t GPFSEL[NFSELS];
    uint32_t Reserved1;
    uint32_t GPSET[NBANKS];
    uint32_t Reserved2;
    uint32_t GPCLR[NBANKS];
    uint32_t Reserved3;
    uint32_t GPLEV[NBANKS];
    uint32_t Reserved4;
    uint32_t GPEDS[NBANKS];
    uint32_t Reserved5;
    uint32_t GPREN[NBANKS];
    uint32_t Reserved6;
    uint32_t GPFEN[NBANKS];
    uint32_t Reserved7;
    uint32_t GPHEN[NBANKS];
    uint32_t Reserved8;
    uint32_t GPLEN[NBANKS];
    uint32_t Reserved9;
    uint32_t GPAREN[NBANKS];
    uint32_t Reserved10;
    uint32_t GPAFEN[NBANKS];
    uint32_t Reserved11;
    uint32_t GPPUD[1];
    uint32_t GPPUDCLK[NBANKS];
};

static vaddr_t gpio_virt_base;
static struct GpioRegister * gpioRegister;

static inline vaddr_t get_base_address_GPFSEL(uint8_t index) {
    return (vaddr_t)&gpioRegister->GPFSEL[index];
}

static inline vaddr_t get_base_address_GPSET(uint8_t index) {
    return (vaddr_t)&gpioRegister->GPSET[index];
}

static inline vaddr_t get_base_address_GPCLR(uint8_t index) {
    return (vaddr_t)&gpioRegister->GPCLR[index];
}

static inline vaddr_t get_base_address_GPLEV(uint8_t index) {
    return (vaddr_t)&gpioRegister->GPLEV[index];
}

static inline vaddr_t get_base_address_GPPUD(void){
    return (vaddr_t)&gpioRegister->GPPUD[0];
}

static inline vaddr_t get_base_address_GPPUDCLK(uint8_t index){
    return (vaddr_t)&gpioRegister->GPPUDCLK[index];
}

static void rpi3_set_gpio_pin_function(gpio_t pin, Rpi3_FunctionSelect_GPFSEL functionCode) {
    // DMSG("pin %d fsel %d",pin,functionCode);
    uint32_t index_GPFSEL = RPI3_GPIO_FSEL_BANK(pin);
    uint32_t bit_GPFSEL = RPI3_GPIO_FSEL_SHIFT(pin);
    uint32_t clear_mask = GPIO_FESL_MASK << bit_GPFSEL; // 111
    io_clrsetbits32(get_base_address_GPFSEL(index_GPFSEL),clear_mask,(functionCode << bit_GPFSEL));
}

static inline void rpi3_set_gpio_pin_value(gpio_t pin, gpio_level_t value) {
	uint32_t index_register = RPI3_GPIO_BANK(pin);
	uint32_t shift = RPI3_GPIO_SHIFT(pin);
	vaddr_t base_addr = (value == GPIO_LOW_LEVEL) ? get_base_address_GPCLR(index_register) : get_base_address_GPSET(index_register);
    io_clrsetbits32(base_addr,BIT(shift),BIT(shift));
}

static inline gpio_level_t rpi3_get_gpio_pin_value(gpio_t pin){
    uint32_t index_register = RPI3_GPIO_BANK(pin);
    uint32_t shift = RPI3_GPIO_SHIFT(pin);
    return (io_read32(get_base_address_GPLEV(index_register)) & BIT(shift)) ? GPIO_HIGH_LEVEL : GPIO_LOW_LEVEL;
}

static inline void rpi3_set_GPPUD(Rpi3_GPPUD_Value value){
     io_write32(get_base_address_GPPUD(),(uint32_t)(value&0x03));
}

static void rpi3_set_GPPUDCLK(gpio_t pin, Rpi3_GPPUDCLK_Value value){
     uint32_t index_register = RPI3_GPIO_BANK(pin);
     uint32_t shift=RPI3_GPIO_SHIFT(pin);
     vaddr_t base_addr_GPPUDCLK=get_base_address_GPPUDCLK(index_register);
     if(value==ASSERT_CLK){
        io_setbits32(base_addr_GPPUDCLK,BIT(shift));
     }else{
        io_clrbits32(base_addr_GPPUDCLK,BIT(shift));
     }
}

/**
 * @param[in] updown `true` pull up, `false` pull down
 */
static void rpi3_ctrl_gpio_pull_up_down(gpio_t pin, bool updown){
    if(updown){
        rpi3_set_GPPUD(GPPUD_ENABLE_PULL_UP_CTRL);
        udelay(5); // 5us, 150 cycles?
        rpi3_set_GPPUDCLK(pin,ASSERT_CLK);
    }else{
        rpi3_set_GPPUD(GPPUD_ENABLE_PULL_DOWN_CTRL);
        udelay(5); // 5us, 150 cycles?
        rpi3_set_GPPUDCLK(pin,ASSERT_CLK);
    }
    udelay(5);
    rpi3_set_GPPUD(GPPUD_OFF);
    udelay(5);
    rpi3_set_GPPUDCLK(pin,NO_EFFECT);
    udelay(5);
}

int gpio_init(gpio_t pin, gpio_mode_t mode){
    if(__builtin_expect(pin>RPI3_MAX_PIN_NUM,0)) return -1;
    switch (mode)
    {
    case GPIO_IN:
        rpi3_set_gpio_pin_function(pin,GPIO_FSEL_INPT);
        break;
    case GPIO_OUT:
        rpi3_set_gpio_pin_function(pin,GPIO_FSEL_OUTP);
        break;
    case GPIO_IN_PU:
        rpi3_set_gpio_pin_function(pin,GPIO_FSEL_INPT);
        rpi3_ctrl_gpio_pull_up_down(pin,true);
        break;
    case GPIO_IN_PD:
        rpi3_set_gpio_pin_function(pin,GPIO_FSEL_INPT);
        rpi3_ctrl_gpio_pull_up_down(pin,false);
        break;
    case GPIO_OD:
        DMSG("Rpi3 has no support on gpio open drain without pull-up or pull-down resistor, use output push-pull");
        rpi3_set_gpio_pin_function(pin,GPIO_FSEL_OUTP);
        break;
    case GPIO_OD_PU:
        rpi3_set_gpio_pin_function(pin,GPIO_FSEL_OUTP);
        rpi3_ctrl_gpio_pull_up_down(pin,true);
        break;
    default:
        DMSG("error: Unrecognized gpio mode on rpi3");
        return -1;
    }
    return 0;
}

int gpio_set(gpio_t pin,gpio_level_t level){
    if(__builtin_expect(pin>RPI3_MAX_PIN_NUM,0)) return -1;
    rpi3_set_gpio_pin_value(pin,level);
    return 0;
}

int gpio_get(gpio_t pin){
    if(__builtin_expect(pin>RPI3_MAX_PIN_NUM,0)) return -1;
    return (int)rpi3_get_gpio_pin_value(pin);
}

void gpio_fsel(gpio_t pin, Rpi3_FunctionSelect_GPFSEL mode){
    if(__builtin_expect(pin>RPI3_MAX_PIN_NUM,0)) return;
    rpi3_set_gpio_pin_function(pin,mode);
}


static TEE_Result rpi3_gpio_init(void){
    // phys to virt secure
    if(cpu_mmu_enabled()){
        gpio_virt_base=(vaddr_t)phys_to_virt_io((paddr_t)GPIO_BASE,sizeof(struct GpioRegister));
        DMSG("rpi3 map gpio phys addr to secure virt addr, %p",gpio_virt_base);
    }else{
        DMSG("rpi3 map gpio phys addr to secure virt addr");
        gpio_virt_base=(vaddr_t)GPIO_BASE;
    }
    gpioRegister=(struct GpioRegister *)gpio_virt_base;
    return TEE_SUCCESS;
}

// init driver
driver_init(rpi3_gpio_init);
