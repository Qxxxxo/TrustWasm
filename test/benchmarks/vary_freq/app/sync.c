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

#define AT24CXX_I2C_NO 0
#define AT24CXX_ADDR 0x50
#define LZ4_FRAME_SIZE	1024
#define FRAME_WIDTH	32
#define FRAME_HEIGHT 32
#define GAUSSIAN_KERNEL_SIZE	5
#define GAUSSIAN_KERNEL_SIGMA	0.8
#define ERODE_KERNEL_SIZE	3
#define DILATE_KERNEL_SIZE	3

#define BENCHMARK_ITERATIONS 25
#define RUN_BENCHMARK_TIMES	1
#define VARY_LEN	4

int main(int argc, char **argv)
{
	for(int k=1;k<=RUN_BENCHMARK_TIMES;k++){
		for(int size=1;size<=VARY_LEN;size++){
			int res=0;
			uint64_t start,end;
			char * buf=(char *)malloc(LZ4_FRAME_SIZE*size*size);
			double kernel[GAUSSIAN_KERNEL_SIZE*GAUSSIAN_KERNEL_SIZE]={0.0};
			int erode_kernel[ERODE_KERNEL_SIZE*ERODE_KERNEL_SIZE]={0};
			int dilate_kernel[DILATE_KERNEL_SIZE*DILATE_KERNEL_SIZE]={0};
			char * out_buf=(char *)malloc(LZ4_FRAME_SIZE*size*size);
			record_benchmark_time(0);
			for(int i=0;i<BENCHMARK_ITERATIONS;i++){
				res=at24cxx_read(AT24CXX_I2C_NO,AT24CXX_ADDR,0x00,buf,LZ4_FRAME_SIZE*size*size);
				if(res<0){
					printf("at24cxx read failed with %x\n",res);
				}
				// generate kernels
				generate_gaussian_kernel(GAUSSIAN_KERNEL_SIZE,GAUSSIAN_KERNEL_SIGMA,kernel);
				generate_circular_kernel(ERODE_KERNEL_SIZE,erode_kernel);
				generate_circular_kernel(DILATE_KERNEL_SIZE,dilate_kernel);			
				// process img
				gaussian_blur(buf,FRAME_WIDTH*size,FRAME_HEIGHT*size,kernel,GAUSSIAN_KERNEL_SIZE);
				otsu_threshold(buf,FRAME_WIDTH*size,FRAME_HEIGHT*size);
				dilate(buf,FRAME_WIDTH*size,FRAME_HEIGHT*size,dilate_kernel,DILATE_KERNEL_SIZE);
				erode(buf,FRAME_WIDTH*size,FRAME_HEIGHT*size,erode_kernel,ERODE_KERNEL_SIZE);
				// compress
				int max_compressed_size=LZ4_compressBound(LZ4_FRAME_SIZE);
				int compressed_size=LZ4_compress_default(buf,out_buf,LZ4_FRAME_SIZE,max_compressed_size);
				// // write back
				// res=at24cxx_write(AT24CXX_I2C_NO,AT24CXX_ADDR,0x00,out_buf,compressed_size);
				// if(res<0){
				// 	printf("write back failed %x\n",res);
				// }
				native_wait(10);
			}
			record_benchmark_time(1);
			get_benchmark_time(0,(uint32_t)&start);
			get_benchmark_time(1,(uint32_t)&end);
			printf("%llu\n",end-start);
			free(buf);
			free(out_buf);
		}
	}
    return 0;
}

