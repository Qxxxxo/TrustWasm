/**
 * @brief native funcs for trust zone io
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#ifndef TZIO_H
#define TZIO_H
#include <stdint.h>
#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include "pta_tel0157.h"
#include "pta_atk301.h"

int record_benchmark_time(uint32_t idx);
int get_benchmark_time(uint32_t idx, uint64_t * out_time);

int i2c_read_bytes(uint32_t i2c_dev, uint32_t addr, uint32_t flags, void * buf, uint32_t buf_len);
int i2c_write_bytes(uint32_t i2c_dev, uint32_t addr, uint32_t flags,   void * buf, uint32_t buf_len);
int i2c_set_freq(uint32_t i2c_dev, uint32_t i2c_freq_opt);
int i2c_read_regs(uint32_t i2c_dev, uint32_t addr, uint32_t flags,uint32_t reg, void * buf, uint32_t buf_len);
int i2c_write_regs(uint32_t i2c_dev, uint32_t addr, uint32_t flags,uint32_t reg,   void * buf, uint32_t buf_len);
int dht_init(uint32_t pin, uint32_t dht_type);
int dht_read(uint32_t pin, uint32_t dht_type, int16_t * temp, int16_t * hum);
int bh1750fvi_init(uint32_t i2c_no,uint32_t addr);
int bh1750fvi_sample(uint32_t i2c_no,uint32_t addr,uint16_t * lux);
int mpu6050_init(uint32_t i2c_no,uint32_t i2c_addr);
int mpu6050_deinit(uint32_t i2c_no,uint32_t i2c_addr);
int mpu6050_sample(uint32_t i2c_no,uint32_t i2c_addr, uint8_t mpu6050_data[14]);
int w25qxx_init(uint32_t spi_no,uint32_t spi_cs_no);
int w25qxx_read_id(uint32_t spi_no,uint32_t spi_cs_no, uint16_t * id);
int w25qxx_read_data(uint32_t spi_no,uint32_t spi_cs_no, uint32_t read_addr, void * buf, uint32_t len);
int w25qxx_page_program(uint32_t spi_no, uint32_t spi_cs_no, uint32_t write_addr,   void * input_data_buf, uint32_t len);
int w25qxx_sector_erase(uint32_t spi_no, uint32_t spi_cs_no, uint32_t sector_addr);
int w25qxx_chip_erase(uint32_t spi_no, uint32_t spi_cs_no);
int tel0157_init(uint32_t i2c_no,uint32_t i2c_addr, uint32_t gnss_mode, uint32_t rgb);
int tel0157_deinit(uint32_t i2c_no,uint32_t i2c_addr);
int tel0157_get_gnss_len(uint32_t i2c_no,uint32_t i2c_addr, uint16_t * len);
int tel0157_get_all_gnss(uint32_t i2c_no,uint32_t i2c_addr, void * buf, uint32_t len);
int tel0157_get_utc_time(uint32_t i2c_no,uint32_t i2c_addr,  tel0157_time_t * buf);
int tel0157_get_gnss_mode(uint32_t i2c_no,uint32_t i2c_addr, uint8_t * mode);
int tel0157_get_num_sat_used(uint32_t i2c_no,uint32_t i2c_addr, uint8_t * num);
int tel0157_get_lon(uint32_t i2c_no,uint32_t i2c_addr, tel0157_lon_lat_t * lon);
int tel0157_get_lat(uint32_t i2c_no,uint32_t i2c_addr, tel0157_lon_lat_t * lat);
int tel0157_get_alt(uint32_t i2c_no,uint32_t i2c_addr, uint8_t alt[3]);
int tel0157_get_sog(uint32_t i2c_no,uint32_t i2c_addr, uint8_t sog[3]);
int tel0157_get_cog(uint32_t i2c_no,uint32_t i2c_addr, uint8_t cog[3]);
int spi_transfer_byte(uint32_t spi_no,uint32_t spi_cs_no, uint32_t cont,
                             uint32_t mode, uint32_t freq,  uint32_t need_byte_out, uint32_t need_byte_in,
                              uint32_t byte_out, uint8_t * byte_in);
int spi_transfer_bytes(uint32_t spi_no,uint32_t spi_cs_no, uint32_t cont,
                             uint32_t mode, uint32_t freq,  uint32_t src_len, uint32_t dst_len,
                                uint8_t * src_buf, uint8_t * dst_buf);
int spi_transfer_diff_bytes(uint32_t spi_no,uint32_t spi_cs_no, uint32_t cont,
                             uint32_t mode, uint32_t freq,  uint32_t src_len, uint32_t dst_len,
                                uint8_t * src_buf, uint8_t * dst_buf);
int write_secure_object(char * id, uint32_t id_len, char * data, uint32_t data_len);
int read_secure_object(char * id, uint32_t id_len, char * data, uint32_t * data_len);
int delete_secure_object(char * id, uint32_t id_len);
int at24cxx_read(uint32_t i2c_no,uint32_t i2c_addr, uint32_t read_addr, uint8_t * buf, uint32_t len);
int at24cxx_write(uint32_t i2c_no,uint32_t i2c_addr, uint32_t write_addr,   uint8_t * buf, uint32_t len);
int atk301_init(uint32_t i2c_no, uint32_t i2c_addr);
int atk301_get_fingerprint_image(uint32_t i2c_no, uint32_t i2c_addr);
int atk301_upload_fingerprint_image(uint32_t i2c_no, uint32_t i2c_addr, uint8_t buf[ATK301_FINGERPRINT_IMAGE_DATA_LEN]);
int native_wait(int ms);
int gpio_init(int pin, int mode);
int gpio_set(int pin, int level);
int gpio_get(int pin);

#endif