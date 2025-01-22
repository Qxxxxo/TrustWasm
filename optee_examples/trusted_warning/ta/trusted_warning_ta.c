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
#include <trusted_warning_ta.h>
#include "bme280.h"
#include "trusted_warning.h"

#define BME280_ADDR		0x77
#define BME280_I2C_NO 1
#define BME280_DATA_LEN		8

#define TRUSTED_WARNING_ITERATIONS 50


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

TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
			uint32_t cmd_id,
			uint32_t param_types, TEE_Param params[4])
{
	switch (cmd_id) {
	case TA_TRUSTED_WARNING_CMD_RUN_TRUSTED_WARNING:
		return run_trusted_warning(param_types,params);
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
