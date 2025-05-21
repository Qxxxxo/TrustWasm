/**
 * @brief wasm trusted warning
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include "native_funcs.h"
#include "safe_bme280.h"

#define BME280_PERIPH_NO 29
#define BME280_I2C_NO 1
#define BME280_ADDR		0x77
#define BME280_DATA_LEN		8

#define LED_GPIO_GREEN_PIN 21
#define LED_GPIO_RED_PIN 26
#define GPIO_MODE_OUTPUT 1
#define GPIO_HIGH_LEVEL	1
#define GPIO_LOW_LEVEL 0

#define ALERT_TEMP 30.0f
#define ALERT_HUM 20.0f
#define ALERT_PRESSURE 1100.0f	

// #define BENCHMARK_ITERATIONS 50
// #define RUN_BENCHMARK_TIMES	10800
#define RUN_BENCHMARK_TIMES	86400
#define RECORD_DURATION	1000000

int wait_ms = 2;

int main(int argc, char **argv)
{
	for(int k=0;k<RUN_BENCHMARK_TIMES;k++){
		int samples=0;
		int res;
		int bme280_future=-1;
		uint64_t start,end;
		float bme280_hum;
		float bme280_pressure;
		float bme280_temp;
		uint8_t bme280_data[BME280_DATA_LEN]={0};
		bme280_calibration_data_t bme280_calib_data;
		int32_t t_fine;
		uint32_t bme_len=BME280_DATA_LEN;

		uint32_t signed_bme280_data=sign_ptr(bme280_data,SIGNATURE,TAG);
		uint32_t signed_bme_len=sign_ptr(&bme_len, SIGNATURE, TAG);
		res=bme280_init(BME280_I2C_NO,BME280_ADDR);
		if(res!=0){
			printf("bme280 init failed with %x\n",res);
		}
		gpio_init(LED_GPIO_GREEN_PIN,GPIO_MODE_OUTPUT);
		gpio_init(LED_GPIO_RED_PIN,GPIO_MODE_OUTPUT);

		// reset gpio
		gpio_set(LED_GPIO_GREEN_PIN,GPIO_LOW_LEVEL);
		gpio_set(LED_GPIO_RED_PIN,GPIO_LOW_LEVEL);

		if(res==0){
			// get bme280 calibration data
			bme280_get_calibration(BME280_I2C_NO,BME280_ADDR,&bme280_calib_data);

			record_benchmark_time(0);
			record_benchmark_time(1);
			get_benchmark_time(0,(uint32_t)&start);
			get_benchmark_time(1,(uint32_t)&end);
			// send async read first
			bme280_future=bme280_sample_async_future(BME280_I2C_NO, BME280_ADDR);
			while(end-start<=RECORD_DURATION){
				if(bme280_future>=0){
					res=get_async_data_future(BME280_PERIPH_NO,bme280_future,signed_bme280_data,signed_bme_len);
					if(res<0){
						printf("bme280 failed with %x\n",res);
					}else{
						bme280_temp=bme280_convert_temperature(bme280_data,&bme280_calib_data,&t_fine);
						bme280_pressure=bme280_convert_pressure(bme280_data,&bme280_calib_data,t_fine);
						bme280_hum=bme280_convert_humidity(bme280_data,&bme280_calib_data,t_fine);
					}
				}
				// next async send
				bme280_future=bme280_sample_async_future(BME280_I2C_NO, BME280_ADDR);
				
				gpio_set(LED_GPIO_RED_PIN,GPIO_LOW_LEVEL);
				gpio_set(LED_GPIO_GREEN_PIN,GPIO_LOW_LEVEL);
				if(bme280_temp>ALERT_TEMP||bme280_hum>ALERT_HUM||bme280_pressure>ALERT_PRESSURE){
					// alert and record time
					gpio_set(LED_GPIO_RED_PIN,GPIO_HIGH_LEVEL);
				}else{
					gpio_set(LED_GPIO_GREEN_PIN,GPIO_HIGH_LEVEL);
				}
				native_wait(wait_ms);
				samples++;
				record_benchmark_time(1);
				get_benchmark_time(1,(uint32_t)&end);
			}
			// consume last async data
			if(bme280_future>=0){
				res=get_async_data_future(BME280_PERIPH_NO,bme280_future,signed_bme280_data,signed_bme_len);
				if(res<0){
					printf("bme280 failed with %x\n",res);
				}
			}
			// samples++;
			printf("%llu\n",samples);
		}else{
			printf("init failed with %x\n",res);
		}

		// reset gpio
		gpio_set(LED_GPIO_GREEN_PIN,GPIO_LOW_LEVEL);
		gpio_set(LED_GPIO_RED_PIN,GPIO_LOW_LEVEL);
	}
    return 0;
}