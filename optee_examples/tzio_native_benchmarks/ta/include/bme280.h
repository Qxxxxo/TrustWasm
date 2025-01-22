#ifndef BME_H
#define BME_H

#include "tzio.h"

#define BME280_REG_HUM_LSB             0xFE        /**< hum lsb register */
#define BME280_REG_HUM_MSB             0xFD        /**< hum msb register */
#define BME280_REG_TEMP_XLSB           0xFC        /**< temp xlsb register */
#define BME280_REG_TEMP_LSB            0xFB        /**< temp lsb register */
#define BME280_REG_TEMP_MSB            0xFA        /**< temp msb register */
#define BME280_REG_PRESS_XLSB          0xF9        /**< press xlsb register */
#define BME280_REG_PRESS_LSB           0xF8        /**< press lsb register */
#define BME280_REG_PRESS_MSB           0xF7        /**< press msb register */
#define BME280_REG_CONFIG              0xF5        /**< config register */
#define BME280_REG_CTRL_MEAS           0xF4        /**< ctrl meas register */
#define BME280_REG_STATUS              0xF3        /**< status register */
#define BME280_REG_CTRL_HUM            0xF2        /**< ctrl hum register */
#define BME280_REG_RESET               0xE0        /**< soft reset register */
#define BME280_REG_ID                  0xD0        /**< chip id register */
#define BME280_CALIBRATION_REG_TEMP_PRESS   0x88
#define BME280_CALIBRATION_REG_HUM_1   0xA1
#define BME280_CALIBRATION_REG_HUM     0xE1

typedef struct {
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;

    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;

    uint8_t dig_H1;
    int16_t dig_H2;
    uint8_t dig_H3;
    int16_t dig_H4;
    int16_t dig_H5;
    int8_t dig_H6;
} bme280_calibration_data_t;

float bme280_convert_temperature(uint8_t *buf, bme280_calibration_data_t * calib_data, int32_t * t_fine) {
    int32_t raw_temp = ((int32_t)buf[3] << 12) | ((int32_t)buf[4] << 4) | ((int32_t)buf[5] >> 4);
    float res;
    float var1;
    float var2;
    float temperature_min = -40.0f;
    float temperature_max = 85.0f;
    var1 = (((float)raw_temp) / 16384.0f - ((float)calib_data->dig_T1) / 1024.0f);         /* set var1 */
    var1 = var1 * ((float)calib_data->dig_T2);                                        /* set var1 */
    var2 = (((float)raw_temp) / 131072.0f - ((float)calib_data->dig_T1) / 8192.0f);        /* set var2 */
    var2 = (var2 * var2) * ((float)calib_data->dig_T3);                               /* set var2 */
    *t_fine = (int32_t)(var1 + var2);                                  /* set t_fine */
    res = (var1 + var2) / 5120.0f;                                    /* set temperature */
    if (res < temperature_min)                                        /* if min */
    {
        res = temperature_min;                                        /* set temperature min */
    }
    else if (res > temperature_max)                                   /* if max */
    {
        res = temperature_max;                                        /* set temperature max */
    }
    return res;                                                               /* return result */
}

float bme280_convert_pressure(uint8_t *buf, bme280_calibration_data_t * calib_data,int32_t t_fine) {
    int32_t raw_pressure = ((int32_t)buf[0] << 12) | ((int32_t)buf[1] << 4) | ((int32_t)buf[2] >> 4);
    float var1;
    float var2;
    float var3;
    float pressure;
    float pressure_min = 30000.0f;
    float pressure_max = 110000.0f;

    var1 = ((float)t_fine / 2.0f) - 64000.0f;                             /* set var1 */
    var2 = var1 * var1 * ((float)calib_data->dig_P6) / 32768.0f;                          /* set var2 */
    var2 = var2 + var1 * ((float)calib_data->dig_P5) * 2.0f;                              /* set var2 */
    var2 = (var2 / 4.0f) + (((float)calib_data->dig_P4) * 65536.0f);                      /* set var2 */
    var3 = ((float)calib_data->dig_P3) * var1 * var1 / 524288.0f;                         /* set var3 */
    var1 = (var3 + ((float)calib_data->dig_P2) * var1) / 524288.0f;                       /* set var1 */
    var1 = (1.0f + var1 / 32768.0f) * ((float)calib_data->dig_P1);                        /* set var1 */
    if (var1 > (0.0f))                                                            /* if over zero */
    {
        pressure = 1048576.0f - (float)raw_pressure;                                       /* set pressure */
        pressure = (pressure - (var2 / 4096.0f)) * 6250.0f / var1;                /* set pressure */
        var1 = ((float)calib_data->dig_P9) * pressure * pressure / 2147483648.0f;         /* set var1 */
        var2 = pressure * ((float)calib_data->dig_P8) / 32768.0f;                         /* set var2 */
        pressure = pressure + (var1 + var2 + ((float)calib_data->dig_P7)) / 16.0f;        /* set pressure */
        if (pressure < pressure_min)                                              /* if min */
        {
            pressure = pressure_min;                                              /* set pressure */
        }
        else if (pressure > pressure_max)                                         /* if max */
        {
            pressure = pressure_max;                                              /* set pressure */
        }
    }
    else
    {
        pressure = pressure_min;                                                  /* set pressure */
       
    }
    return pressure/100.0f;                                                                   /* return result */
}

