#ifndef _PERIPH_GPIO_H
#define _PERIPH_GPIO_H

#ifndef HAVE_GPIO_T
/**
 * @brief   GPIO type identifier
 */
typedef unsigned int gpio_t;
#endif

/**
 * @brief   Available pib modes
 */
#ifndef HAVE_GPIO_MODE_T
typedef enum {
    GPIO_IN=0,    // input without pull resistor
    GPIO_OUT,   // output in push-pull mode
    GPIO_IN_PU, // input with pull-up resistor
    GPIO_IN_PD, // input with pull-down resistor
    GPIO_OD,    // output in open-drain mode without pull resistor
    GPIO_OD_PU, // output in open-drain mode with pull resistor
} gpio_mode_t;
#endif

/**
 * @brief gpio level
 */
typedef enum {
    GPIO_LOW_LEVEL=0,
    GPIO_HIGH_LEVEL
} gpio_level_t;

/**
 * @brief Initialize given pin as input or ouput
 * @return 0 on success
 * @return -1 on error
 */
int gpio_init(gpio_t pin,gpio_mode_t mode);

/**
 * @brief Set given pin gpio level
 * @return 0 on success
 * @return -1 on error
 */
int gpio_set(gpio_t pin,gpio_level_t level);

/**
 * @brief Get given pin gpio level
 * @return 0 on low level
 * @return 1 on high level
 * @return -1 on error
 */
int gpio_get(gpio_t pin);


#endif 