/**
 * @brief wasm use i2c native
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

__attribute__((import_name("i2c_read_bytes"))) int
i2c_read_bytes(uint32_t i2c_dev, uint32_t addr, uint32_t flags, uint32_t dst_buf_addr, uint32_t buf_len);

__attribute__((import_name("i2c_write_bytes"))) int
i2c_write_bytes(uint32_t i2c_no, uint32_t addr, uint32_t flags, uint32_t src_buf_addr, uint32_t buf_len);

__attribute__((import_name("i2c_read_regs"))) int
i2c_read_regs(uint32_t i2c_dev, uint32_t addr, uint32_t flags, uint32_t reg, uint32_t dst_buf_addr, uint32_t buf_len);

__attribute__((import_name("i2c_write_regs"))) int
i2c_write_regs(uint32_t i2c_no, uint32_t addr, uint32_t flags, uint32_t reg, uint32_t src_buf_addr, uint32_t buf_len);

__attribute__((import_name("native_wait"))) int 
native_wait(int ms);

#define BME280_ADDR 0x77

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

void print_array(uint8_t * buf, uint32_t len){
    for(uint32_t i=0;i<len;i++){
        printf("%x ",buf[i]);
    }
    printf("\n");
}

// 简单的温度转换（忽略校准）
float convert_temperature(uint8_t *buf) {
    int32_t adc_T = ((int32_t)buf[3] << 12) | ((int32_t)buf[4] << 4) | ((int32_t)buf[5] >> 4);
    float temperature = adc_T / 100.0;  // 温度转换公式，不进行校准
    return temperature;
}

// 简单的气压转换（忽略校准）
float convert_pressure(uint8_t *buf) {
    int32_t adc_P = ((int32_t)buf[0] << 12) | ((int32_t)buf[1] << 4) | ((int32_t)buf[2] >> 4);
    float pressure = adc_P / 256.0;  // 气压转换公式，不进行校准
    return pressure;
}

// 简单的湿度转换（忽略校准）
float convert_humidity(uint8_t *buf) {
    int32_t adc_H = ((int32_t)buf[6] << 8) | (int32_t)buf[7];
    float humidity = adc_H / 1024.0;  // 湿度转换公式，不进行校准
    return humidity;
}



int main(int argc, char **argv)
{
    uint8_t buf[8]={0};
    float temperature, pressure, humidity;
    uint8_t config[2]={0};
    
    
    
    config[0]=BME280_REG_RESET;
    config[1]=0xB6;
    i2c_write_bytes(1, BME280_ADDR,0,config, 2);
    native_sleep(5);
    config[0]=BME280_REG_CTRL_MEAS;
    config[1]=(0x05 << 5) | (0x01 << 2) | 0x03;
    i2c_write_bytes(1, BME280_ADDR,0,config, 2); 
    config[0]=BME280_REG_CTRL_HUM;
    config[1]=0x01;
    i2c_write_bytes(1, BME280_ADDR,0,config, 2); 
    config[0]=BME280_REG_CONFIG;
    config[1]=0xA0;
    i2c_write_bytes(1, BME280_ADDR,0,config, 2);

    native_sleep(1000);
    for(int i=0;i<5;i++){
        i2c_read_regs(1,BME280_ADDR,0,BME280_REG_PRESS_MSB,buf,8);
        print_array(buf,8);

        temperature = convert_temperature(buf);
        pressure = convert_pressure(buf);
        humidity = convert_humidity(buf);

        printf("Temperature: %.2f C\n", temperature);
        printf("Pressure: %.2f hPa\n", pressure);
        printf("Humidity: %.2f %%\n", humidity);
        native_sleep(200);
    }
    return 0;
}

