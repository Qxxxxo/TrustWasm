/**
 * @brief safe native funcs
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// for test only
#define SIGNATURE 0xE5
#define TAG 0x12

typedef struct{
    uint16_t year; 
    uint8_t month;
    uint8_t date;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
}tel0157_time_t;

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

__attribute__((import_name("at24cxx_read"))) int
at24cxx_read(uint32_t i2c_no,uint32_t i2c_addr, uint32_t read_addr, uint32_t dst_buf_addr, uint32_t len);

__attribute__((import_name("at24cxx_write"))) int
at24cxx_write(uint32_t i2c_no,uint32_t i2c_addr, uint32_t write_addr, uint32_t src_buf_addr, uint32_t len);

__attribute__((import_name("bh1750fvi_init"))) int
bh1750fvi_init(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("bh1750fvi_sample"))) int
bh1750fvi_sample(uint32_t i2c_no, uint32_t addr, uint32_t dst_lux_addr);

__attribute__((import_name("dht_init"))) int
dht_init(uint32_t pin, uint32_t type);

__attribute__((import_name("dht_read"))) int
dht_read(uint32_t pin, uint32_t type, uint32_t dst_temp_addr, uint32_t dst_hum_addr);

__attribute__((import_name("gpio_init"))) int
gpio_init(int pin, int func_code);

__attribute__((import_name("gpio_set"))) int
gpio_set(int pin, int val);

__attribute__((import_name("gpio_get"))) int
gpio_get(int pin);

__attribute__((import_name("mpu6050_init"))) int
mpu6050_init(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("mpu6050_deinit"))) int
mpu6050_deinit(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("mpu6050_sample"))) int
mpu6050_sample(uint32_t i2c_no, uint32_t addr, uint32_t dst_addr);

__attribute__((import_name("write_secure_object"))) int
write_secure_object(uint32_t id_addr, uint32_t id_len, uint32_t data_addr, uint32_t data_len);

__attribute__((import_name("read_secure_object"))) int
read_secure_object(uint32_t id_addr, uint32_t id_len, uint32_t data_addr, uint32_t data_len_addr);

__attribute__((import_name("delete_secure_object"))) int
delete_secure_object(uint32_t id_addr, uint32_t id_len);

__attribute__((import_name("tel0157_init"))) int
tel0157_init(uint32_t i2c_no, uint32_t addr, uint32_t gnss_mode, uint32_t rgb);

__attribute__((import_name("tel0157_deinit"))) int
tel0157_deinit(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("tel0157_get_gnss_len"))) int
tel0157_get_gnss_len(uint32_t i2c_no, uint32_t addr, uint32_t len_dst_addr);

__attribute__((import_name("tel0157_get_all_gnss"))) int
tel0157_get_all_gnss(uint32_t i2c_no, uint32_t addr, uint32_t dst_buf_addr,uint32_t len);

__attribute__((import_name("tel0157_get_utc_time"))) int
tel0157_get_utc_time(uint32_t i2c_no, uint32_t addr, uint32_t utc_dst_addr);

__attribute__((import_name("tel0157_get_gnss_mode"))) int
tel0157_get_gnss_mode(uint32_t i2c_no, uint32_t addr, uint32_t mode_dst_addr);

__attribute__((import_name("tel0157_get_num_sat_used"))) int
tel0157_get_num_sat_used(uint32_t i2c_no, uint32_t addr, uint32_t num_dst_addr);

__attribute__((import_name("tel0157_get_lon"))) int
tel0157_get_lon(uint32_t i2c_no, uint32_t addr, uint32_t lon_dst_addr);

__attribute__((import_name("tel0157_get_lat"))) int
tel0157_get_lat(uint32_t i2c_no, uint32_t addr, uint32_t lat_dst_addr);

__attribute__((import_name("tel0157_get_alt"))) int
tel0157_get_alt(uint32_t i2c_no, uint32_t addr, uint32_t alt_dst_addr);

__attribute__((import_name("tel0157_get_sog"))) int
tel0157_get_sog(uint32_t i2c_no, uint32_t addr, uint32_t sog_dst_addr);

__attribute__((import_name("tel0157_get_cog"))) int
tel0157_get_cog(uint32_t i2c_no, uint32_t addr, uint32_t cog_dst_addr);

__attribute__((import_name("atk301_init"))) int
atk301_init(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("atk301_get_fingerprint_image"))) int
atk301_get_fingerprint_image(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("atk301_upload_fingerprint_image"))) int
atk301_upload_fingerprint_image(uint32_t i2c_no, uint32_t addr, uint32_t dst_buf_addr);

__attribute__((import_name("sign_ptr")))
uint32_t sign_ptr(uint32_t original_ptr, uint32_t signature, uint32_t tag);

__attribute__((import_name("native_wait"))) int 
native_wait(int ms);

__attribute__((import_name("record_benchmark_time"))) int record_benchmark_time(uint32_t idx);
__attribute__((import_name("get_benchmark_time"))) int get_benchmark_time(uint32_t idx, uint32_t out_time_addr);

#define AT24CXX_ADDR 0x50
#define AT24CXX_I2C_NO  0
#define AT24CXX_MEM_ADDR 0x00
#define AT24CXX_TEST_LEN 4*1024
#define ATK301_ADDR 0x27
#define ATK301_I2C_NO   0
#define ATK301_TEST_LEN  12800
#define BH1750FVI_ADDR 0x23
#define BH1750FVI_I2C_NO   1
#define DHT_GPIO_NO 22
#define DHT_TYPE 0
#define GPIO_TEST_NO 26
#define GPIO_FSEL_OUTPUT 1
#define GPIO_FSEL_INPUT  0
#define I2C_TEST_ADDR 0x55
#define I2C_TEST_LEN 4*1024
#define MPU6050_ADDR 0x68
#define MPU6050_I2C_NO  1
#define MPU6050_TEST_LEN 14
#define SECURE_STORAGE_OBJECT_ID "test"
#define SECURE_STORAGE_OBJECT_ID_LEN 4
#define SECURE_STORAGE_OBJECT_LEN 4*1024
#define SPI_NO  0
#define SPI_CS_NO 1
#define SPI_TEST_LEN    4*1024
#define TEL0157_ADDR 0x20
#define TEL0157_I2C_NO 1
#define TEL0157_GNSS_MODE 7
#define TEL0157_RGB 1

#define RUN_BENCHMARK_TIMES 100

int main(int argc, char **argv)
{
    int res;
    uint64_t start,end;
    uint8_t * data;
    uint32_t signed_data;
    for(int i=0;i<RUN_BENCHMARK_TIMES;i++){
    // AT24CXX native funcs
    // data = (uint8_t*)malloc(AT24CXX_TEST_LEN);
    // record_benchmark_time(0);
    // signed_data=sign_ptr(data,SIGNATURE,TAG);
    // res=at24cxx_write(AT24CXX_I2C_NO,AT24CXX_ADDR,AT24CXX_MEM_ADDR,signed_data,AT24CXX_TEST_LEN);
    // record_benchmark_time(1);
    // get_benchmark_time(0,(uint32_t)&start);
    // get_benchmark_time(1,(uint32_t)&end);
    // printf("res %x, at24cxx_write (4K data) interval %llu\n",res,end-start);

    // record_benchmark_time(0);
    // res=at24cxx_read(AT24CXX_I2C_NO,AT24CXX_ADDR,AT24CXX_MEM_ADDR,signed_data,AT24CXX_TEST_LEN);
    // record_benchmark_time(1);
    // get_benchmark_time(0,(uint32_t)&start);
    // get_benchmark_time(1,(uint32_t)&end);
    // printf("res %x, at24cxx_read (4K data) interval %llu\n",res,end-start);
    // free(data);

    // // ATK301 native funcs
    // record_benchmark_time(0);
    // res=atk301_init(ATK301_I2C_NO,ATK301_ADDR);
    // record_benchmark_time(1);
    // get_benchmark_time(0,(uint32_t)&start);
    // get_benchmark_time(1,(uint32_t)&end);
    // printf("res %x, atk301_init interval %llu\n",res,end-start);

    // record_benchmark_time(0);
    // res=atk301_get_fingerprint_image(ATK301_I2C_NO,ATK301_ADDR);
    // record_benchmark_time(1);
    // get_benchmark_time(0,(uint32_t)&start);
    // get_benchmark_time(1,(uint32_t)&end);
    // printf("res %x, atk301_get_fingerprint_image interval %llu\n",res,end-start);

    // data=(uint8_t*)malloc(ATK301_TEST_LEN);
    // record_benchmark_time(0);
    // signed_data=sign_ptr(data,SIGNATURE,TAG);
    // res=atk301_upload_fingerprint_image(ATK301_I2C_NO,ATK301_ADDR,signed_data);
    // record_benchmark_time(1);
    // get_benchmark_time(0,(uint32_t)&start);
    // get_benchmark_time(1,(uint32_t)&end);
    // printf("res %x, atk301_upload_fingerprint_image interval %llu\n",res,end-start);
    // free(data);

    // BH1750FVI native funcs
    // record_benchmark_time(0);
    // res=bh1750fvi_init(BH1750FVI_I2C_NO,BH1750FVI_ADDR);
    // record_benchmark_time(1);
    // get_benchmark_time(0,(uint32_t)&start);
    // get_benchmark_time(1,(uint32_t)&end);
    // printf("res %x, bh1750fvi_init interval %llu\n",res,end-start);

    // uint16_t lux=0;
    // record_benchmark_time(0);
    // uint32_t signed_lux=sign_ptr(&lux,SIGNATURE,TAG);
    // res=bh1750fvi_sample(BH1750FVI_I2C_NO,BH1750FVI_ADDR,signed_lux);
    // record_benchmark_time(1);
    // get_benchmark_time(0,(uint32_t)&start);
    // get_benchmark_time(1,(uint32_t)&end);
    // printf("res %x, bh1750fvi_sample interval %llu\n",res,end-start);


    // // DHT native funcs
    // record_benchmark_time(0);
    // res=dht_init(DHT_GPIO_NO,DHT_TYPE);
    // record_benchmark_time(1);
    // get_benchmark_time(0,(uint32_t)&start);
    // get_benchmark_time(1,(uint32_t)&end);
    // printf("res %x, dht_init interval %llu\n",res,end-start);

    // int16_t temp=0;
    // int16_t hum=0;
    // record_benchmark_time(0);
    // uint32_t signed_temp=sign_ptr(&temp,SIGNATURE,TAG);
    // uint32_t signed_hum=sign_ptr(&hum,SIGNATURE,TAG);
    // res=dht_read(DHT_GPIO_NO,DHT_TYPE,signed_temp,signed_hum);
    // record_benchmark_time(1);
    // get_benchmark_time(0,(uint32_t)&start);
    // get_benchmark_time(1,(uint32_t)&end);
    // printf("res %x, dht_read interval %llu\n",res,end-start);

    // // GPIO native funcs
    // record_benchmark_time(0);
    // res=gpio_init(GPIO_TEST_NO,GPIO_FSEL_OUTPUT);
    // record_benchmark_time(1);
    // get_benchmark_time(0,(uint32_t)&start);
    // get_benchmark_time(1,(uint32_t)&end);
    // printf("res %x, gpio_init interval %llu\n",res,end-start);

    // record_benchmark_time(0);
    // res=gpio_set(GPIO_TEST_NO,1);
    // record_benchmark_time(1);
    // get_benchmark_time(0,(uint32_t)&start);
    // get_benchmark_time(1,(uint32_t)&end);
    // printf("res %x, gpio_set interval %llu\n",res,end-start);

    // res=gpio_init(GPIO_TEST_NO,GPIO_FSEL_INPUT);

    // record_benchmark_time(0);
    // res=gpio_get(GPIO_TEST_NO);
    // record_benchmark_time(1);
    // get_benchmark_time(0,(uint32_t)&start);
    // get_benchmark_time(1,(uint32_t)&end);
    // printf("res %x, gpio_get interval %llu\n",res,end-start);

    // // MPU6050 native funcs
    // record_benchmark_time(0);
    // res=mpu6050_init(MPU6050_I2C_NO,MPU6050_ADDR);
    // record_benchmark_time(1);
    // get_benchmark_time(0,(uint32_t)&start);
    // get_benchmark_time(1,(uint32_t)&end);
    // printf("res %x, mpu6050_init interval %llu\n",res,end-start);

    // data=(uint8_t *)malloc(MPU6050_TEST_LEN);
    // record_benchmark_time(0);
    // signed_data=sign_ptr(data,SIGNATURE,TAG);
    // res=mpu6050_sample(MPU6050_I2C_NO,MPU6050_ADDR,signed_data);
    // record_benchmark_time(1);
    // get_benchmark_time(0,(uint32_t)&start);
    // get_benchmark_time(1,(uint32_t)&end);
    // printf("res %x, mpu6050_sample interval %llu\n",res,end-start);

    // Secure storage native funcs
    char id[SECURE_STORAGE_OBJECT_ID_LEN]=SECURE_STORAGE_OBJECT_ID;
    data=(uint8_t*)malloc(SECURE_STORAGE_OBJECT_LEN);
    record_benchmark_time(0);
    uint32_t signed_id=sign_ptr(id,SIGNATURE,TAG);
    signed_data=sign_ptr(data,SIGNATURE,TAG);
    res=write_secure_object(signed_id,SECURE_STORAGE_OBJECT_ID_LEN,signed_data,SECURE_STORAGE_OBJECT_LEN);
    record_benchmark_time(1);
    get_benchmark_time(0,(uint32_t)&start);
    get_benchmark_time(1,(uint32_t)&end);
    printf("res %x, write_secure_object,%llu\n",res,end-start);

    native_wait(10);
    
    uint32_t len=SECURE_STORAGE_OBJECT_LEN;
    record_benchmark_time(0);
    uint32_t signed_len=sign_ptr(&len,SIGNATURE,TAG);
    // printf("origin %08x, signed: %08x\n",&len,signed_len);
    res=read_secure_object(signed_id,SECURE_STORAGE_OBJECT_ID_LEN,signed_data,signed_len);
    record_benchmark_time(1);
    get_benchmark_time(0,(uint32_t)&start);
    get_benchmark_time(1,(uint32_t)&end);
    printf("res %x, read_secure_object,%llu\n",res,end-start);
    native_wait(10);

    char new_id[SECURE_STORAGE_OBJECT_ID_LEN]=SECURE_STORAGE_OBJECT_ID;
    record_benchmark_time(0);
    uint32_t new_signed_id=sign_ptr(new_id,SIGNATURE,TAG);
    res=delete_secure_object(new_signed_id,SECURE_STORAGE_OBJECT_ID_LEN);
    record_benchmark_time(1);
    get_benchmark_time(0,(uint32_t)&start);
    get_benchmark_time(1,(uint32_t)&end);
    printf("res %x, delete_secure_object,%llu\n",res,end-start);
    free(data);

    // TEL0157 native funcs
    // record_benchmark_time(0);
    // res=tel0157_init(TEL0157_I2C_NO,TEL0157_ADDR,TEL0157_GNSS_MODE, TEL0157_RGB);
    // record_benchmark_time(1);
    // get_benchmark_time(0,(uint32_t)&start);
    // get_benchmark_time(1,(uint32_t)&end);
    // printf("res %x, tel0157_init interval %llu\n",res,end-start);

    // tel0157_time_t time;
    // record_benchmark_time(0);
    // uint32_t signed_time=sign_ptr(&time,SIGNATURE,TAG);
    // res=tel0157_get_utc_time(TEL0157_I2C_NO,TEL0157_ADDR,signed_time);
    // record_benchmark_time(1);
    // get_benchmark_time(0,(uint32_t)&start);
    // get_benchmark_time(1,(uint32_t)&end);
    // printf("res %x, tel0157_get_utc_time interval %llu\n",res,end-start);

    // uint8_t gnss_mode;
    // record_benchmark_time(0);
    // uint32_t signed_gnss_mode=sign_ptr(&gnss_mode,SIGNATURE,TAG);
    // res=tel0157_get_gnss_mode(TEL0157_I2C_NO,TEL0157_ADDR,signed_gnss_mode);
    // record_benchmark_time(1);
    // get_benchmark_time(0,(uint32_t)&start);
    // get_benchmark_time(1,(uint32_t)&end);
    // printf("res %x, tel0157_get_gnss_mode interval %llu\n",res,end-start);

    // uint8_t gnss_num_sat_used;
    // record_benchmark_time(0);
    // uint32_t signed_gnss_num_sat_used=sign_ptr(&gnss_num_sat_used,SIGNATURE,TAG);
    // res=tel0157_get_num_sat_used(TEL0157_I2C_NO,TEL0157_ADDR,signed_gnss_num_sat_used);
    // record_benchmark_time(1);
    // get_benchmark_time(0,(uint32_t)&start);
    // get_benchmark_time(1,(uint32_t)&end);
    // printf("res %x, tel0157_get_num_sat_used interval %llu\n",res,end-start);

    // tel0157_lon_lat_t lon_lat;
    // record_benchmark_time(0);
    // uint32_t signed_lon_lat=sign_ptr(&lon_lat,SIGNATURE,TAG);
    // res=tel0157_get_lon(TEL0157_I2C_NO,TEL0157_ADDR,signed_lon_lat);
    // record_benchmark_time(1);
    // get_benchmark_time(0,(uint32_t)&start);
    // get_benchmark_time(1,(uint32_t)&end);
    // printf("res %x, tel0157_get_lon,%llu\n",res,end-start);

    // record_benchmark_time(0);
    // res=tel0157_get_lat(TEL0157_I2C_NO,TEL0157_ADDR,signed_lon_lat);
    // record_benchmark_time(1);
    // get_benchmark_time(0,(uint32_t)&start);
    // get_benchmark_time(1,(uint32_t)&end);
    // printf("res %x, tel0157_get_lat,%llu\n",res,end-start);

    // uint8_t alt[3];
    // record_benchmark_time(0);
    // uint32_t signed_alt=sign_ptr(alt,SIGNATURE,TAG);
    // res=tel0157_get_alt(TEL0157_I2C_NO,TEL0157_ADDR,signed_alt);
    // record_benchmark_time(1);
    // get_benchmark_time(0,(uint32_t)&start);
    // get_benchmark_time(1,(uint32_t)&end);
    // printf("res %x, tel0157_get_alt,%llu\n",res,end-start);

    // uint8_t sog[3];
    // record_benchmark_time(0);
    // uint32_t signed_sog=sign_ptr(sog,SIGNATURE,TAG);
    // res=tel0157_get_sog(TEL0157_I2C_NO,TEL0157_ADDR,signed_sog);
    // record_benchmark_time(1);
    // get_benchmark_time(0,(uint32_t)&start);
    // get_benchmark_time(1,(uint32_t)&end);
    // printf("res %x, tel0157_get_sog interval %llu\n",res,end-start);

    // uint8_t cog[3];
    // record_benchmark_time(0);
    // uint32_t signed_cog=sign_ptr(cog,SIGNATURE,TAG);
    // res=tel0157_get_cog(TEL0157_I2C_NO,TEL0157_ADDR,signed_cog);
    // record_benchmark_time(1);
    // get_benchmark_time(0,(uint32_t)&start);
    // get_benchmark_time(1,(uint32_t)&end);
    // printf("res %x, tel0157_get_cog,%llu\n",res,end-start);

    // uint16_t gnss_len;
    // record_benchmark_time(0);
    // uint32_t signed_gnss_len=sign_ptr(&gnss_len,SIGNATURE,TAG);
    // res=tel0157_get_gnss_len(TEL0157_I2C_NO,TEL0157_ADDR,signed_gnss_len);
    // record_benchmark_time(1);
    // get_benchmark_time(0,(uint32_t)&start);
    // get_benchmark_time(1,(uint32_t)&end);
    // printf("res %x, tel0157_get_gnss_len interval %llu\n",res,end-start);

    // data=(uint8_t*)malloc(gnss_len);
    // record_benchmark_time(0);
    // signed_data=sign_ptr(data,SIGNATURE,TAG);
    // res=tel0157_get_all_gnss(TEL0157_I2C_NO,TEL0157_ADDR,signed_data,gnss_len);
    // record_benchmark_time(1);
    // get_benchmark_time(0,(uint32_t)&start);
    // get_benchmark_time(1,(uint32_t)&end);
    // printf("res %x, tel0157_get_all_gnss interval %llu\n",res,end-start);
    // free(data);

    // record_benchmark_time(0);
    // res=tel0157_deinit(TEL0157_I2C_NO,TEL0157_ADDR);
    // record_benchmark_time(1);
    // get_benchmark_time(0,(uint32_t)&start);
    // get_benchmark_time(1,(uint32_t)&end);
    // printf("res %x, tel0157_deinit interval %llu\n",res,end-start);
    }
}

