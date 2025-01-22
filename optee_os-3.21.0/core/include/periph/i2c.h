/**
 * @brief driver for i2c
 * This header file is based on RIOT i2c header file.
 * https://github.com/RIOT-OS/RIOT/blob/master/drivers/include/periph/i2c.h
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#ifndef _PERIPH_I2C_H
#define _PERIPH_I2C_H
#include <stdint.h>
#include <tee_api_types.h>

/**
 * @brief   Default i2c_t type definition
 * @{
 */
#ifndef HAVE_I2C_T
typedef uint_fast8_t i2c_t;
#endif
/**  @} */


#ifndef HAVE_I2C_SPEED_T
typedef enum {
    I2C_SPEED_LOW = 0,      /**< low speed mode:     ~10 kbit/s */
    I2C_SPEED_NORMAL,       /**< normal mode:       ~100 kbit/s */
    I2C_SPEED_FAST,         /**< fast mode:         ~400 kbit/s */
    I2C_SPEED_FAST_PLUS,    /**< fast plus mode:   ~1000 kbit/s */
    I2C_SPEED_HIGH,         /**< high speed mode:  ~3400 kbit/s */
} i2c_speed_t;
#endif


#ifndef HAVE_I2C_FLAGS_T
/**
 * @brief   I2C transfer flags
 */
typedef enum {
    I2C_ADDR10  = 0x01,     /**< use 10-bit device addressing */
    I2C_REG16   = 0x02,     /**< use 16-bit register addressing, big-endian */
    I2C_NOSTOP  = 0x04,     /**< do not issue a STOP condition after transfer */
    I2C_NOSTART = 0x08,     /**< skip START sequence, ignores address field */
    I2C_CONT    = 0x10,     /**< continue last, don't clear fifo */
} i2c_flags_t;
#endif


/**
 * @brief   Get mutually exclusive access to the given I2C bus
 *
 * In case the I2C device is busy, this function will block until the bus is
 * free again.
 *
 * @param[in] dev           I2C device to access
 */
TEE_Result i2c_acquire(i2c_t dev);

/**
 * @brief   Release the given I2C device to be used by others
 *
 * @param[in] dev           I2C device to release
 */
TEE_Result i2c_release(i2c_t dev);


/**
 * @brief   Convenience function for reading one byte from a given register
 *          address
 *
 * @note    This function is using a repeated start sequence for reading from
 *          the specified register address.
 *
 * @pre     i2c_acquire must be called before accessing the bus
 *
 * @param[in]  dev          I2C peripheral device
 * @param[in]  reg          register address to read from (8- or 16-bit,
 *                          right-aligned)
 * @param[in]  addr         7-bit or 10-bit device address (right-aligned)
 * @param[out] data         memory location to store received data
 * @param[in]  flags        optional flags (see i2c_flags_t)
 *
 * @return                  `TEE_SUCCESS` When success \\
 * @return                  `TEE_ERROR_NO_DATA` When slave device doesn't ACK the byte \\
 * @return                  `TEE_ERROR_ITEM_NOT_FOUND` When no devices respond on the address sent on the bus \\
 * @return                  `TEE_ERROR_TIMEOUT`  When timeout occurs before device's response \\
 * @return                  `TEE_ERROR_BAD_PARAMETERS` When an invalid argument is given \\
 * @return                  `TEE_ERROR_NOT_SUPPORTED` When MCU driver doesn't support the flag operation \\
 * @return                  `TEE_ERROR_BAD_STATE` When a lost bus arbitration occurs \\
 */
TEE_Result i2c_read_reg(i2c_t dev, uint16_t addr, uint16_t reg,
                 void *data, uint8_t flags);

/**
 * @brief   Convenience function for reading several bytes from a given
 *          register address
 *
 * @note    This function is using a repeated start sequence for reading from
 *          the specified register address.
 *
 * @pre     i2c_acquire must be called before accessing the bus
 *
 * @param[in]  dev          I2C peripheral device
 * @param[in]  reg          register address to read from (8- or 16-bit,
 *                          right-aligned)
 * @param[in]  addr         7-bit or 10-bit device address (right-aligned)
 * @param[out] data         memory location to store received data
 * @param[in]  len          the number of bytes to read into data
 * @param[in]  flags        optional flags (see i2c_flags_t)
 *
 * @return                  `TEE_SUCCESS` When success \\
 * @return                  `TEE_ERROR_NO_DATA` When slave device doesn't ACK the byte \\
 * @return                  `TEE_ERROR_ITEM_NOT_FOUND` When no devices respond on the address sent on the bus \\
 * @return                  `TEE_ERROR_TIMEOUT`  When timeout occurs before device's response \\
 * @return                  `TEE_ERROR_BAD_PARAMETERS` When an invalid argument is given \\
 * @return                  `TEE_ERROR_NOT_SUPPORTED` When MCU driver doesn't support the flag operation \\
 * @return                  `TEE_ERROR_BAD_STATE` When a lost bus arbitration occurs \\
 */
