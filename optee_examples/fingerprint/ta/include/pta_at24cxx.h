/**
 * @brief pta at24cxx service
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#ifndef __PTA_AT24CXX_SERVICE_H
#define __PTA_AT24CXX_SERVICE_H

// f79b45e0-0ac7-40f1-8dc9-cad2f9450933
#define PTA_AT24CXX_SERVICE_UUID    \
{   \
    0xf79b45e0, 0x0ac7, 0x40f1, \
    {   \
        0x8d, 0xc9, 0xca, 0xd2, 0xf9, 0x45, 0x09, 0x33 \
    }   \
}   \

/**
 * AT24CXX READ
 * [in] params[0].value.a: i2c number
 * [in] params[0].value.b: i2c addr
 * [in] params[1].value.a: read addr
 * [out] params[2].memref.buffer: read buffer
 * [out] params[2].memref.size: read len    
 */
#define PTA_CMD_AT24CXX_READ 0

/**
 * AT24CXX WRITE
 * [in] params[0].value.a: i2c number
 * [in] params[0].value.b: i2c addr
 * [in] params[1].value.a: write addr
 * [in] params[2].memref.buffer: write buffer
 * [in] params[2].memref.size: write len
 */
#define PTA_CMD_AT24CXX_WRITE 1

#endif
