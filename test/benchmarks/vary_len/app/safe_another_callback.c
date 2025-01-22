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
#define RUN_BENCHMARK_TIMES	15
#define VARY_LEN	5

int size;
char * buf;
double kernel[GAUSSIAN_KERNEL_SIZE*GAUSSIAN_KERNEL_SIZE]={0.0};
int erode_kernel[ERODE_KERNEL_SIZE*ERODE_KERNEL_SIZE]={0};
int dilate_kernel[DILATE_KERNEL_SIZE*DILATE_KERNEL_SIZE]={0};
char * out_buf;
uint32_t read_len;
uint32_t signed_buf;
uint32_t signed_out_buf;
uint32_t signed_read_len;

int handler(uint32_t signo, uint32_t sub_signo)
{
    int res=get_async_data(signo,sub_signo,signed_buf,signed_read_len);
    if(res<0){
		printf("get async data failed with %x\n",res);
	}else{
		// for test. record poll times
		record_poll_times(res);
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
	int max_compressed_size=LZ4_compressBound(LZ4_FRAME_SIZE*size*size);
	int compressed_size=LZ4_compress_default(buf,out_buf,LZ4_FRAME_SIZE*size*size,max_compressed_size);
	res=i2c_write_bytes(TEST_I2C_NO,TEST_I2C_ADDR,0,signed_out_buf,compressed_size);
	if(res<0){
		printf("write back failed %x\n",res);
	}
	return 0;
}

int main(int argc, char **argv)
{
	for(int k=0;k<RUN_BENCHMARK_TIMES;k++){
		for(size=1;size<=VARY_LEN;size++){
			int res=0;
			uint64_t start,end;
			buf=(char *)malloc(LZ4_FRAME_SIZE*size*size);
			out_buf=(char *)malloc(LZ4_FRAME_SIZE*size*size);
			read_len=LZ4_FRAME_SIZE*size*size;
			signed_buf=sign_ptr(buf,SIGNATURE,TAG);
			signed_out_buf=sign_ptr(out_buf,SIGNATURE,TAG);
			signed_read_len=sign_ptr(&read_len,SIGNATURE,TAG);

			record_benchmark_time(0);

			for(int i=0;i<BENCHMARK_ITERATIONS;i++){
				res=i2c_read_bytes_async(TEST_I2C_NO,TEST_I2C_ADDR,0,LZ4_FRAME_SIZE*size*size,handler);
                // wait previous async req handled 
                while(!all_async_req_handled()){}
				native_wait(10);
			}
			
			record_benchmark_time(1);
			get_benchmark_time(0,(uint32_t)&start);
			get_benchmark_time(1,(uint32_t)&end);
			uint64_t poll_times;
			get_poll_times(&poll_times);
			printf("%dK,%llu,%llu\n",size*size,end-start,poll_times);
			free(buf);
			free(out_buf);
		}
	}
    return 0;
}

