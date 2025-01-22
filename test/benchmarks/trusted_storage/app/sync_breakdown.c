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

#define BREAKDOWN_POINTS    4

void print_breakdown(uint64_t breakdown[BENCHMARK_ITERATIONS][BREAKDOWN_POINTS]){
    for(int i=0;i<BENCHMARK_ITERATIONS;i++){
        for(int j=0;j<BREAKDOWN_POINTS-1;j++){
            printf("%llu",breakdown[i][j+1]-breakdown[i][j]);
            if(j!=BREAKDOWN_POINTS-2){
                printf(",");
            }
        }
        printf("\n");
    }
}

int main(int argc, char **argv)
{
	for(int k=0;k<RUN_BENCHMARK_TIMES;k++){
		int res=0;
		uint64_t start,end;
        uint64_t breakdown[BENCHMARK_ITERATIONS][BREAKDOWN_POINTS];
		char * buf=(char *)malloc(LZ4_FRAME_SIZE);
		double kernel[GAUSSIAN_KERNEL_SIZE*GAUSSIAN_KERNEL_SIZE]={0.0};
		int erode_kernel[ERODE_KERNEL_SIZE*ERODE_KERNEL_SIZE]={0};
		int dilate_kernel[DILATE_KERNEL_SIZE*DILATE_KERNEL_SIZE]={0};
		char * out_buf=(char *)malloc(LZ4_FRAME_SIZE);
		uint32_t signed_buf=sign_ptr(buf,SIGNATURE,TAG);
		uint32_t signed_out_buf=sign_ptr(buf,SIGNATURE,TAG);
		record_benchmark_time(0);
		for(int i=0;i<BENCHMARK_ITERATIONS;i++){
            record_benchmark_time(2);
			res=i2c_read_bytes(TEST_I2C_NO,TEST_I2C_ADDR,0,signed_buf,LZ4_FRAME_SIZE);
			if(res<0){
				printf("i2c read failed with %x\n",res);
			}
            record_benchmark_time(3);
			// generate kernels
			generate_gaussian_kernel(GAUSSIAN_KERNEL_SIZE,GAUSSIAN_KERNEL_SIGMA,kernel);
			generate_circular_kernel(ERODE_KERNEL_SIZE,erode_kernel);
			generate_circular_kernel(DILATE_KERNEL_SIZE,dilate_kernel);			
			// process img
			gaussian_blur(buf,FRAME_WIDTH,FRAME_HEIGHT,kernel,GAUSSIAN_KERNEL_SIZE);
			otsu_threshold(buf,FRAME_WIDTH,FRAME_HEIGHT);
			dilate(buf,FRAME_WIDTH,FRAME_HEIGHT,dilate_kernel,DILATE_KERNEL_SIZE);
			erode(buf,FRAME_WIDTH,FRAME_HEIGHT,erode_kernel,ERODE_KERNEL_SIZE);
			// compress
			int max_compressed_size=LZ4_compressBound(LZ4_FRAME_SIZE);
			int compressed_size=LZ4_compress_default(buf,out_buf,LZ4_FRAME_SIZE,max_compressed_size);
            record_benchmark_time(4);
			native_wait(10);
            record_benchmark_time(5);
            get_benchmark_time(2,(uint32_t)&(breakdown[i][0]));
            get_benchmark_time(3,(uint32_t)&(breakdown[i][1]));
            get_benchmark_time(4,(uint32_t)&(breakdown[i][2]));
            get_benchmark_time(5,(uint32_t)&(breakdown[i][3]));
		}
		record_benchmark_time(1);
		get_benchmark_time(0,(uint32_t)&start);
		get_benchmark_time(1,(uint32_t)&end);
		printf("%llu\n",end-start);
        print_breakdown(breakdown);
		free(buf);
		free(out_buf);
	}
    return 0;
}

