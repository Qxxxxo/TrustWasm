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
#include <fingerprint_ta.h>
#include "fingerprint.h"

#define AT24CXX_ADDR 0x50
#define AT24CXX_I2C_NO  0
#define ATK301_ADDR 0x27

#define FINGERPRINT_ITERATIONS	5

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

TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
			uint32_t cmd_id,
			uint32_t param_types, TEE_Param params[4])
{
	switch (cmd_id) {
	case TA_FINGERPRINT_CMD_RUN_FINGERPRINT:
		return run_fingerprint(param_types,params);
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
