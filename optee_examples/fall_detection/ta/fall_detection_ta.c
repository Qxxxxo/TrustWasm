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
#include <fall_detection_ta.h>
#include "fall_detection.h"

#define AT24CXX_ADDR 0x50
#define AT24CXX_I2C_NO  0
#define AT24CXX_MEM_ADDR 0x00
#define MPU6050_ADDR 0x68
#define MPU6050_I2C_NO  1
#define MPU6050_TEST_LEN 14

#define FALL_DETECTION_ITERATIONS 50

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


TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
			uint32_t cmd_id,
			uint32_t param_types, TEE_Param params[4])
{
	switch (cmd_id) {
	case TA_FALL_DETECTION_CMD_RUN_FALL_DETECTION:
		return run_fall_detection(param_types, params);
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
