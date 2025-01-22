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
#include <env_monitor_ta.h>
#include "env_monitor.h"
#include "bme280.h"

#define BH1750FVI_ADDR 0x23
#define BH1750FVI_I2C_NO   1
#define BME280_ADDR		0x77
#define BME280_I2C_NO 1
#define BME280_DATA_LEN		8
#define DHT_GPIO_NO 22
#define DHT_TYPE 0
#define MPU6050_ADDR 0x68
#define MPU6050_I2C_NO  1
#define MPU6050_TEST_LEN 14

#define ENV_MONITOR_ITERATIONS 50

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

TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
			uint32_t cmd_id,
			uint32_t param_types, TEE_Param params[4])
{
	switch (cmd_id) {
	case TA_ENV_MONITOR_CMD_RUN_ENV_MONITOR:
		return run_env_monitor(param_types,params);
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