TEE_Result i2c_read_regs(i2c_t dev, uint16_t addr, uint16_t reg,
                  void *data, uint16_t len, uint8_t flags);

/**
 * @brief   Convenience function for reading one byte from a device
 *
 * @note    This function is using a repeated start sequence for reading from
 *          the specified register address.
 *
 * @pre     i2c_acquire must be called before accessing the bus
 *
 * @param[in]  dev          I2C peripheral device
 * @param[in]  addr         7-bit or 10-bit device address (right-aligned)
 * @param[out] data         memory location to store received data
 * @param[in]  flags        optional flags (see i2c_flags_t)
 *
 * @return                  `TEE_SUCCESS` When success \\
 * @return                  `TEE_ERROR_NO_DATA` When slave device doesn't ACK the byte \\
 * @return                  `TEE_ERROR_ITEM_NOT_FOUND` When no devices respond on the address sent on the bus \\
 * @return                  `TEE_ERROR_TIMEOUT`  When timeout occurs before device's response \\
 * @return                  `TEE_ERROR_BAD_PARAMETERS` When an invalid argument is given \\
 * @return                  `TEE_ERROR_NOT_SUPPORTED` When MCU driver doesn't support the flag operation \\
 * @return                  `TEE_ERROR_BAD_STATE` When a lost bus arbitration occurs \\
 */

TEE_Result i2c_read_byte(i2c_t dev, uint16_t addr, void *data, uint8_t flags);

/**
 * @brief   Convenience function for reading bytes from a device
 *
 * @note    This function is using a repeated start sequence for reading from
 *          the specified register address.
 *
 * @pre     i2c_acquire must be called before accessing the bus
 *
 * @param[in]  dev          I2C peripheral device
 * @param[in]  addr         7-bit or 10-bit device address (right-aligned)
 * @param[out] data         memory location to store received data
 * @param[in]  len          the number of bytes to read into data
 * @param[in]  flags        optional flags (see i2c_flags_t)
 *
 * @return                  `TEE_SUCCESS` When success \\
 * @return                  `TEE_ERROR_NO_DATA` When slave device doesn't ACK the byte \\
 * @return                  `TEE_ERROR_ITEM_NOT_FOUND` When no devices respond on the address sent on the bus \\
 * @return                  `TEE_ERROR_TIMEOUT`  When timeout occurs before device's response \\
 * @return                  `TEE_ERROR_BAD_PARAMETERS` When an invalid argument is given \\
 * @return                  `TEE_ERROR_NOT_SUPPORTED` When MCU driver doesn't support the flag operation \\
 * @return                  `TEE_ERROR_BAD_STATE` When a lost bus arbitration occurs \\
 */

TEE_Result i2c_read_bytes(i2c_t dev, uint16_t addr,
                   void *data, uint16_t len, uint8_t flags);

/**
 * @brief   Convenience function for writing a single byte onto the bus
 *
 * @pre     i2c_acquire must be called before accessing the bus
 *
 * @param[in] dev           I2C peripheral device
 * @param[in] addr          7-bit or 10-bit device address (right-aligned)
 * @param[in] data          byte to write to the device
 * @param[in] flags         optional flags (see i2c_flags_t)
 *
 * @return                  `TEE_SUCCESS` When success \\
 * @return                  `TEE_ERROR_NO_DATA` When slave device doesn't ACK the byte \\
 * @return                  `TEE_ERROR_ITEM_NOT_FOUND` When no devices respond on the address sent on the bus \\
 * @return                  `TEE_ERROR_TIMEOUT`  When timeout occurs before device's response \\
 * @return                  `TEE_ERROR_BAD_PARAMETERS` When an invalid argument is given \\
 * @return                  `TEE_ERROR_NOT_SUPPORTED` When MCU driver doesn't support the flag operation \\
 * @return                  `TEE_ERROR_BAD_STATE` When a lost bus arbitration occurs \\
 */
TEE_Result i2c_write_byte(i2c_t dev, uint16_t addr, uint8_t data, uint8_t flags);

