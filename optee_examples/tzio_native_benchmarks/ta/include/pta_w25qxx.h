/**
 * @brief pta w25qxx service
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#ifndef __PTA_W25QXX_SERVICE_H
#define __PTA_W25QXX_SERVICE_H
// d0ab2107-41e7-49bf-8223-2b4385393ef1
#define PTA_W25QXX_SERVICE_UUID                            \
    {                                                      \
        0xd0ab2107, 0x41e7, 0x49bf,                        \
        {                                                  \
            0x82, 0x23, 0x2b, 0x43, 0x85, 0x39, 0x3e, 0xf1 \
        }                                                  \
    }

/**
 * W25QXX INIT
 * [in] params[0].value.a: spi number
 * [in] params[0].value.b: spi cs
 */
#define PTA_CMD_W25QXX_INIT 0

/**
 * W25QXX READ ID
 * [in] params[0].value.a: spi number
 * [in] params[0].value.b: spi cs
 * [out] params[0].value.a: id
 */
#define PTA_CMD_W25QXX_READ_ID  1

/**
 * W25QXX READ DATA
 * [in] params[0].value.a: spi number
 * [in] params[0].value.b: spi cs
 * [in] params[1].value.a: read addr
 * [in/out] params[2].memref.buffer: output data buffer
 * [in/out] params[2].memref.len: output data len 
 */
#define PTA_CMD_W25QXX_READ_DATA    2

/**
 * W25QXX PAGE PROGRAM
 * [in] params[0].value.a: spi number
 * [in] params[0].value.b: spi cs
 * [in] params[1].value.a: write addr
 * [in] params[2].memref.buffer: input data buffer
 * [in] params[2].memref.len: input data len 
 */
#define PTA_CMD_W25QXX_PAGE_PROGRAM 3

/**
 * W25QXX SECTOR ERASE
 * [in] params[0].value.a: spi number
 * [in] params[0].value.b: spi cs
 * [in] params[1].value.a: erase addr
 */
#define PTA_CMD_W25QXX_SECTOR_ERASE 4

/**
 * W25QXX CHIP ERASE
 * [in] params[0].value.a: spi number
 * [in] params[0].value.b: spi cs
 */
#define PTA_CMD_W25QXX_CHIP_ERASE   5

#endif
