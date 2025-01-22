/**
 * @brief pta spi service
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#ifndef __PTA_SPI_SERVICE_H
#define __PTA_SPI_SERVICE_H
// 777f715d-885d-40b9-8398-e386bf958080
#define PTA_SPI_SERVICE_UUID    \
    {   \
        0x777f715d, 0x885d, 0x40b9, \
        {   \
            0x83, 0x98, 0xe3, 0x86, 0xbf, 0x95, 0x80, 0x80 \
        }   \
    }

/**
 * SPI TRANSFER BYTE
 * [in] params[0].value.a (H 16): spi number
 * [in] params[0].value.a (L 16): spi cs
 * [in] params[0].value.b: contitnue last transfer, don't clear
 * [in] params[1].value.a: spi mode
 * [in] params[1].value.a: spi freq
 * [in] params[2].value.a: need write byte ?
 * [in] params[2].value.b: need read byte ?
 * [in] params[3].value.a: byte in
 * [out] params[0].value.a: byte out
 */
#define PTA_CMD_SPI_TRANSFER_BYTE   0

/**
 * SPI TRANSFER BYTES
 * [in] params[0].value.a (H 16): spi number
 * [in] params[0].value.a (L 16): spi cs
 * [in] params[0].value.b: contitnue last transfer, don't clear
 * [in] params[1].value.a: spi mode
 * [in] params[1].value.b: spi freq
 * [in] params[2].memref.buffer: bytes out buffer
 * [in] params[2].memref.size: bytes out len
 * [in/out] params[3].memref.buffer: bytes in buffer
 * [in/out] params[3].memref.size: bytes in len
 */
#define PTA_CMD_SPI_TRANSFER_BYTES  1

/**
 * SPI TRANSFER BYTES
 * [in] params[0].value.a (H 16): spi number
 * [in] params[0].value.a (L 16): spi cs
 * [in] params[0].value.b: contitnue last transfer, don't clear
 * [in] params[1].value.a: spi mode
 * [in] params[1].value.b: spi freq
 * [in] params[2].memref.buffer: bytes out buffer
 * [in] params[2].memref.size: bytes out len
 * [in/out] params[3].memref.buffer: bytes in buffer
 * [in/out] params[3].memref.size: bytes in len
 */
#define PTA_CMD_SPI_TRANSFER_DIFF_BYTES  2

#endif