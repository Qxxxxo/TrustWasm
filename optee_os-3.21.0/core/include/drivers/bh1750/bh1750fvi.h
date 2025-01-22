/**
 * @brief bh1750fvi driver
 * This header file is based on RIOT bh1750fvi.h
 * https://github.com/RIOT-OS/RIOT/blob/master/drivers/include/bh1750fvi.h
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#ifndef _BH1750FVI_H
#define _BH1750FVI_H

#include "periph/i2c.h"

#define BH1750FVI_ADDR_PIN_LOW          (0x23)      /**< ADDR pin := 0 */
#define BH1750FVI_ADDR_PIN_HIGH         (0x5c)      /**< ADDR pin := 1 */

/**
 * @brief   Default address of BH1750FVI sensors
 */
#define BH1750FVI_DEFAULT_ADDR          BH1750FVI_ADDR_PIN_LOW

/**
 * @brief   Maximum I2C bus speed to use with the device
 */
#define BH1750FVI_I2C_MAX_CLK           I2C_SPEED_FAST

/**
 * @brief   Device descriptor for BH1570FVI devices
 */
typedef struct {
    i2c_t i2c;          /**< I2C bus the device is connected to */
    uint8_t addr;       /**< slave address of the device */
} bh1750fvi_t;

/**
 * @brief   Initialize the given BH1750FVI device
 *
 * @param[out] dev      device descriptor of the targeted device
 */
TEE_Result bh1750fvi_init(const bh1750fvi_t *dev);

/**
 * @brief   Read a ambient light value from the given device [in LUX]
 *
 * The result value is the measured ambient light intensity in LUX and ranges
 * from 0 to 54612. Taking one measurement takes ~120ms, so it takes this amount
 * of time until the function returns.
 *
 * @param[in] dev       device descriptor of the targeted device
 * 
 */
TEE_Result bh1750fvi_sample(const bh1750fvi_t *dev, uint16_t *lux);

#endif