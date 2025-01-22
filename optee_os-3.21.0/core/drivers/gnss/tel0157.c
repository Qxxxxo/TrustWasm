/**
 * @brief DFRobot TEL0157 gnss driver
 * GNSS Positioning Module
 * refer to https://gitee.com/dfrobot/DFRobot_GNSS 
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <drivers/gnss/tel0157.h>
#include <kernel/delay.h>
#include <trace.h>

#define TEL0157_I2C_YEAR_H 0
#define TEL0157_I2C_YEAR_L 1
#define TEL0157_I2C_MONTH 2
#define TEL0157_I2C_DATE  3
#define TEL0157_I2C_HOUR  4
#define TEL0157_I2C_MINUTE 5
#define TEL0157_I2C_SECOND 6
#define TEL0157_I2C_LAT_1 7
#define TEL0157_I2C_LAT_2 8
#define TEL0157_I2C_LAT_X_24 9
#define TEL0157_I2C_LAT_X_16 10
#define TEL0157_I2C_LAT_X_8  11
#define TEL0157_I2C_LON_DIS  12
#define TEL0157_I2C_LON_1 13
#define TEL0157_I2C_LON_2 14
#define TEL0157_I2C_LON_X_24 15
#define TEL0157_I2C_LON_X_16 16
#define TEL0157_I2C_LON_X_8  17
#define TEL0157_I2C_LAT_DIS  18
#define TEL0157_I2C_USE_STAR 19
#define TEL0157_I2C_ALT_H 20
#define TEL0157_I2C_ALT_L 21
#define TEL0157_I2C_ALT_X 22
#define TEL0157_I2C_SOG_H 23
#define TEL0157_I2C_SOG_L 24
#define TEL0157_I2C_SOG_X 25
#define TEL0157_I2C_COG_H 26
#define TEL0157_I2C_COG_L 27
#define TEL0157_I2C_COG_X 28

#define TEL0157_I2C_START_GET 29
#define TEL0157_I2C_ID 30
#define TEL0157_I2C_DATA_LEN_H 31
#define TEL0157_I2C_DATA_LEN_L 32
#define TEL0157_I2C_ALL_DATA 33

#define TEL0157_I2C_GNSS_MODE 34
#define TEL0157_I2C_SLEEP_MODE 35
#define TEL0157_I2C_RGB_MODE 36

#define TEL0157_I2C_FLAG  1
// #define TEL0157_UART_FLAG 2

#define TEL0157_ENABLE_POWER 0
#define TEL0157_DISABLE_POWER 1

#define TEL0157_RGB_ON 0x05
#define TEL0157_RGB_OFF 0x02
#define TEL0157_START_GET_CMD 0x55
#define TEL0157_GENERAL_DELAY_US    50000
#define TEL0157_GET_GNSS_LEN_DELAY_US    100000


static inline TEE_Result tel0157_enable_power_internal(const tel0157_t * dev){
    return i2c_write_reg(dev->i2c,dev->addr,TEL0157_I2C_SLEEP_MODE,TEL0157_ENABLE_POWER,0); 
} 

static inline TEE_Result tel0157_disable_power_internal(const tel0157_t * dev){
    return i2c_write_reg(dev->i2c,dev->addr,TEL0157_I2C_SLEEP_MODE,TEL0157_DISABLE_POWER,0); 
}

static inline TEE_Result  tel0157_set_gnss_mode_internal(const tel0157_t * dev){
    if(dev->gnss_mode>(uint8_t)eGPS_BeiDou_GLONASS){
        return TEE_ERROR_NOT_SUPPORTED;
    }
    return i2c_write_reg(dev->i2c,dev->addr,TEL0157_I2C_GNSS_MODE,dev->gnss_mode,0);
}

static inline TEE_Result tel0157_set_rgb_internal(const tel0157_t * dev){
    uint8_t rgb=TEL0157_RGB_OFF;
    if(dev->rgb>0){
        rgb=TEL0157_RGB_ON;
    }
    return i2c_write_reg(dev->i2c,dev->addr,TEL0157_I2C_RGB_MODE,rgb,0);
}

static TEE_Result tel0157_get_date_internal(const tel0157_t * dev, tel0157_time_t * utc_time){
    TEE_Result res;
    uint8_t date_buf[4]={0};
    res=i2c_read_regs(dev->i2c,dev->addr,TEL0157_I2C_YEAR_H,date_buf,4,0);
    if(res!=TEE_SUCCESS){
        return res;
    }
    utc_time->year=((uint16_t)date_buf[0]<<8) | date_buf[1];
    utc_time->month=date_buf[2];
    utc_time->date=date_buf[3];
    DMSG("%d %d %d",utc_time->year,utc_time->month, utc_time->date);
    return res;
}

static TEE_Result tel0157_get_utc_internal(const tel0157_t * dev, tel0157_time_t * utc_time){
    TEE_Result res;
    uint8_t utc_buf[3]={0};
    res=i2c_read_regs(dev->i2c,dev->addr,TEL0157_I2C_HOUR,utc_buf,3,0);
    if(res!=TEE_SUCCESS){
        return res;
    }
    utc_time->hour=utc_buf[0];
    utc_time->minute=utc_buf[1];
    utc_time->second=utc_buf[2];
    DMSG("%d %d %d", utc_time->hour, utc_time->minute, utc_time->second);
    return res;
}

static TEE_Result tel0157_get_lat_internal(const tel0157_t * dev, tel0157_lon_lat_t * lon_lat){
    TEE_Result res;
    uint8_t lat_buf[6]={0};
    res=i2c_read_regs(dev->i2c,dev->addr,TEL0157_I2C_LAT_1,lat_buf,6,0);
    if(res!=TEE_SUCCESS){
        return res;
    }
    lon_lat->latDD=lat_buf[0];
    lon_lat->latMM=lat_buf[1];
    lon_lat->latMMMMM=((uint32_t)lat_buf[2]<<16) | ((uint32_t)lat_buf[3]<<8) | ((uint32_t)lat_buf[4]);
    // float not support in OPTEE OS
    // lon_lat->latitude=(double)(lon_lat->latDD*100.0) + ((double)(lon_lat->latMM)) + ((double)(lon_lat->latMMMMM)/100000.0);
    // lon_lat->latitudeDegree=(double)(lon_lat->latDD) + (double)(lon_lat->latMM)/60.0 + (double)(lon_lat->latMMMMM)/100000.0/60.0;

    res=i2c_read_regs(dev->i2c,dev->addr, TEL0157_I2C_LAT_DIS, lat_buf,1,0);
    if(res!=TEE_SUCCESS){
        return res;
    }
    lon_lat->latDirection=lat_buf[0];
    return res;
}

static TEE_Result tel0157_get_lon_internal(const tel0157_t * dev, tel0157_lon_lat_t * lon_lat){
    TEE_Result res;
    uint8_t lon_buf[6]={0};
    res=i2c_read_regs(dev->i2c,dev->addr,TEL0157_I2C_LON_1,lon_buf,6,0);
    if(res!=TEE_SUCCESS){
        return res;
    }
    lon_lat->lonDD=lon_buf[0];
    lon_lat->lonMM=lon_buf[1];
    lon_lat->lonMMMMM=((uint32_t)lon_buf[2]<<16) | ((uint32_t)lon_buf[3]<<8) | ((uint32_t)lon_buf[4]);
    // float not support in OPTEE OS
    // lon_lat->lonitude=(double)(lon_lat->lonDD)*100.0 + ((double)(lon_lat->lonMM)) + ((double)(lon_lat->lonMMMMM)/100000.0);
    // lon_lat->lonitudeDegree=(double)(lon_lat->lonDD) + (double)(lon_lat->lonMM)/60.0 + (double)(lon_lat->lonMMMMM)/100000.0/60.0;

    res=i2c_read_regs(dev->i2c,dev->addr, TEL0157_I2C_LON_DIS, lon_buf,1,0);
    if(res!=TEE_SUCCESS){
        return res;
    }
    lon_lat->lonDirection=lon_buf[0];
    return res;
}

static inline TEE_Result tel0157_get_alt_internal(const tel0157_t * dev, uint8_t alt[3]){
    // TEE_Result res;
    // res=i2c_read_regs(dev->i2c,dev->addr,TEL0157_I2C_ALT_H,alt,3,0);
    // if(res!=TEE_SUCCESS){
    //     return res;
    // }
    // *alt = (double)((uint16_t)(alt_buf[0]&0x7F)<<8|alt_buf[1])+(double)alt_buf[2]/100.0;
    // if(alt_buf[0] & 0x80){
    //     *alt=-(*alt);
    // }
    // return res;
    return i2c_read_regs(dev->i2c,dev->addr,TEL0157_I2C_ALT_H,alt,3,0);
}

static inline TEE_Result tel0157_get_sog_internal(const tel0157_t * dev, uint8_t sog[3]){
    // TEE_Result res;
    // uint8_t sog_buf[3]={0};
    // res=i2c_read_regs(dev->i2c,dev->addr,TEL0157_I2C_SOG_H,sog_buf,3,0);
    // if(res!=TEE_SUCCESS){
    //     return res;
    // }
    // *sog = (double)((uint16_t)(sog_buf[0]&0x7F)<<8|sog_buf[1])+(double)sog_buf[2]/100.0;
    // if(sog_buf[0] & 0x80){
    //     *sog=-(*sog);
    // }
    // return res;
    return i2c_read_regs(dev->i2c,dev->addr,TEL0157_I2C_SOG_H,sog,3,0);
}

static inline TEE_Result tel0157_get_cog_internal(const tel0157_t * dev, uint8_t cog[3]){
    // TEE_Result res;
    // uint8_t cog_buf[3]={0};
    // res=i2c_read_regs(dev->i2c,dev->addr,TEL0157_I2C_COG_H,cog_buf,3,0);
    // if(res!=TEE_SUCCESS){
    //     return res;
    // }
    // *cog = (double)((uint16_t)(cog_buf[0]&0x7F)<<8|cog_buf[1])+(double)cog_buf[2]/100.0;
    // if(cog_buf[0] & 0x80){
    //     *cog=-(*cog);
    // }
    // return res;
    return i2c_read_regs(dev->i2c,dev->addr,TEL0157_I2C_COG_H,cog,3,0);
}

static inline TEE_Result tel0157_get_num_sat_used_internal(const tel0157_t * dev, uint8_t * num){
    return i2c_read_reg(dev->i2c, dev->addr, TEL0157_I2C_USE_STAR,num,0);
}

static inline TEE_Result tel0157_get_gnss_mode_internal(const tel0157_t * dev, uint8_t * mode){
    return i2c_read_reg(dev->i2c, dev->addr, TEL0157_I2C_GNSS_MODE,mode,0);
}

static inline TEE_Result tel0157_get_gnss_len_internal(const tel0157_t * dev, uint16_t * len){
    TEE_Result res;
    uint8_t len_buf[2]={0};
    len_buf[0]=TEL0157_START_GET_CMD;
    res=i2c_write_reg(dev->i2c,dev->addr,TEL0157_I2C_START_GET,len_buf[0],0);
    if(res!=TEE_SUCCESS){
        return res;
    }
    // wait len ready
    udelay(TEL0157_GET_GNSS_LEN_DELAY_US);
    res=i2c_read_regs(dev->i2c,dev->addr,TEL0157_I2C_DATA_LEN_H,len_buf,2,0);
    if(res!=TEE_SUCCESS){
        return res;
    }
    *len=(uint16_t)len_buf[0]<<8 | len_buf[1];
    return res;
}

static TEE_Result tel0157_get_all_data_internal(const tel0157_t * dev, uint8_t * buf, uint16_t len){
    TEE_Result res;
    res=i2c_write_byte(dev->i2c,dev->addr,TEL0157_I2C_ALL_DATA,0);
    if(res!=TEE_SUCCESS){
        return res;
    }
    return i2c_read_bytes(dev->i2c,dev->addr,buf,len,I2C_CONT);
}

static inline TEE_Result tel0157_get_all_gnss_internal(const tel0157_t * dev, uint8_t * all_gnss_buf, uint16_t len){
    TEE_Result res;
    uint8_t tmp_len;
    // uint8_t buf[256]={0};
    uint32_t block_size=32;
    if(len>1024+200||len==0){
        return TEE_ERROR_BAD_PARAMETERS;
    }
    tmp_len = (len + block_size - 1) / block_size; 
    DMSG("read all gnss len %d, %d blocks, block size %d", len, tmp_len, block_size);

    for (uint16_t i = 0; i < tmp_len; i++) {
        uint32_t current_block_size = (i == tmp_len - 1 && len % block_size != 0) ? len % block_size : block_size;

        res = tel0157_get_all_data_internal(dev, (void *)&(all_gnss_buf[block_size * i]), current_block_size);
        if (res != TEE_SUCCESS) {
            DMSG("failed read all data i=%d", i);
            return res;
        }

        // replace \0
        for (uint8_t j = 0; j < current_block_size; j++) {
            if (all_gnss_buf[block_size * i + j] == '\0') {
                all_gnss_buf[block_size * i + j] = '\n';
            }
        }
    }
    return TEE_SUCCESS;
}

// ~150ms
TEE_Result tel0157_init(const tel0157_t * dev){
    TEE_Result res;
    res=i2c_acquire(dev->i2c);
    if(res!=TEE_SUCCESS){
        return res;
    }
    // enable power
    res=tel0157_enable_power_internal(dev);
    if(res!=TEE_SUCCESS){
        DMSG("enable power failed");
        goto tel0157_init_release_exit;
    }
    udelay(TEL0157_GENERAL_DELAY_US);
    // set gnss
    res=tel0157_set_gnss_mode_internal(dev);
    if(res!=TEE_SUCCESS){
        DMSG("set gnss mode failed");
        goto tel0157_init_release_exit;
    }
    udelay(TEL0157_GENERAL_DELAY_US);
    // set rgb on/off
    res=tel0157_set_rgb_internal(dev);
    if(res!=TEE_SUCCESS){
        DMSG("set rgb failed");
        goto tel0157_init_release_exit;
    }
    udelay(TEL0157_GENERAL_DELAY_US);
tel0157_init_release_exit:
    i2c_release(dev->i2c);
    return res;
}

TEE_Result tel0157_deinit(const tel0157_t * dev){
    TEE_Result res;
    res=i2c_acquire(dev->i2c);
    if(res!=TEE_SUCCESS){
        return res;
    }
    // power down
    res=tel0157_disable_power_internal(dev);
    if(res!=TEE_SUCCESS){
        DMSG("disable power failed");
        goto tel0157_deinit_release_exit;
    }
    udelay(TEL0157_GENERAL_DELAY_US);
tel0157_deinit_release_exit:
    i2c_release(dev->i2c);
    return res;
}

TEE_Result tel0157_get_utc_time(const tel0157_t * dev, tel0157_time_t * utc_time){
    TEE_Result res;
    res=i2c_acquire(dev->i2c);
    if(res!=TEE_SUCCESS){
        return res;
    }
    // get date
    res=tel0157_get_date_internal(dev,utc_time);
    if(res!=TEE_SUCCESS){
        DMSG("get date failed");
        goto tel0157_get_utc_time_release_exit;
    }
    // get utc
    res=tel0157_get_utc_internal(dev,utc_time);
    if(res!=TEE_SUCCESS){
        DMSG("get utc failed");
        goto tel0157_get_utc_time_release_exit;
    }
tel0157_get_utc_time_release_exit:
    i2c_release(dev->i2c);
    return res;
}

TEE_Result tel0157_get_lon(const tel0157_t * dev, tel0157_lon_lat_t * lon_lat){
    TEE_Result res;
    res=i2c_acquire(dev->i2c);
    if(res!=TEE_SUCCESS){
        return res;
    }
    // get lon
    res=tel0157_get_lon_internal(dev,lon_lat);
    if(res!=TEE_SUCCESS){
        DMSG("get lon failed");
        goto tel0157_get_lon_release_exit;
    }
tel0157_get_lon_release_exit:
    i2c_release(dev->i2c);
    return res;
}

TEE_Result tel0157_get_lat(const tel0157_t * dev, tel0157_lon_lat_t * lon_lat){
    TEE_Result res;
    res=i2c_acquire(dev->i2c);
    if(res!=TEE_SUCCESS){
        return res;
    }
    // get lon
    res=tel0157_get_lat_internal(dev,lon_lat);
    if(res!=TEE_SUCCESS){
        DMSG("get lon failed");
        goto tel0157_get_lat_release_exit;
    }
tel0157_get_lat_release_exit:
    i2c_release(dev->i2c);
    return res;
}

TEE_Result tel0157_get_alt(const tel0157_t * dev, uint8_t alt[3]){
    TEE_Result res;
    res=i2c_acquire(dev->i2c);
    if(res!=TEE_SUCCESS){
        return res;
    }
    // get alt
    res=tel0157_get_alt_internal(dev,alt);
    if(res!=TEE_SUCCESS){
        DMSG("get alt failed");
        goto tel0157_get_alt_release_exit;
    }
tel0157_get_alt_release_exit:
    i2c_release(dev->i2c);
    return res;
}

TEE_Result tel0157_get_sog(const tel0157_t * dev, uint8_t sog[3]){
    TEE_Result res;
    res=i2c_acquire(dev->i2c);
    if(res!=TEE_SUCCESS){
        return res;
    }
    // get sog
    res=tel0157_get_sog_internal(dev,sog);
    if(res!=TEE_SUCCESS){
        DMSG("get sog failed");
        goto tel0157_get_sog_release_exit;
    }
tel0157_get_sog_release_exit:
    i2c_release(dev->i2c);
    return res;
}

TEE_Result tel0157_get_cog(const tel0157_t * dev, uint8_t cog[3]){
    TEE_Result res;
    res=i2c_acquire(dev->i2c);
    if(res!=TEE_SUCCESS){
        return res;
    }
    // get cog
    res=tel0157_get_cog_internal(dev,cog);
    if(res!=TEE_SUCCESS){
        DMSG("get cog failed");
        goto tel0157_get_cog_release_exit;
    }
tel0157_get_cog_release_exit:
    i2c_release(dev->i2c);
    return res;
}

TEE_Result tel0157_get_num_sat_used(const tel0157_t * dev, uint8_t * num){
    TEE_Result res;
    res=i2c_acquire(dev->i2c);
    if(res!=TEE_SUCCESS){
        return res;
    }
    res=tel0157_get_num_sat_used_internal(dev,num);
    if(res!=TEE_SUCCESS){
        DMSG("get num sat used failed");
        goto tel0157_get_num_sat_used_release_exit;
    }
tel0157_get_num_sat_used_release_exit:
    i2c_release(dev->i2c);
    return res;
}

TEE_Result tel0157_get_gnss_mode(const tel0157_t * dev, uint8_t * mode){
    TEE_Result res;
    res=i2c_acquire(dev->i2c);
    if(res!=TEE_SUCCESS){
        return res;
    }
    res=tel0157_get_gnss_mode_internal(dev, mode);
    if(res!=TEE_SUCCESS){
        DMSG("get gnss mode failed");
        goto tel0157_get_gnss_mode_release_exit;
    }
tel0157_get_gnss_mode_release_exit:
    i2c_release(dev->i2c);
    return res;
}

TEE_Result tel0157_get_gnss_len(const tel0157_t * dev, uint16_t * len){
    TEE_Result res;
    res=i2c_acquire(dev->i2c);
    if(res!=TEE_SUCCESS){
        return res;
    }
    res=tel0157_get_gnss_len_internal(dev, len);
    if(res!=TEE_SUCCESS){
        DMSG("get gnss len failed");
        goto tel0157_get_gnss_len_release_exit;
    }
tel0157_get_gnss_len_release_exit:
    i2c_release(dev->i2c);
    return res;
}

TEE_Result tel0157_get_all_gnss(const tel0157_t * dev, uint8_t * all_gnss_buf, uint16_t len){
    TEE_Result res;
    res=i2c_acquire(dev->i2c);
    if(res!=TEE_SUCCESS){
        return res;
    }
    res=tel0157_get_all_gnss_internal(dev, all_gnss_buf,len);
    if(res!=TEE_SUCCESS){
        DMSG("get all gnss failed");
        goto tel0157_get_all_gnss_release_exit;
    }
tel0157_get_all_gnss_release_exit:
    i2c_release(dev->i2c);
    return res;
}