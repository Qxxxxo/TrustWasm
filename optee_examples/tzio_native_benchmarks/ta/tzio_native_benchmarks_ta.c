/*
 * Copyright (c) 2016, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdlib.h>
#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include "tzio.h"
#include <tzio_native_benchmarks_ta.h>
#include "fall_detection.h"
#include "gnss_track.h"
#include "env_monitor.h"
#include "bme280.h"
#include "fingerprint.h"
#include "trusted_warning.h"
#include "trusted_flash.h"
#include "lz4.h"

#define AT24CXX_ADDR 0x50
#define AT24CXX_I2C_NO  0
#define AT24CXX_MEM_ADDR 0x00
#define AT24CXX_TEST_LEN 4*1024
#define ATK301_ADDR 0x27
#define ATK301_I2C_NO   0
#define ATK301_TEST_LEN  12800
#define BH1750FVI_ADDR 0x23
#define BH1750FVI_I2C_NO   1
#define BME280_ADDR		0x77
#define BME280_I2C_NO 1
#define BME280_DATA_LEN		8
#define DHT_GPIO_NO 22
#define DHT_TYPE 0
#define GPIO_TEST_NO 26
#define GPIO_FSEL_OUTPUT 1
#define GPIO_FSEL_INPUT  0
#define I2C_TEST_NO 0
#define I2C_TEST_ADDR 0x55
#define I2C_TEST_FLAG 0
#define I2C_TEST_LEN 4*1024
#define MPU6050_ADDR 0x68
#define MPU6050_I2C_NO  1
#define MPU6050_TEST_LEN 14
#define SECURE_STORAGE_OBJECT_ID "test"
#define SECURE_STORAGE_OBJECT_ID_LEN 4
#define SECURE_STORAGE_OBJECT_LEN 4*1024
#define SPI_TEST_NO 0
#define SPI_TEST_CS_NO 1
#define SPI_TEST_CONT 0
#define SPI_TEST_MODE 0
#define SPI_TEST_FREQ 0
#define SPI_TEST_LEN 4*1024
#define TEL0157_ADDR 0x20
#define TEL0157_I2C_NO 1
#define TEL0157_GNSS_MODE 7
#define TEL0157_RGB 1

#define FALL_DETECTION_ITERATIONS 50
#define GNSS_TRACK_ITERATIONS 25
#define ENV_MONITOR_ITERATIONS 50
#define FINGERPRINT_ITERATIONS	5
#define TRUSTED_WARNING_ITERATIONS 50
#define TRUSTED_FLASH_BENCHMARK_ITERATIONS 25

TEE_Result TA_CreateEntryPoint(void)
{
	DMSG("has been called");
	return TEE_SUCCESS;
}

void TA_DestroyEntryPoint(void)
{
	DMSG("has been called");
}

TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
		TEE_Param __maybe_unused params[4],
		void __maybe_unused **sess_ctx)
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);
	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;
	return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void __maybe_unused *sess_ctx)
{
	IMSG("Goodbye!\n");
}

static TEE_Result run_native_funcs(uint32_t param_types,
						   TEE_Param params[4]){
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);
	if(param_types!=exp_param_types){
		return TEE_ERROR_BAD_PARAMETERS;
	}
	for(uint32_t i=0;i<params[0].value.a;i++){
		int res;
		uint64_t start,end;
		uint8_t * data;
		// AT24CXX native funcs
		data = (uint8_t*)malloc(AT24CXX_TEST_LEN);
		record_benchmark_time(0);
		res=at24cxx_write(AT24CXX_I2C_NO,AT24CXX_ADDR,AT24CXX_MEM_ADDR,data,AT24CXX_TEST_LEN);
		record_benchmark_time(1);
		get_benchmark_time(0, &start);
		get_benchmark_time(1, &end);
		EMSG("res %x, at24cxx_write (4K data) interval %lu\n",res,end-start);

		record_benchmark_time(0);
		res=at24cxx_read(AT24CXX_I2C_NO,AT24CXX_ADDR,AT24CXX_MEM_ADDR,data,AT24CXX_TEST_LEN);
		record_benchmark_time(1);
		get_benchmark_time(0, &start);
		get_benchmark_time(1, &end);
		EMSG("res %x, at24cxx_read (4K data) interval %lu\n",res,end-start);
		free(data);

		// ATK301 native funcs
		record_benchmark_time(0);
		res=atk301_init(ATK301_I2C_NO,ATK301_ADDR);
		record_benchmark_time(1);
		get_benchmark_time(0,&start);
		get_benchmark_time(1,&end);
		EMSG("res %x, atk301_init interval %lu\n",res,end-start);

		record_benchmark_time(0);
		res=atk301_get_fingerprint_image(ATK301_I2C_NO,ATK301_ADDR);
		record_benchmark_time(1);
		get_benchmark_time(0, &start);
		get_benchmark_time(1, &end);
		EMSG("res %x, atk301_get_fingerprint_image interval %lu\n",res,end-start);

		data=(uint8_t*)malloc(ATK301_TEST_LEN);
		record_benchmark_time(0);
		res=atk301_upload_fingerprint_image(ATK301_I2C_NO,ATK301_ADDR,data);
		record_benchmark_time(1);
		get_benchmark_time(0, &start);
		get_benchmark_time(1, &end);
		EMSG("res %x, atk301_upload_fingerprint_image interval %lu\n",res,end-start);
		free(data);

		// BH1750FVI native funcs
		record_benchmark_time(0);
		res=bh1750fvi_init(BH1750FVI_I2C_NO,BH1750FVI_ADDR);
		record_benchmark_time(1);
		get_benchmark_time(0, &start);
		get_benchmark_time(1, &end);
		EMSG("res %x, bh1750fvi_init interval %lu\n",res,end-start);

		uint16_t lux=0;
		record_benchmark_time(0);
		res=bh1750fvi_sample(BH1750FVI_I2C_NO,BH1750FVI_ADDR,&lux);
		record_benchmark_time(1);
		get_benchmark_time(0, &start);
		get_benchmark_time(1, &end);
		EMSG("res %x, bh1750fvi_sample interval %lu\n",res,end-start);

		// DHT native funcs
		record_benchmark_time(0);
		res=dht_init(DHT_GPIO_NO,DHT_TYPE);
		record_benchmark_time(1);
		get_benchmark_time(0, &start);
		get_benchmark_time(1, &end);
		EMSG("res %x, dht_init interval %lu\n",res,end-start);

		int16_t temp=0;
		int16_t hum=0;
		record_benchmark_time(0);
		res=dht_read(DHT_GPIO_NO,DHT_TYPE,&temp,&hum);
		record_benchmark_time(1);
		get_benchmark_time(0, &start);
		get_benchmark_time(1, &end);
		EMSG("res %x, dht_read interval %lu\n",res,end-start);

		// MPU6050 native funcs
		record_benchmark_time(0);
		res=mpu6050_init(MPU6050_I2C_NO,MPU6050_ADDR);
		record_benchmark_time(1);
		get_benchmark_time(0, &start);
		get_benchmark_time(1, &end);
		EMSG("res %x, mpu6050_init interval %lu\n",res,end-start);

		data=(uint8_t *)malloc(MPU6050_TEST_LEN);
		record_benchmark_time(0);
		res=mpu6050_sample(MPU6050_I2C_NO,MPU6050_ADDR,data);
		record_benchmark_time(1);
		get_benchmark_time(0, &start);
		get_benchmark_time(1, &end);
		EMSG("res %x, mpu6050_sample interval %lu\n",res,end-start);

		// Secure storage native funcs
		char id[SECURE_STORAGE_OBJECT_ID_LEN]=SECURE_STORAGE_OBJECT_ID;
		data=(uint8_t*)malloc(SECURE_STORAGE_OBJECT_LEN);
		record_benchmark_time(0);
		res=write_secure_object(id,SECURE_STORAGE_OBJECT_ID_LEN,data,SECURE_STORAGE_OBJECT_LEN);
		record_benchmark_time(1);
		get_benchmark_time(0, &start);
		get_benchmark_time(1, &end);
		EMSG("res %x, write_secure_object interval %lu\n",res,end-start);

		uint32_t len=SECURE_STORAGE_OBJECT_LEN;
		record_benchmark_time(0);
		res=read_secure_object(id,SECURE_STORAGE_OBJECT_ID_LEN,data,&len);
		record_benchmark_time(1);
		get_benchmark_time(0, &start);
		get_benchmark_time(1, &end);
		EMSG("res %x, read_secure_object interval %lu\n",res,end-start);
		free(data);

		char new_id[SECURE_STORAGE_OBJECT_ID_LEN]=SECURE_STORAGE_OBJECT_ID;
		record_benchmark_time(0);
		res=delete_secure_object(new_id,SECURE_STORAGE_OBJECT_ID_LEN);
		record_benchmark_time(1);
		get_benchmark_time(0, &start);
		get_benchmark_time(1, &end);
		EMSG("res %x, delete_secure_object interval %lu\n",res,end-start);

		// TEL0157 native funcs
		record_benchmark_time(0);
		res=tel0157_init(TEL0157_I2C_NO,TEL0157_ADDR,TEL0157_GNSS_MODE, TEL0157_RGB);
		record_benchmark_time(1);
		get_benchmark_time(0, &start);
		get_benchmark_time(1, &end);
		EMSG("res %x, tel0157_init interval %lu\n",res,end-start);

		tel0157_time_t time;
		record_benchmark_time(0);
		res=tel0157_get_utc_time(TEL0157_I2C_NO,TEL0157_ADDR,&time);
		record_benchmark_time(1);
		get_benchmark_time(0, &start);
		get_benchmark_time(1, &end);
		EMSG("res %x, tel0157_get_utc_time interval %lu\n",res,end-start);

		uint8_t gnss_mode;
		record_benchmark_time(0);
		res=tel0157_get_gnss_mode(TEL0157_I2C_NO,TEL0157_ADDR,&gnss_mode);
		record_benchmark_time(1);
		get_benchmark_time(0, &start);
		get_benchmark_time(1, &end);
		EMSG("res %x, tel0157_get_gnss_mode interval %lu\n",res,end-start);

		uint8_t gnss_num_sat_used;
		record_benchmark_time(0);
		res=tel0157_get_num_sat_used(TEL0157_I2C_NO,TEL0157_ADDR,&gnss_num_sat_used);
		record_benchmark_time(1);
		get_benchmark_time(0, &start);
		get_benchmark_time(1, &end);
		EMSG("res %x, tel0157_get_num_sat_used interval %lu\n",res,end-start);

		tel0157_lon_lat_t lon_lat;

		record_benchmark_time(0);
		res=tel0157_get_lon(TEL0157_I2C_NO,TEL0157_ADDR,&lon_lat);
		record_benchmark_time(1);
		get_benchmark_time(0, &start);
		get_benchmark_time(1, &end);
		EMSG("res %x, tel0157_get_lon interval %lu\n",res,end-start);

		record_benchmark_time(0);
		res=tel0157_get_lon(TEL0157_I2C_NO,TEL0157_ADDR,&lon_lat);
		record_benchmark_time(1);
		get_benchmark_time(0, &start);
		get_benchmark_time(1, &end);
		EMSG("res %x, tel0157_get_lat interval %lu\n",res,end-start);

		uint8_t alt[3];
		record_benchmark_time(0);
		res=tel0157_get_alt(TEL0157_I2C_NO,TEL0157_ADDR,alt);
		record_benchmark_time(1);
		get_benchmark_time(0, &start);
		get_benchmark_time(1, &end);
		EMSG("res %x, tel0157_get_alt interval %lu\n",res,end-start);

		uint8_t sog[3];
		record_benchmark_time(0);
		res=tel0157_get_sog(TEL0157_I2C_NO,TEL0157_ADDR,sog);
		record_benchmark_time(1);
		get_benchmark_time(0, &start);
		get_benchmark_time(1, &end);
		EMSG("res %x, tel0157_get_sog interval %lu\n",res,end-start);

		uint8_t cog[3];
		record_benchmark_time(0);
		res=tel0157_get_cog(TEL0157_I2C_NO,TEL0157_ADDR,cog);
		record_benchmark_time(1);
		get_benchmark_time(0, &start);
		get_benchmark_time(1, &end);
		EMSG("res %x, tel0157_get_cog interval %lu\n",res,end-start);

		uint16_t gnss_len;
		record_benchmark_time(0);
		res=tel0157_get_gnss_len(TEL0157_I2C_NO,TEL0157_ADDR,&gnss_len);
		record_benchmark_time(1);
		get_benchmark_time(0, &start);
		get_benchmark_time(1, &end);
		EMSG("res %x, tel0157_get_gnss_len interval %lu\n",res,end-start);

		data=(uint8_t*)malloc(gnss_len);
		record_benchmark_time(0);
		res=tel0157_get_all_gnss(TEL0157_I2C_NO,TEL0157_ADDR,data,gnss_len);
		record_benchmark_time(1);
		get_benchmark_time(0, &start);
		get_benchmark_time(1, &end);
		EMSG("res %x, tel0157_get_all_gnss interval %lu\n",res,end-start);
		free(data);

		record_benchmark_time(0);
		res=tel0157_deinit(TEL0157_I2C_NO,TEL0157_ADDR);
		record_benchmark_time(1);
		get_benchmark_time(0, &start);
		get_benchmark_time(1, &end);
		EMSG("res %x, tel0157_deinit interval %lu\n",res,end-start);
	}
	return TEE_SUCCESS;
}


static TEE_Result run_i2c(uint32_t param_types,
						   TEE_Param params[4]){
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);
	if(param_types!=exp_param_types){
		return TEE_ERROR_BAD_PARAMETERS;
	}
	for(uint32_t i=0;i<params[0].value.a;i++){
		int res;
		uint64_t start,end;
		uint8_t * data;
		
		// I2C native funcs
		data = (uint8_t*)malloc(I2C_TEST_LEN);
		record_benchmark_time(0);
		res=i2c_write_bytes(I2C_TEST_NO,I2C_TEST_ADDR,I2C_TEST_FLAG,data,I2C_TEST_LEN);
		record_benchmark_time(1);
		get_benchmark_time(0,&start);
		get_benchmark_time(1,&end);
		EMSG("res %x, i2c_write_bytes (4K data) interval %lu\n",res,end-start);
		free(data);

		data = (uint8_t*)malloc(I2C_TEST_LEN);
		record_benchmark_time(0);
		res=i2c_read_bytes(I2C_TEST_NO,I2C_TEST_ADDR,I2C_TEST_FLAG,data,I2C_TEST_LEN);
		record_benchmark_time(1);
		get_benchmark_time(0,&start);
		get_benchmark_time(1,&end);
		EMSG("res %x, i2c_read_bytes (4K data) interval %lu\n",res,end-start);
		free(data);
	}
	return TEE_SUCCESS;
}

static TEE_Result run_spi(uint32_t param_types,
						   TEE_Param params[4]){
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);
	if(param_types!=exp_param_types){
		return TEE_ERROR_BAD_PARAMETERS;
	}
	for(uint32_t i=0;i<params[0].value.a;i++){
		int res;
		uint64_t start,end;
		uint8_t * src_data;
		uint8_t * dst_data;

		// I2C native funcs
		src_data = (uint8_t*)malloc(SPI_TEST_LEN);
		dst_data = (uint8_t*)malloc(SPI_TEST_LEN);
		record_benchmark_time(0);
		res=spi_transfer_bytes(SPI_TEST_NO,SPI_TEST_CS_NO,SPI_TEST_CONT,
								SPI_TEST_MODE,SPI_TEST_FREQ,SPI_TEST_LEN,SPI_TEST_LEN,
								src_data,dst_data);
		record_benchmark_time(1);
		get_benchmark_time(0,(uint32_t)&start);
		get_benchmark_time(1,(uint32_t)&end);
		EMSG("res %x, spi_transfer_bytes (4K data) interval %lu\n",res,end-start);
		free(src_data);
		free(dst_data); 
	}
	return TEE_SUCCESS;
}

static TEE_Result run_fall_detection(uint32_t param_types,
						   TEE_Param params[4]){
	int wait_ms = 1000/(int)FALL_SAMPLE_FREQ;
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);
	if(param_types!=exp_param_types){
		return TEE_ERROR_BAD_PARAMETERS;
	}
	for(uint32_t i=0;i<params[0].value.a;i++){
		uint64_t start,end;
		int res;
		int storage_res;
		int len=MPU6050_TEST_LEN;
		uint8_t data[MPU6050_TEST_LEN+1]={0};
		float ax, ay, az, gx, gy, gz,temp;
		float pitch, roll, yaw;
		float prev_pitch=0.0f, prev_roll=0.0f;
		float acc_magnitude;
		
		if (mpu6050_init(MPU6050_I2C_NO, MPU6050_ADDR) == 0)
		{
			// send async sample first
			record_benchmark_time(0);
			for(int i=0;i<FALL_DETECTION_ITERATIONS;i++){
				res=mpu6050_sample(MPU6050_I2C_NO,MPU6050_ADDR,data); 
				if(res>=0){
					// parse and record
					parse_mpu6050_data(data,&ax,&ay,&az,&gx,&gy,&gz,&temp);
					madgwick_ahrs_update(gx,gy,gz,ax,ay,az);
					get_euler_angles(&pitch, &roll, &yaw);
					data[MPU6050_TEST_LEN]=detect_fall(prev_pitch,prev_roll,pitch,roll,get_acceleration_magnitude(ax,ay,az));
					// async write mpu6050 data and detect result to storage here use eeprom
					at24cxx_write(AT24CXX_I2C_NO,AT24CXX_ADDR,0x00,data,MPU6050_TEST_LEN+1);
				}else{
					EMSG("sample failed with %x\n",res);
				}
				native_wait(wait_ms);
			}
			record_benchmark_time(1);
			get_benchmark_time(0,(uint32_t)&start);
			get_benchmark_time(1,(uint32_t)&end);
			EMSG("%lu\n",end-start);
		}else{
			EMSG("mpu6050 init failed\n");
		} 
	}
	return TEE_SUCCESS;
}

static TEE_Result run_gnss_track(int32_t param_types,
						   TEE_Param params[4]){
	int wait_ms = GNSS_TRACK_T*800;
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);
	if(param_types!=exp_param_types){
		return TEE_ERROR_BAD_PARAMETERS;
	}
	for(uint32_t i=0;i<params[0].value.a;i++){
		// run gndd track
		uint64_t start,end;
		tel0157_time_t time;
		tel0157_lon_lat_t lon_lat;
		uint8_t raw_alt[3];
		double longitude=0;
		double latitude=0;
		double altitude=0;
		double ax=0;
		double ay=0;
		double az=9.8;
		double gx=0;
		double gy=0; 
		double gz=0;
		double temp=0;
		uint8_t mpu_data[MPU6050_TEST_LEN]={0};
		double eccf_pos[3];
		char save_str[512]={0};
		ekf_t ekf = {0};
		gnss_track_init(&ekf);

		if (tel0157_init(TEL0157_I2C_NO, TEL0157_ADDR, TEL0157_GNSS_MODE, TEL0157_RGB) == 0&&mpu6050_init(MPU6050_I2C_NO,MPU6050_ADDR)==0)
		{
			// send async sample first
			record_benchmark_time(0);
			for(int i=0;i<GNSS_TRACK_ITERATIONS;i++){
				if(tel0157_get_utc_time(1,0x20,(uint32_t)&time)!=0){
					EMSG("tel0157 get utc time failed\n");
				}
				if(tel0157_get_lon(TEL0157_I2C_NO, TEL0157_ADDR,(uint32_t)&lon_lat)==0){
					longitude=(double)lon_lat.lonDD + (double)lon_lat.lonMM/60.0 + (double)lon_lat.lonMMMMM/100000.0/60.0;
				}else{
					EMSG("tel0157 get lon failed\n");
				}
				if(tel0157_get_lat(TEL0157_I2C_NO, TEL0157_ADDR,(uint32_t)&lon_lat)==0){
					latitude=(double)lon_lat.latDD + (double)lon_lat.latMM/60.0 + (double)lon_lat.latMMMMM/100000.0/60.0;
				}else{
					EMSG("tel0157 get lat failed\n");
				}
				if(tel0157_get_alt(TEL0157_I2C_NO, TEL0157_ADDR,(uint32_t)raw_alt)==0){
					altitude=(double)((uint16_t)(raw_alt[0]&0x7F)<<8|raw_alt[1])+(double)raw_alt[2]/100.0;
					if(raw_alt[0]&0x80){
						altitude=-altitude;
					}
				}else{
					EMSG("tel0157 get alt failed\n");
				}
				if(mpu6050_sample(MPU6050_I2C_NO,MPU6050_ADDR,mpu_data)==0){
					gnss_track_parse_mpu6050_data(mpu_data,&ax,&ay,&az,&gx,&gy,&gz,&temp);
				}else{
					EMSG("mpu6050 sample failed\n");
				}
				gnss_track_latlon_to_ecef(latitude,longitude,altitude,eccf_pos);
				// Run our model to get the EKF inputs
				double fx[15] = {0};
				double F[15*15] = {0};
				double hx[3] = {0};
				double H[3*15] = {0};
				gnss_track_run_model(&ekf,ax,ay,az,gx,gy,gz,fx,F,hx,H);
				
				// Run the EKF prediction step
				ekf_predict(&ekf, fx, F, Q);

				// Run the EKF update step
				ekf_update(&ekf,eccf_pos, hx, H, R);

				// save time & state
				sprintf(save_str,"UTC %d/%d/%d %d:%d:%d, x:%.2f, y:%.2f, z:%.2f, vx: %.2f, vy:%.2f, vz:%.2f, pitch:%.2f, roll:%.2f, yaw: %.2f",
								time.year,time.month,time.date,time.hour,time.minute,time.second,
								ekf.x[0],ekf.x[1],ekf.x[2],ekf.x[3],ekf.x[4],ekf.x[5],ekf.x[6],ekf.x[7],ekf.x[8]);
				at24cxx_write(AT24CXX_I2C_NO,AT24CXX_ADDR,0x00,save_str,512);
				native_wait(wait_ms);
			}
			record_benchmark_time(1);
			get_benchmark_time(0,(uint32_t)&start);
			get_benchmark_time(1,(uint32_t)&end);
			EMSG("%llu\n",end-start);
		}else{
			EMSG("tel0157 init failed\n");
		}
	}
	return TEE_SUCCESS;
}


static TEE_Result run_env_monitor(int32_t param_types,
						   TEE_Param params[4]){
	int wait_ms = 1;
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);
	if(param_types!=exp_param_types){
		return TEE_ERROR_BAD_PARAMETERS;
	}
	for(uint32_t i=0;i<params[0].value.a;i++){
		uint64_t start,end;
		uint16_t lux;
		int16_t dht11_temp_raw;
		int16_t dht11_hum_raw;
		float dht11_temp;
		float dht11_hum;
		float bme280_hum;
		float bme280_pressure;
		float bme280_temp;
		float mpu_temp;
		uint8_t mpu_data[MPU6050_TEST_LEN]={0};
		uint8_t bme280_data[BME280_DATA_LEN]={0};
		char save_str[256]={0};
		bme280_calibration_data_t bme280_calib_data;
		int32_t t_fine;
		// observation vector
		float z[_EKF_M]={0};
		// process model is f(x)=x
		float fx[_EKF_N]={0};
		// measurement function
		float hx[_EKF_M]={0};

		_ekf_t ekf = {0};
		env_monitor_init(&ekf);
		int res=mpu6050_init(MPU6050_I2C_NO,MPU6050_ADDR);
		if(res!=0){
			EMSG("mpu6050 init failed with %x\n",res);
		}
		res=bh1750fvi_init(BH1750FVI_I2C_NO,BH1750FVI_ADDR);
		if(res!=0){
			EMSG("bh1750fvi init failed with %x\n",res);
		}
		res=bme280_init(BME280_I2C_NO,BME280_ADDR);
		if(res!=0){
			EMSG("bme280 init failed with %x\n",res);
		}
		res=dht_init(DHT_GPIO_NO,0);
		if(res==0){
			// get bme280 calibration data
			bme280_get_calibration(BME280_I2C_NO,BME280_ADDR,&bme280_calib_data);

			record_benchmark_time(0);
			for(int i=0;i<ENV_MONITOR_ITERATIONS;i++){
				if(bh1750fvi_sample(BH1750FVI_I2C_NO,BH1750FVI_ADDR,&lux)!=0){
					EMSG("bh1750fvi sample failed\n");
				}
				if(mpu6050_sample(MPU6050_I2C_NO,MPU6050_ADDR,mpu_data)==0){
					env_monitor_parse_mpu6050_temp(mpu_data,&mpu_temp);
				}else{
					EMSG("mpu6050 sample failed\n");
				}
				if(bme280_sample(BME280_I2C_NO,BME280_ADDR,bme280_data)==0){
					bme280_temp=bme280_convert_temperature(bme280_data,&bme280_calib_data,&t_fine);
					bme280_pressure=bme280_convert_pressure(bme280_data,&bme280_calib_data,t_fine);
					bme280_hum=bme280_convert_humidity(bme280_data,&bme280_calib_data,t_fine);
				}else{
					EMSG("bme280 sample failed\n");
				}
				if(dht_read(DHT_GPIO_NO,0,&dht11_temp_raw,&dht11_hum_raw)!=0){
					// dht read error 
					dht11_temp=24.3;
					dht11_hum=30.0;
				}else{
					dht11_temp=(float)dht11_temp_raw/10+ (float)(dht11_temp_raw%10)/10.0;
					dht11_hum=(float)dht11_hum_raw/10+ (float)(dht11_hum_raw%10)/10.0;
				}
				// fusion data
				z[0]=(float)lux;
				z[1]=bme280_pressure;
				z[2]=bme280_temp;
				z[3]=bme280_hum;
				z[4]=mpu_temp;
				z[5]=dht11_temp;
				z[6]=dht11_hum;
				
				// process model
				fx[0]=ekf.x[0];
				fx[1]=ekf.x[1];
				fx[2]=ekf.x[2];
				fx[3]=ekf.x[3];

				_ekf_predict(&ekf,fx,_F,_Q);
				
				hx[0]=ekf.x[0];
				hx[1]=ekf.x[1];
				hx[2]=ekf.x[2];
				hx[3]=ekf.x[3];
				hx[4]=ekf.x[2];
				hx[5]=ekf.x[2];
				hx[6]=ekf.x[3];

				_ekf_update(&ekf, z, hx, _H, _R);

				int len=sprintf(save_str,"illumilance: %.2f lux, pressure: %.2f pa, temperature: %.2f °C, humidity %.2f\n",
						ekf.x[0],ekf.x[1],ekf.x[2],ekf.x[3]);
				
				if(len>0){
					char obj_id[ENV_MONITOR_SECURE_OBJECT_ID_LEN]=ENV_MONITOR_SEUCRE_OBJECT_ID;
					res=write_secure_object(obj_id,ENV_MONITOR_SECURE_OBJECT_ID_LEN,save_str,len);
					if(res!=0){
						EMSG("write secure object failed with %x\n",res);
					}
				}
				native_wait(wait_ms);
			}
			record_benchmark_time(1);
			get_benchmark_time(0,(uint32_t)&start);
			get_benchmark_time(1,(uint32_t)&end);
			EMSG("%llu\n",end-start);
		}else{
			EMSG("init failed with %x\n",res);
		}
	}
	return TEE_SUCCESS;
}

// alert: atk301 connected on i2c 1
static TEE_Result run_fingerprint(int32_t param_types,
						   TEE_Param params[4]){
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);
	if(param_types!=exp_param_types){
		return TEE_ERROR_BAD_PARAMETERS;
	}
	// EMSG("fingerprint start");
	for(uint32_t i=0;i<params[0].value.a;i++){
		int res;
		uint64_t start,end;
		double kernel[GAUSSIAN_KERNEL_SIZE*GAUSSIAN_KERNEL_SIZE]={0.0};
		int erode_kernel[ERODE_KERNEL_SIZE*ERODE_KERNEL_SIZE]={0};
		int dilate_kernel[DILATE_KERNEL_SIZE*DILATE_KERNEL_SIZE]={0};
		uint8_t src_fp_img[ATK301_IMG_DATA_LEN]={0};
		uint8_t dst_fp_img[ATK301_IMG_EXT_DATA_LEN]={0};
		if(atk301_init(FINGERPRINT_ATK301_I2C_NO, ATK301_ADDR)==0){
			record_benchmark_time(0);
			for(int i=0;i<FINGERPRINT_ITERATIONS;i++){
				atk301_get_fingerprint_image(FINGERPRINT_ATK301_I2C_NO,ATK301_ADDR);
				res=atk301_upload_fingerprint_image(FINGERPRINT_ATK301_I2C_NO,ATK301_ADDR,src_fp_img);
				if(res<0){
					EMSG("upload fingerprint image failed\n");
				}
				// print_fingerprint_image(fp_img,ATK301_IMG_DATA_LEN);
				extend_fp_img(src_fp_img, dst_fp_img);
				// EMSG("extend fp img");
				generate_gaussian_kernel(GAUSSIAN_KERNEL_SIZE,GAUSSIAN_KERNEL_SIGMA,kernel);
				// EMSG("generate gaussian kernel");
				generate_circular_kernel(ERODE_KERNEL_SIZE,erode_kernel);
				generate_circular_kernel(DILATE_KERNEL_SIZE,dilate_kernel);
				// EMSG("generate circular kernel");
				gaussian_blur(dst_fp_img,ATK301_IMG_WIDTH,ATK301_IMG_HEIGHT,kernel,GAUSSIAN_KERNEL_SIZE);
				// EMSG("gaussian blur");
				otsu_threshold(dst_fp_img,ATK301_IMG_WIDTH,ATK301_IMG_HEIGHT);
				// EMSG("ostu threshold");
				dilate(dst_fp_img,ATK301_IMG_WIDTH,ATK301_IMG_HEIGHT,dilate_kernel,DILATE_KERNEL_SIZE);
				// EMSG("dilate");
				erode(dst_fp_img,ATK301_IMG_WIDTH,ATK301_IMG_HEIGHT,erode_kernel,ERODE_KERNEL_SIZE);
				// EMSG("erode");
				res=at24cxx_write(AT24CXX_I2C_NO,AT24CXX_ADDR,0x00,dst_fp_img,ATK301_IMG_EXT_DATA_LEN);
				if(res<0){
					EMSG("failed to write\n");
				}
			}
			record_benchmark_time(1);
			get_benchmark_time(0,&start);
			get_benchmark_time(1,&end);
			EMSG("%llu\n",end-start);
		}else{
			EMSG("init failed with %x\n",res);
		}
	}
	return TEE_SUCCESS;
}

static TEE_Result run_trusted_warning(int32_t param_types,
						   TEE_Param params[4]){
	int wait_ms=50;
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);
	if(param_types!=exp_param_types){
		return TEE_ERROR_BAD_PARAMETERS;
	}
	for(uint32_t i=0;i<params[0].value.a;i++){
		int res;
		uint64_t start,end;
		float bme280_hum;
		float bme280_pressure;
		float bme280_temp;
		uint8_t bme280_data[BME280_DATA_LEN]={0};
		bme280_calibration_data_t bme280_calib_data;
		int32_t t_fine;
	
		res=bme280_init(BME280_I2C_NO,BME280_ADDR);
		if(res!=0){
			EMSG("bme280 init failed with %x\n",res);
		}
		gpio_init(LED_GPIO_GREEN_PIN,GPIO_MODE_OUTPUT);
		gpio_init(LED_GPIO_RED_PIN,GPIO_MODE_OUTPUT);

		if(res==0){
			// get bme280 calibration data
			bme280_get_calibration(BME280_I2C_NO,BME280_ADDR,&bme280_calib_data);
			record_benchmark_time(0);
			for(int i=0;i<TRUSTED_WARNING_ITERATIONS;i++){
				if(bme280_sample(BME280_I2C_NO,BME280_ADDR,bme280_data)==0){
					bme280_temp=bme280_convert_temperature(bme280_data,&bme280_calib_data,&t_fine);
					bme280_pressure=bme280_convert_pressure(bme280_data,&bme280_calib_data,t_fine);
					bme280_hum=bme280_convert_humidity(bme280_data,&bme280_calib_data,t_fine);
				}else{
					EMSG("bme280 sample failed\n");
				}
				gpio_set(LED_GPIO_RED_PIN,GPIO_LOW_LEVEL);
				gpio_set(LED_GPIO_GREEN_PIN,GPIO_LOW_LEVEL);
				if(bme280_temp>ALERT_TEMP||bme280_hum>ALERT_HUM||bme280_pressure>ALERT_PRESSURE){
					// alert and record time
					gpio_set(LED_GPIO_RED_PIN,GPIO_HIGH_LEVEL);
				}else{
					gpio_set(LED_GPIO_GREEN_PIN,GPIO_HIGH_LEVEL);
				}
				native_wait(wait_ms);
			}
			record_benchmark_time(1);
			get_benchmark_time(0,(uint32_t)&start);
			get_benchmark_time(1,(uint32_t)&end);
			EMSG("%llu\n",end-start);
		}else{
			EMSG("init failed with %x\n",res);
		}
	}
	return TEE_SUCCESS;
}

static TEE_Result run_trusted_flash(int32_t param_types,
						   TEE_Param params[4]){
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);
	if(param_types!=exp_param_types){
		return TEE_ERROR_BAD_PARAMETERS;
	}
	for(uint32_t i=0;i<params[0].value.a;i++){
		int res=0;
		uint64_t start,end;
		char * buf=(char *)malloc(LZ4_FRAME_SIZE);
		double kernel[TRUSTED_FLASH_GAUSSIAN_KERNEL_SIZE*TRUSTED_FLASH_GAUSSIAN_KERNEL_SIZE]={0.0};
		int erode_kernel[TRUSTED_FLASH_ERODE_KERNEL_SIZE*TRUSTED_FLASH_ERODE_KERNEL_SIZE]={0};
		int dilate_kernel[TRUSTED_FLASH_DILATE_KERNEL_SIZE*TRUSTED_FLASH_DILATE_KERNEL_SIZE]={0};
		char * out_buf=(char *)malloc(LZ4_FRAME_SIZE);
		record_benchmark_time(0);
		for(int i=0;i<TRUSTED_FLASH_BENCHMARK_ITERATIONS;i++){
			res=i2c_read_bytes(1,I2C_TEST_ADDR,0,buf,LZ4_FRAME_SIZE);
			if(res<0){
				EMSG("i2c read failed with %x\n",res);
			}
			// generate kernels
			generate_gaussian_kernel(TRUSTED_FLASH_GAUSSIAN_KERNEL_SIZE,TRUSTED_FLASH_GAUSSIAN_KERNEL_SIGMA,kernel);
			generate_circular_kernel(TRUSTED_FLASH_ERODE_KERNEL_SIZE,erode_kernel);
			generate_circular_kernel(TRUSTED_FLASH_DILATE_KERNEL_SIZE,dilate_kernel);			
			// process img
			gaussian_blur(buf,FRAME_WIDTH,FRAME_HEIGHT,kernel,TRUSTED_FLASH_GAUSSIAN_KERNEL_SIZE);
			otsu_threshold(buf,FRAME_WIDTH,FRAME_HEIGHT);
			dilate(buf,FRAME_WIDTH,FRAME_HEIGHT,dilate_kernel,TRUSTED_FLASH_DILATE_KERNEL_SIZE);
			erode(buf,FRAME_WIDTH,FRAME_HEIGHT,erode_kernel,TRUSTED_FLASH_ERODE_KERNEL_SIZE);
			// compress
			int max_compressed_size=LZ4_compressBound(LZ4_FRAME_SIZE);
			int compressed_size=LZ4_compress_default(buf,out_buf,LZ4_FRAME_SIZE,max_compressed_size);
			native_wait(10);
		}
		record_benchmark_time(1);
		get_benchmark_time(0,(uint32_t)&start);
		get_benchmark_time(1,(uint32_t)&end);
		EMSG("%llu\n",end-start);
		free(buf);
		free(out_buf);
	}
	return TEE_SUCCESS;
}

TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
			uint32_t cmd_id,
			uint32_t param_types, TEE_Param params[4])
{
	switch (cmd_id) {
	case TA_TZIO_NATIVE_BENCHMARKS_CMD_RUN_NATIVE_FUNCS:
		return run_native_funcs(param_types, params);
	case TA_TZIO_NATIVE_BENCHMARKS_CMD_RUN_I2C:
		return run_i2c(param_types, params);
	case TA_TZIO_NATIVE_BENCHMARKS_CMD_RUN_SPI:
		return run_spi(param_types, params);
	case TA_TZIO_NATIVE_BENCHMARKS_CMD_RUN_FALL_DETECTION:
		return run_fall_detection(param_types, params);
	case TA_TZIO_NATIVE_BENCHMARKS_CMD_RUN_GNSS_TRACK:
		return run_gnss_track(param_types, params);
	case TA_TZIO_NATIVE_BENCHMARKS_CMD_RUN_ENV_MONITOR:
		return run_env_monitor(param_types,params);
	case TA_TZIO_NATIVE_BENCHMARKS_CMD_RUN_FINGERPRINT:
		return run_fingerprint(param_types,params);
	case TA_TZIO_NATIVE_BENCHMARKS_CMD_RUN_TRUSTED_WARNING:
		return run_trusted_warning(param_types,params);
	case TA_TZIO_NATIVE_BENCHMARKS_CMD_RUN_TRUSTED_FLASH:
		return run_trusted_flash(param_types,params);
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
