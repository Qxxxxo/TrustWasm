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
#include <gnss_track_ta.h>
#include "gnss_track.h"

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

TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
			uint32_t cmd_id,
			uint32_t param_types, TEE_Param params[4])
{
	switch (cmd_id) {
	case TA_GNSS_TRACK_CMD_RUN_GNSS_TRACK:
		return run_gnss_track(param_types, params);
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