/**
 * @brief   Convenience function for writing several bytes onto the bus
 *
 * @pre     i2c_acquire must be called before accessing the bus
 *
 * @param[in] dev           I2C peripheral device
 * @param[in] addr          7-bit or 10-bit device address (right-aligned)
 * @param[in] data          array holding the bytes to write to the device
 * @param[in] len           the number of bytes to write
 * @param[in] flags         optional flags (see i2c_flags_t)
 *
 * @return                  `TEE_SUCCESS` When success \\
 * @return                  `TEE_ERROR_NO_DATA` When slave device doesn't ACK the byte \\
 * @return                  `TEE_ERROR_ITEM_NOT_FOUND` When no devices respond on the address sent on the bus \\
 * @return                  `TEE_ERROR_TIMEOUT`  When timeout occurs before device's response \\
 * @return                  `TEE_ERROR_BAD_PARAMETERS` When an invalid argument is given \\
 * @return                  `TEE_ERROR_NOT_SUPPORTED` When MCU driver doesn't support the flag operation \\
 * @return                  `TEE_ERROR_BAD_STATE` When a lost bus arbitration occurs \\
 */
TEE_Result i2c_write_bytes(i2c_t dev, uint16_t addr, const void *data,
                    uint16_t len, uint8_t flags);

/**
 * @brief   Convenience function for writing one byte to a given
 *          register address
 *
 * @note    This function is using a continuous sequence for writing to the
 *          specified register address. It first writes the register then data.
 *
 * @pre     i2c_acquire must be called before accessing the bus
 *
 * @param[in]  dev          I2C peripheral device
 * @param[in]  reg          register address to read from (8- or 16-bit,
 *                          right-aligned)
 * @param[in]  addr         7-bit or 10-bit device address (right-aligned)
 * @param[in]  data         byte to write
 * @param[in]  flags        optional flags (see i2c_flags_t)
 *
 * @return                  `TEE_SUCCESS` When success \\
 * @return                  `TEE_ERROR_NO_DATA` When slave device doesn't ACK the byte \\
 * @return                  `TEE_ERROR_ITEM_NOT_FOUND` When no devices respond on the address sent on the bus \\
 * @return                  `TEE_ERROR_TIMEOUT`  When timeout occurs before device's response \\
 * @return                  `TEE_ERROR_BAD_PARAMETERS` When an invalid argument is given \\
 * @return                  `TEE_ERROR_NOT_SUPPORTED` When MCU driver doesn't support the flag operation \\
 * @return                  `TEE_ERROR_BAD_STATE` When a lost bus arbitration occurs \\
 */
TEE_Result i2c_write_reg(i2c_t dev, uint16_t addr, uint16_t reg,
                  uint8_t data, uint8_t flags);

/**
 * @brief   Convenience function for writing data to a given register address
 *
 * @note    This function is using a continuous sequence for writing to the
 *          specified register address. It first writes the register then data.
 *
 * @pre     i2c_acquire must be called before accessing the bus
 *
 * @param[in]  dev          I2C peripheral device
 * @param[in]  reg          register address to read from (8- or 16-bit,
 *                          right-aligned)
 * @param[in]  addr         7-bit or 10-bit device address (right-aligned)
 * @param[out] data         memory location to store received data
 * @param[in]  len          the number of bytes to write
 * @param[in]  flags        optional flags (see i2c_flags_t)
 *
 * @return                  `TEE_SUCCESS` When success \\
 * @return                  `TEE_ERROR_NO_DATA` When slave device doesn't ACK the byte \\
 * @return                  `TEE_ERROR_ITEM_NOT_FOUND` When no devices respond on the address sent on the bus \\
 * @return                  `TEE_ERROR_TIMEOUT`  When timeout occurs before device's response \\
 * @return                  `TEE_ERROR_BAD_PARAMETERS` When an invalid argument is given \\
 * @return                  `TEE_ERROR_NOT_SUPPORTED` When MCU driver doesn't support the flag operation \\
 * @return                  `TEE_ERROR_BAD_STATE` When a lost bus arbitration occurs \\
 */
TEE_Result i2c_write_regs(i2c_t dev, uint16_t addr, uint16_t reg,
                  const void *data, uint16_t len, uint8_t flags);


TEE_Result i2c_write_with_prefix(i2c_t dev, uint16_t addr, const void *prefix, uint16_t prefix_len, const void *data, uint16_t len, uint8_t flags);

/**
 * @brief Set i2c frequency
 * 
 * 
 * @param[in] dev   I2C peripheral device
 * @param[in] freq  frequency option
 * 
 * @return  `TEE_SUCCESS`   When success \\
 * @return  `TEE_ERROR_BAD_PARAMETERS` When frequency option is invalid \\
 * @return  `TEE_ERROR_NOT_SUPPORT` When frequency option not supported \\
 */
TEE_Result i2c_set_frequency(i2c_t dev, i2c_speed_t freq);


#endif