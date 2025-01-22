/**
 * @brief pta atk301 service
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#ifndef __PTA_ATK301_SERVICE_H
#define __PTA_ATK301_SERVICE_H

#define ATK301_DEFAULT_ADDR       (0x27)
#define ATK301_FINGERPRINT_IMAGE_DATA_LEN 12800

// 6f66c838-ff96-490c-8673-c6d4fbcdec46
#define PTA_ATK301_SERVICE_UUID    \
{   \
    0x6f66c838, 0xff96, 0x490c, \
    {   \
        0x86, 0x73, 0xc6, 0xd4, 0xfb, 0xcd, 0xec, 0x46 \
    }   \
}   \

#define PTA_CMD_ATK301_INIT 0
#define PTA_CMD_ATK301_GET_FINGERPRINT_IMAGE 1
#define PTA_CMD_ATK301_UPLOAD_FINGERPRINT_IMAGE 2

#endif