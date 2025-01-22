/**
 * @brief spi driver interface
 * This header file is based on RIOT spi header file.
 * https://github.com/RIOT-OS/RIOT/blob/master/drivers/include/periph/spi.h
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#ifndef _PERIPH_SPI_H
#define _PERIPH_SPI_H
#include <stdint.h>
#include <tee_api_types.h>
#include <periph/gpio.h>

/**
 * @brief   Default type for SPI devices
 */
#ifndef HAVE_SPI_T
typedef uint_fast8_t spi_t;
#endif

/**
 * @brief   Chip select pin type overlaps with gpio_t so it can be casted to
 *          this
 */
#ifndef HAVE_SPI_CS_T
typedef uint_fast8_t spi_cs_t;
#endif

/**
 * @brief   Available SPI modes, defining the configuration of clock polarity
 *          and clock phase
 *
 * RIOT is using the mode numbers as commonly defined by most vendors
 * (https://en.wikipedia.org/wiki/Serial_Peripheral_Interface_Bus#Mode_numbers):
 *
 * - MODE_0: CPOL=0, CPHA=0 - The first data bit is sampled by the receiver on
 *           the first SCK rising SCK edge (this mode is used most often).
 * - MODE_1: CPOL=0, CPHA=1 - The first data bit is sampled by the receiver on
 *           the second rising SCK edge.
 * - MODE_2: CPOL=1, CPHA=0 - The first data bit is sampled by the receiver on
 *           the first falling SCK edge.
 * - MODE_3: CPOL=1, CPHA=1 - The first data bit is sampled by the receiver on
 *           the second falling SCK edge.
 */
#ifndef HAVE_SPI_MODE_T
typedef enum {
    SPI_MODE_0 = 0,         /**< CPOL=0, CPHA=0 */
    SPI_MODE_1,             /**< CPOL=0, CPHA=1 */
    SPI_MODE_2,             /**< CPOL=1, CPHA=0 */
    SPI_MODE_3              /**< CPOL=1, CPHA=1 */
} spi_mode_t;
#endif

/**
 * @brief   Available SPI clock speeds
 *
 * The actual speed of the bus can vary to some extend, as the combination of
 * CPU clock and available prescaler values on certain platforms may not make
 * the exact values possible.
 */
#ifndef HAVE_SPI_CLK_T
typedef enum {
    SPI_CLK_100KHZ = 0,     /**< drive the SPI bus with 100KHz */
    SPI_CLK_400KHZ,         /**< drive the SPI bus with 400KHz */
    SPI_CLK_1MHZ,           /**< drive the SPI bus with 1MHz */
    SPI_CLK_5MHZ,           /**< drive the SPI bus with 5MHz */
    SPI_CLK_10MHZ,           /**< drive the SPI bus with 10MHz */
    SPI_CLK_40MHZ           /**< drive the SPI bus with 40MHz */
} spi_clk_t;
#endif

/**
 * @brief   Start a new SPI transaction
 *
 * Starting a new SPI transaction will get exclusive access to the SPI bus
 * and configure it according to the given values. If another SPI transaction
 * is active when this function is called, this function will block until the
 * other transaction is complete (spi_relase was called).
 *
 * @param[in]   bus     SPI device to access
 * @param[in]   cs      chip select pin/line to use, set to SPI_CS_UNDEF if chip
 *                      select should not be handled by the SPI driver
 * @param[in]   mode    mode to use for the new transaction
 * @param[in]   clk     bus clock speed to use for the transaction
 *
 * @pre     All parameters are valid and supported, otherwise an assertion blows
 *          up (if assertions are enabled).
 */
TEE_Result spi_acquire(spi_t bus, spi_cs_t cs, spi_mode_t mode, spi_clk_t clk);

/**
 * @brief   Finish an ongoing SPI transaction by releasing the given SPI bus
 *
 * After release, the given SPI bus should be fully powered down until acquired
 * again.
 *
 * @param[in]   bus     SPI device to release
 */
TEE_Result spi_release(spi_t bus);

/**
 * @brief Transfer one byte on the given SPI bus
 *
 * @param[in]   bus     SPI device to use
 * @param[in]   cs      chip select pin/line to use, set to SPI_CS_UNDEF if chip
 *                      select should not be handled by the SPI driver
 * @param[in]   cont    if true, keep device selected after transfer
 * @param[in]   out     byte to send out
 * @param[in]   in     byte received
 */
TEE_Result spi_transfer_byte(spi_t bus, spi_cs_t cs, bool cont, uint8_t out, uint8_t *in);

/**
 * @brief   Transfer a number bytes using the given SPI bus
 *
 * @param[in]   bus     SPI device to use
 * @param[in]   cs      chip select pin/line to use, set to SPI_CS_UNDEF if chip
 *                      select should not be handled by the SPI driver
 * @param[in]   cont    if true, keep device selected after transfer
 * @param[in]   out     buffer to send data from, set NULL if only receiving
 * @param[out]  in      buffer to read into, set NULL if only sending
 * @param[in]   len     number of bytes to transfer
 */
TEE_Result spi_transfer_bytes(spi_t bus, spi_cs_t cs, bool cont,
                        const void *out, void *in, size_t len);

TEE_Result spi_transfer_diff_bytes(spi_t bus, spi_cs_t cs, bool cont,
            const void *out, void *in, size_t out_len, size_t in_len);

/**
 * @brief   Transfer one byte to/from a given register address
 *
 * This function is a shortcut function for easier handling of SPI devices that
 * implement a register based access scheme.
 *
 * @param[in]   bus     SPI device to use
 * @param[in]   cs      chip select pin/line to use, set to SPI_CS_UNDEF if chip
 *                      select should not be handled by the SPI driver
 * @param[in]   reg     register address to transfer data to/from
 * @param[in]   out     byte to send
 * @param[in]   in      byte received
 */
TEE_Result spi_transfer_reg(spi_t bus, spi_cs_t cs, uint8_t reg, uint8_t out, uint8_t* in);

/**
 * @brief   Transfer a number of bytes to/from a given register address
 *
 * This function is a shortcut function for easier handling of SPI devices that
 * implement a register based access scheme.
 *
 * @param[in]   bus     SPI device to use
 * @param[in]   cs      chip select pin/line to use, set to SPI_CS_UNDEF if chip
 *                      select should not be handled by the SPI driver
 * @param[in]   reg     register address to transfer data to/from
 * @param[in]   out     buffer to send data from, set NULL if only receiving
 * @param[out]  in      buffer to read into, set NULL if only sending
 * @param[in]   len     number of bytes to transfer
 */
TEE_Result spi_transfer_regs(spi_t bus, spi_cs_t cs, uint8_t reg,
                       const void *out, void *in, size_t len);


#endif