float bme280_convert_humidity(uint8_t *buf, bme280_calibration_data_t * calib_data,int32_t t_fine) {
    int32_t raw_humidity = ((int32_t)buf[6] << 8) | (int32_t)buf[7];
    float var1;
    float var2;
    float var3;
    float var4;
    float var5;
    float var6;
    float humidity;
    float humidity_min = 0.0f;
    float humidity_max = 100.0f;
    var1 = ((float)t_fine) - 76800.0f;                                             /* set var1 */
    var2 = (((float)calib_data->dig_H4) * 64.0f + (((float)calib_data->dig_H5) / 16384.0f) * var1);        /* set var2 */
    var3 = (float)raw_humidity - var2;                                                              /* set var3 */
    var4 = ((float)calib_data->dig_H2) / 65536.0f;                                                 /* set var4 */
    var5 = (1.0f + (((float)calib_data->dig_H3) / 67108864.0f) * var1);                            /* set var5 */
    var6 = 1.0f + (((float)calib_data->dig_H6) / 67108864.0f) * var1 * var5;                       /* set var6 */
    var6 = var3 * var4 * (var5 * var6);                                                    /* set var6 */
    humidity = var6 * (1.0f - ((float)calib_data->dig_H1) * var6 / 524288.0f);                     /* set humidity */
    if (humidity > humidity_max)                                                           /* if max */
    {
        humidity = humidity_max;                                                           /* set humidity */
    }
    else if (humidity < humidity_min)                                                      /* if min */
    {
        humidity = humidity_min;                                                           /* set humidity */
    }
    return humidity;                                                                            /* return result */
}

int bme280_init(uint32_t i2c_no,uint32_t i2c_addr){
    int res;
    uint8_t config[2]={0};
    uint8_t id=0;
    res=i2c_read_regs(i2c_no,i2c_addr,0,BME280_REG_ID,&id,1);
    // printf("BME280 get id %x, res: %x\n",id,res);
    if(res!=0) return res;
    if(id!=0x60) return -1;
    config[0]=BME280_REG_CTRL_HUM;
    config[1]=0x01;
    res=i2c_write_bytes(i2c_no,i2c_addr,0,config, 2);
    // printf("BME280 set ctrl hum res: %x\n",res);
    if(res!=0) return res;
    config[0]=BME280_REG_CTRL_MEAS;
    config[1]=0x27;
    res=i2c_write_bytes(i2c_no,i2c_addr,0,config, 2);
    // printf("BME280 set ctrl meas res: %x\n",res);
    config[0]=BME280_REG_CONFIG;
    config[1]=0xA0;
    res=i2c_write_bytes(i2c_no,i2c_addr,0,config, 2);
    // printf("BME280 set ctrl meas res: %x\n",res);
    return res;
}

int bme280_get_calibration(uint32_t i2c_no,uint32_t i2c_addr, bme280_calibration_data_t * calib_data){
    int res;
    uint8_t data[24];
    res=i2c_read_regs(i2c_no,i2c_addr,0,BME280_CALIBRATION_REG_TEMP_PRESS,data,24);
    if(res!=0) return res;
    // Temperature calibration data
    calib_data->dig_T1 = ((uint16_t)data[1] << 8) | (uint16_t)data[0];
    calib_data->dig_T2 = ((int16_t)data[3] << 8) | (int16_t)data[2];
    calib_data->dig_T3 = ((int16_t)data[5] << 8) | (int16_t)data[4];

    // Pressure calibration data  
    calib_data->dig_P1 = ((uint16_t)data[7] << 8) | (uint16_t)data[6];
    calib_data->dig_P2 = ((int16_t)data[9] << 8) | (int16_t)data[8];
    calib_data->dig_P3 = ((int16_t)data[11] << 8) | (int16_t)data[10];
    calib_data->dig_P4 = ((int16_t)data[13] << 8) | (int16_t)data[12];
    calib_data->dig_P5 = ((int16_t)data[15] << 8) | (int16_t)data[14];
    calib_data->dig_P6 = ((int16_t)data[17] << 8) | (int16_t)data[16];
    calib_data->dig_P7 = ((int16_t)data[19] << 8) | (int16_t)data[18];
    calib_data->dig_P8 = ((int16_t)data[21] << 8) | (int16_t)data[20];
    calib_data->dig_P9 = ((int16_t)data[23] << 8) | (int16_t)data[22];
    // hum
    res=i2c_read_regs(i2c_no,i2c_addr,0,BME280_CALIBRATION_REG_HUM_1,data,1);
    if(res!=0) return res;
    calib_data->dig_H1=data[0];
    res=i2c_read_regs(i2c_no,i2c_addr,0,BME280_CALIBRATION_REG_HUM,data,8);
    if(res!=0) return res;
    calib_data->dig_H2 = (int16_t)(((uint16_t)data[1] << 8) | data[0]);
    calib_data->dig_H3 = data[2];
    calib_data->dig_H4 = (int16_t)(((uint16_t)data[3] << 4) | (data[4] & 0x0F));
    calib_data->dig_H5 = (int16_t)(((uint16_t)data[6] << 4) | ((data[5] >> 4)&0x0F));
    calib_data->dig_H6 = (int8_t)(data[7]);
    return 0;
}

int bme280_sample(uint32_t i2c_no,uint32_t i2c_addr, uint8_t buf[8]){
    return i2c_read_regs(i2c_no,i2c_addr,0,BME280_REG_PRESS_MSB,(uint32_t)buf,8);
}


#endif