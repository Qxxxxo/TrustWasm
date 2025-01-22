/**
 * @brief atk301 driver
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#ifndef _ATK301_H
#define _ATK301_H

#include "periph/i2c.h"
#include "pta_atk301.h"

typedef struct {
    i2c_t i2c;
    uint8_t addr;
} atk301_t;

TEE_Result atk301_init(const atk301_t * dev);
TEE_Result atk301_get_fingerprint_image(const atk301_t * dev);
TEE_Result atk301_upload_fingerprint_image(const atk301_t * dev, uint8_t data[ATK301_FINGERPRINT_IMAGE_DATA_LEN]);

#endif