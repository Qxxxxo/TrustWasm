/**
 * @brief pta bh1750fvi service
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#ifndef __PTA_BH1750FVI_H
#define __PTA_BH1750FVI_H
// 0f87a7fe-39cd-4f47-a129-2ef7d7b0d6a6
#define PTA_BH1750FVI_SERVICE_UUID                         \
    {                                                      \
        0x0f87a7fe, 0x39cd, 0x4f47,                        \
        {                                                  \
            0x81, 0xe8, 0x02, 0x00, 0xa6, 0x8e, 0xe3, 0x46 \
        }                                                  \
    }

/**
 * BH1750FVI INIT
 * [in] params[0].value.a: i2c number
 * [in] params[0].value.b: i2c addr
 */
#define PTA_CMD_BH1750FVI_INIT  0

/**
 * BH1750FVI sample
 * [in] params[0].value.a: i2c number
 * [in] params[0].value.b: i2c addr
 * [out] params[0].value.a: lux value
 */
#define PTA_CMD_BH1750FVI_SAMPLE   1

#endif