/**
 * @brief DFRobot TEL0157 gnss driver
 * GNSS Positioning Module
 * refer to https://gitee.com/dfrobot/DFRobot_GNSS 
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#ifndef _TEL0157_H
#define _TEL0157_H

#include <periph/i2c.h>
#include "pta_tel0157.h"


/**
 * @brief set GNSS mode
 */
typedef enum {
  eGPS=1,
  eBeiDou,
  eGPS_BeiDou,
  eGLONASS,
  eGPS_GLONASS,
  eBeiDou_GLONASS,
  eGPS_BeiDou_GLONASS,
}tel0157_gnss_mode_t;

/**
 * @brief tel0157 device type
 */
typedef struct{
    i2c_t i2c;
    uint8_t addr;
    uint8_t gnss_mode;
    uint8_t rgb; // >0 rgb on, =0 rgb off
}tel0157_t;

TEE_Result tel0157_init(const tel0157_t * dev);
TEE_Result tel0157_deinit(const tel0157_t * dev);
TEE_Result tel0157_get_gnss_len(const tel0157_t * dev, uint16_t * len);
// should malloc all_gnss_buf len with tel0157_get_gnss_len
TEE_Result tel0157_get_all_gnss(const tel0157_t * dev, uint8_t * all_gnss_buf, uint16_t len);
// TEE_Result tel0157_get_gnss_data(const tel0157_t * dev, tel0157_time_t * utc_time, tel0157_lon_lat_t * lon_lat, tel0157_gnss_mode_t * mode);
TEE_Result tel0157_get_utc_time(const tel0157_t * dev, tel0157_time_t * utc_time);
TEE_Result tel0157_get_lon(const tel0157_t * dev, tel0157_lon_lat_t * lon_lat);
TEE_Result tel0157_get_lat(const tel0157_t * dev, tel0157_lon_lat_t * lon_lat);
TEE_Result tel0157_get_alt(const tel0157_t * dev, uint8_t alt[3]);
TEE_Result tel0157_get_sog(const tel0157_t * dev, uint8_t sog[3]);
TEE_Result tel0157_get_cog(const tel0157_t * dev, uint8_t cog[3]);
TEE_Result tel0157_get_num_sat_used(const tel0157_t * dev, uint8_t * num);
// TEE_Result tel0157_set_gnss_mode(const tel0157_t * dev, uint8_t mode);
TEE_Result tel0157_get_gnss_mode(const tel0157_t * dev, uint8_t * mode);

#endif