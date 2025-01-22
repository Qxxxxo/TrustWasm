/**
 * @brief some gpio function for rpi3/bcm2835
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#ifndef _RPI3_GPIO_H
#define _RPI3_GPIO_H

#include <periph/gpio.h>

typedef enum {
	GPIO_FSEL_INPT = 0x00,
	GPIO_FSEL_OUTP = 0x01,
    GPIO_FSEL_ALT0 = 0x04,
    GPIO_FSEL_ALT1 = 0x05,
    GPIO_FSEL_ALT2 = 0x06,
    GPIO_FSEL_ALT3 = 0x07,
    GPIO_FSEL_ALT4 = 0x03,
    GPIO_FSEL_ALT5 = 0x02,
    GPIO_FESL_MASK = 0x07
} Rpi3_FunctionSelect_GPFSEL;

void gpio_fsel(gpio_t pin, Rpi3_FunctionSelect_GPFSEL mode);

#endif