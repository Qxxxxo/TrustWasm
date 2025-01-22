/**
 * @brief w25qxx driver
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */

#ifndef _W25QXX_H
#define _W25QXX_H

#include <periph/spi.h>
#include <tee_api_types.h>

#define W25Q80 	0xEF13 	
#define W25Q16 	0xEF14
#define W25Q32 	0xEF15
#define W25Q64 	0xEF16
#define W25Q128	0xEF17
#define W25Q256 0xEF18

// Commands
#define W25QXX_CMD_READ_ID          0x9F // read ID
#define W25QXX_CMD_READ_STATUS      0x05 // read status register
#define W25QXX_CMD_WRITE_STATUS     0x01 // write status register
#define W25QXX_CMD_WRITE_ENABLE     0x06 // write enable
#define W25QXX_CMD_WRITE_DISABLE    0x04 // write disable
#define W25QXX_CMD_PAGE_PROGRAM     0x02 // page program
#define W25QXX_CMD_READ_DATA        0x03 // read data
#define W25QXX_CMD_SECTOR_ERASE     0x20 // sector erase
#define W25QXX_CMD_CHIP_ERASE       0xC7 // chip erase

#define W25QXX_STATUS_BUSY            0x01  // storage busy
#define W25QXX_STATUS_WEL             0x02  // write enable

TEE_Result w25qxx_init(spi_t bus, spi_cs_t cs);
// id len is 2
TEE_Result w25qxx_read_id(spi_t bus, spi_cs_t cs, uint16_t * id);
TEE_Result w25qxx_read_status(spi_t bus, spi_cs_t cs, uint8_t * status);
TEE_Result w25qxx_write_status(spi_t bus, spi_cs_t cs, uint8_t status);
TEE_Result w25qxx_wait_until_ready(spi_t bus, spi_cs_t cs,uint32_t timeout);
TEE_Result w25qxx_write_enable(spi_t bus, spi_cs_t cs);
TEE_Result w25qxx_write_disable(spi_t bus, spi_cs_t cs);
// void w25qxx_power_down(spi_t bus, spi_cs_t cs);
// void w25qxx_wake_up(spi_t bus, spi_cs_t cs);
TEE_Result w25qxx_read_data(spi_t bus, spi_cs_t cs, uint8_t * buf, uint32_t read_addr, uint32_t len);
// void w25qxx_write_data(spi_t bus, spi_cs_t cs, const uint8_t * buf, uint32_t write_addr, uint32_t len);
TEE_Result w25qxx_page_program(spi_t bus, spi_cs_t cs, const uint8_t * buf, uint32_t write_addr,  uint32_t len);
TEE_Result w25qxx_sector_erase(spi_t bus, spi_cs_t cs, uint32_t erase_addr);
TEE_Result w25qxx_chip_erase(spi_t bus, spi_cs_t cs);

#endif