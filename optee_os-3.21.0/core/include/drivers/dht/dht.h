/**
 * @brief driver for dht
 * This driver was developed with reference to RIOT dht driver
 * https://github.com/RIOT-OS/RIOT/blob/master/drivers/include/dht.h
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */

#ifndef _DHT_H
#define _DHT_H

#include "periph/gpio.h"
#include <tee_api_types.h>

/**
 * @brief DHT return codes
 */
enum
{
    DHT_OK = TEE_SUCCESS,
    DHT_NOCSUM = TEE_ERROR_BAD_FORMAT,
    DHT_TIMEOUT = TEE_ERROR_TIMEOUT,
};

/**
 * @brief   Data type for storing DHT sensor readings
 */
typedef struct
{
    uint16_t humidity;    /**< relative humidity in deci-percent */
    uint16_t temperature; /**< temperature in deci-Celsius */
} dht_data_t;

/**
 * @brief   Device type of the DHT device
 */
typedef enum
{
    DHT11 = 0,      /**< Older DHT11 variants with either 1 °C or
                     *  0.1 °C resolution */
    DHT11_2022 = 1, /**< New DHT11 variant with 0.01 °C resolution */
    DHT22 = 2,      /**< DHT22 device identifier */
    DHT21 = DHT22,  /**< DHT21 device identifier */
    AM2301 = DHT22, /**< AM2301 device identifier */
} dht_type_t;

/**
 * @brief   Configuration parameters for DHT devices
 */
typedef struct
{
    gpio_t pin;          /**< GPIO pin of the device's data pin */
    dht_type_t type;     /**< type of the DHT device */
    gpio_mode_t in_mode; /**< input pin configuration, with or without pull
                          *   resistor */
} dht_params_t;

/**
 * @brief   Device descriptor for DHT sensor devices
 */
typedef struct
{
    dht_params_t params; /**< Device parameters */
    dht_data_t last_val; /**< Values of the last measurement */
} dht_t;

/**
 * @brief   Initialize a new DHT device
 *
 * @param[out] dev      device descriptor of a DHT device
 * @param[in]  params   configuration parameters
 *
 * @retval  `TEE_SUCCESS`               Success
 * @retval  `TEE_ERROR_BAD_STATE`        A low level on the input after the sensor's startup
 *                                      time indicates that either no sensor or pull-up
 *                                      resistor is connected, or the sensor is physically
 *                                      poorly connected or powered.
 * @retval `TEE_ERROR_ITEM_NOT_FOUND`   The sensor did not respond to the transmission of a
 *                                      start signal. Likely there were a pull-up resistor but
 *                                      no sensor connected on the data line.
 * @retval `TEE_ERROR_BAD_PARAMETES`    bad parameters
 */
TEE_Result dht_init(dht_t *dev, const dht_params_t *params);

/**
 * @brief   get a new temperature and/or humidity value from the device
 *
 * @note    if reading fails or checksum is invalid, no new values will be
 *          written into the result values
 *
 * @param[in]  dev      device descriptor of a DHT device
 * @param[out] temp     temperature value [in °C * 10^-1],
 *                      may be NULL if not needed
 * @param[out] hum      relative humidity value [in percent * 10^-1],
 *                      may be NULL if not needed
 *
 * @retval `TEE_SUCCESS`                Success
 * @retval `TEE_ERROR_ITEM_NOT_FOUND`   The sensor did not respond to the transmission of a
 *                                      start signal. Likely the RESPAWN_TIMEOUT is
 *                                      insufficient.
 * @retval `TEE_ERROR_BAD_FORMAT`       The received and the expected checksum didn't match.
 * @retval `TEE_ERROR_NOT_SUPPORTED`    Unable to parse the received data. Likely the data
 *                                      format is not implemented.
 * @retval `TEE_ERROR_BAD_PARAMETERS`   Temperature low byte >= 10. Likely misconfigured
 *                                      evice type (DHT11_2022).
 */
TEE_Result dht_read(dht_t *dev, int16_t *temp, int16_t *hum);

#endif
