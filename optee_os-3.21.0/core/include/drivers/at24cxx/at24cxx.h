/**
 * @brief at24cxx driver
 * @author XXXXXXXX
 */
#ifndef _AT24CXX_H
#define _AT24CXX_H

#include <periph/i2c.h>

typedef struct{
  i2c_t dev;
  uint8_t addr;
}at24cxx_t;

TEE_Result at24cxx_write(const at24cxx_t *dev, uint16_t addr, const uint8_t *buf, uint32_t len);
TEE_Result at24cxx_read(const at24cxx_t *dev, uint16_t addr, uint8_t *buf, uint32_t len);

#endif