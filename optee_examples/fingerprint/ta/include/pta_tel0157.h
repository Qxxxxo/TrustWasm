/**
 * @brief pta tel0157 service
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#ifndef __PTA_TEL0157_SERVICE_UUID
#define __PTA_TEL0157_SERVICE_UUID

#include <stdint.h>

/**
 * @brief store time and date obtained from GPS
 */
typedef struct{
    uint16_t year; 
    uint8_t month;
    uint8_t date;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
}tel0157_time_t;

/**
 * @brief store longitude and latitude
 */
typedef struct{
    uint32_t lonMMMMM;
    uint32_t latMMMMM;
    uint8_t lonDD;
    uint8_t lonMM;
    uint8_t latDD;
    uint8_t latMM;
    char lonDirection;
    char latDirection;
}tel0157_lon_lat_t;

#define TEL0157_DEVICE_ADDR 0x20

// a95d2941-150b-4163-9fcb-7e0a26b815d2
#define PTA_TEL0157_SERVICE_UUID                           \
    {                                                      \
        0xa95d2941, 0x150b, 0x4163,                        \
        {                                                  \
            0x9f, 0xcb, 0x7e, 0x0a, 0x26, 0xb8, 0x15, 0xd2 \
        }                                                  \
    }

/**
 * TEL0157 INIT
 * [in] params[0].value.a: i2c number
 * [in] params[0].value.b: i2c addr
 * [in] params[1].value.a: gnss mode
 * [in] params[1].value.b: rgb
 */
#define PTA_CMD_TEL0157_INIT    0

/**
 * TEL0157 INIT
 * [in] params[0].value.a: i2c number
 * [in] params[0].value.b: i2c addr
 */
#define PTA_CMD_TEL0157_DEINIT  1

/**
 * TEL0157 GET UTC TIME
 * [in] params[0].value.a: i2c number
 * [in] params[0].value.b: i2c addr
 * [out] params[0].value.a: year
 * [out] params[0].value.b: month
 * [out] params[1].value.a: date
 * [out] params[1].value.b: hour
 * [out] params[2].value.a: minute
 * [out] params[2].value.b: second
 * 
 */
#define PTA_CMD_TEL0157_GET_UTC_TIME  2

/**
 * TEL0157 GET LON
 * [in] params[0].value.a: i2c number
 * [in] params[0].value.b: i2c addr
 * [out] params[0].value.a: lonDDD
 * [out] params[0].value.b: lonMM
 * [out] params[1].value.a: lonMMMM
 * [out] params[1].value.b: lonDirection
 */
#define PTA_CMD_TEL0157_GET_LON  3

/**
 * TEL0157 GET LAT
 * [in] params[0].value.a: i2c number
 * [in] params[0].value.b: i2c addr
 * [out] params[0].value.a: latDDD
 * [out] params[0].value.b: latMM
 * [out] params[1].value.a: latMMMM
 * [out] params[1].value.b: latDirection
 * 
 */
#define PTA_CMD_TEL0157_GET_LAT  4

/**
 * TEL0157 GET ALT
 * [in] params[0].value.a: i2c number
 * [in] params[0].value.b: i2c addr
 * [in/out] params[1].memref.buffer: alt 
 * [in/out] params[1].memref.size: expect 3
 */
#define PTA_CMD_TEL0157_GET_ALT  5

/**
 * TEL0157 GET SOG
 * [in] params[0].value.a: i2c number
 * [in] params[0].value.b: i2c addr
 * [in/out] params[1].memref.buffer: sog 
 * [in/out] params[1].memref.size: expect 3
 */
#define PTA_CMD_TEL0157_GET_SOG  6

/**
 * TEL0157 GET COG
 * [in] params[0].value.a: i2c number
 * [in] params[0].value.b: i2c addr
 * [in/out] params[1].memref.buffer: cog
 * [in/out] params[1].memref.size: expect 3
 */
#define PTA_CMD_TEL0157_GET_COG  7

/**
 * TEL0157 GET NUM SAT USED
 * [in] params[0].value.a: i2c number
 * [in] params[0].value.b: i2c addr
 * [out] params[0].value.a: num sat used
 */
#define PTA_CMD_TEL0157_GET_NUM_SAT_USED  8

/**
 * TEL0157 GET GNSS MODE
 * [in] params[0].value.a: i2c number
 * [in] params[0].value.b: i2c addr
 * [out] params[0].value.a: gnss mode
 */
#define PTA_CMD_TEL0157_GET_GNSS_MODE  9

/**
 * TEL0157 GET GNSS LEN
 * [in] params[0].value.a: i2c number
 * [in] params[0].value.b: i2c addr
 * [out] params[0].value.a: gnss len
 */
#define PTA_CMD_TEL0157_GET_GNSS_LEN  10

/**
 * TEL0157 GET GNSS LEN
 * [in] params[0].value.a: i2c number
 * [in] params[0].value.b: i2c addr
 * [in/out] params[1].memref.buffer: gnss content
 * [in/out] params[1].memref.size: gnss len
 */
#define PTA_CMD_TEL0157_GET_ALL_GNSS  11

#endif