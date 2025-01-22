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
#include <trusted_flash_ta.h>
#include "trusted_flash.h"
#include "fingerprint.h"
#include "lz4.h"

#define I2C_TEST_ADDR 0x55
#define I2C_TEST_FLAG 0

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
	case TA_TRUSTED_FLASH_CMD_RUN_TRUSTED_FLASH:
		return run_trusted_flash(param_types,params);
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
