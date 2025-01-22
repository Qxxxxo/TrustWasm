/**
 * @brief wasm trusted flash
 * LZ4
 * https://github.com/lz4/lz4
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include <time.h>
#include "native_funcs.h"
#include "lz4.h"
#include "img_process.h"

#define TEST_I2C_NO 1
#define TEST_I2C_ADDR 0x55
#define LZ4_FRAME_SIZE	1024
#define FRAME_WIDTH	32
#define FRAME_HEIGHT 32
#define GAUSSIAN_KERNEL_SIZE	5
#define GAUSSIAN_KERNEL_SIGMA	0.8
#define ERODE_KERNEL_SIZE	3
#define DILATE_KERNEL_SIZE	3

#define BENCHMARK_ITERATIONS 25
#define RUN_BENCHMARK_TIMES	1
#define VARY_LEN	6

int main(int argc, char **argv)
{
	for(int k=0;k<RUN_BENCHMARK_TIMES;k++){
		for(int size=1;size<=VARY_LEN;size++){
			int res=0;
			int hint=size*size;
			int total=LZ4_FRAME_SIZE*size*size;
			int width=FRAME_WIDTH*size;
			int height=FRAME_HEIGHT*size;
			if(size==7){
				hint=50;
				total=LZ4_FRAME_SIZE*hint;
				width=FRAME_WIDTH*10;
				height=FRAME_HEIGHT*5;
			}
			uint64_t start,end;
			char * buf=(char *)malloc(total);
			double kernel[GAUSSIAN_KERNEL_SIZE*GAUSSIAN_KERNEL_SIZE]={0.0};
			int erode_kernel[ERODE_KERNEL_SIZE*ERODE_KERNEL_SIZE]={0};
			int dilate_kernel[DILATE_KERNEL_SIZE*DILATE_KERNEL_SIZE]={0};
			char * out_buf=(char *)malloc(total);
			uint32_t signed_buf=sign_ptr(buf,SIGNATURE,TAG);
			uint32_t signed_out_buf=sign_ptr(buf,SIGNATURE,TAG);
			record_benchmark_time(0);
			for(int i=0;i<BENCHMARK_ITERATIONS;i++){
				res=i2c_read_bytes(TEST_I2C_NO,TEST_I2C_ADDR,0,signed_buf,total);
				if(res<0){
					printf("i2c read failed with %x\n",res);
				}
				// generate kernels
				generate_gaussian_kernel(GAUSSIAN_KERNEL_SIZE,GAUSSIAN_KERNEL_SIGMA,kernel);
				generate_circular_kernel(ERODE_KERNEL_SIZE,erode_kernel);
				generate_circular_kernel(DILATE_KERNEL_SIZE,dilate_kernel);			
				// process img
				gaussian_blur(buf,width,height,kernel,GAUSSIAN_KERNEL_SIZE);
				otsu_threshold(buf,width,height);
				dilate(buf,width,height,dilate_kernel,DILATE_KERNEL_SIZE);
				erode(buf,width,height,erode_kernel,ERODE_KERNEL_SIZE);
				// compress
				int max_compressed_size=LZ4_compressBound(total);
				int compressed_size=LZ4_compress_default(buf,out_buf,total,max_compressed_size);
				native_wait(10);
			}
			record_benchmark_time(1);
			get_benchmark_time(0,(uint32_t)&start);
			get_benchmark_time(1,(uint32_t)&end);
			printf("%dK,%llu\n",hint,end-start);
			free(buf);
			free(out_buf);
		}
	}
    return 0;
}

