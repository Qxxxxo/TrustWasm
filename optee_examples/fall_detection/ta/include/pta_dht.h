/**
 * @brief pta dht service
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#ifndef __PTA_DHT_H
#define __PTA_DHT_H

#define PTA_DHT_SERVICE_UUID                               \
    {                                                      \
        0xcb0f6a01, 0x31e6, 0x452e,                        \
        {                                                  \
            0x81, 0xe8, 0x02, 0x00, 0xa6, 0x8e, 0xe3, 0x46 \
        }                                                  \
    }

/**
 * DHT INIT 
 * [in] value[0].a: gpio pin number
 * [in] value[0].b: dht type, see dht_type_t
 */
#define PTA_CMD_DHT_INIT    0

/**
 * DHT READ
 * [in]     value[0].a: gpio pin number
 * [in]     value[0].b: dht type, see dht_type_t
 * [out]    value[1].a: temperature
 * [out]    value[1].b: humidity
 */
#define PTA_CMD_DHT_READ    1

#define PTA_CMD_TEST_GET_SYS_TIME_OVERHEAD  2

#endif

