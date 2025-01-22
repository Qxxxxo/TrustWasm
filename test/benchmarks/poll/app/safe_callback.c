/**
 * @brief wasm 
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>
#include "native_funcs.h"


#define TEST_I2C_NO 1
#define TEST_I2C_ADDR 0x55
#define TEST_SIZE 128

#define MAX_ITERATION 5
#define BENCHMARK_ITERATIONS 25
#define RUN_BENCHMARK_TIMES	50
#define N_LOOP	1
#define N_SIZE	1


uint64_t loop_and_if(uint64_t n){
	volatile uint64_t total=0;
	volatile uint64_t a=0;
	while(total<=n){
		total+=1;
		if(total>n/2){
			a+=2;
		}else{
			a+=1;
		}
	}
	return total+a;
}

int size;

int handler(uint32_t signo, uint32_t sub_signo)
{
	int len=TEST_SIZE*size;
	uint8_t * buf=(uint8_t*)malloc(len);
	uint32_t signed_len=sign_ptr(&len, SIGNATURE, TAG); 
	uint32_t signed_buf=sign_ptr(buf, SIGNATURE, TAG);
    int res=get_async_data(signo,sub_signo,signed_buf,signed_len);
    if(res<0){
		printf("get async data failed with %x\n",res);
	}else{
		// for test. record poll times
		record_poll_times(res);
	}
	record_benchmark_time(4);
	free(buf);
	return 0;
}

int main(int argc, char **argv)
{
	for(int k=0;k<RUN_BENCHMARK_TIMES;k++){
		for(int iteration=1;iteration<=MAX_ITERATION;iteration++){
			for(size=1;size<=N_SIZE;size++){
				for(int n=1;n<=N_LOOP;n++){
					int res=0;
					uint64_t start,end;
					uint64_t other_code_start,other_code_end;
					uint64_t handler_over;
					uint64_t origin_loop_time;

					// printf("ta get sys time overhead %d\n",test_ta_get_sys_time_overhead());

					record_benchmark_time(0);
					loop_and_if(n*1000);
					record_benchmark_time(1);
					get_benchmark_time(0,(uint32_t)&start);
					get_benchmark_time(1,(uint32_t)&end);
					origin_loop_time=end-start;

					record_benchmark_time(0);
					// async 
					for(int i=0;i<iteration*BENCHMARK_ITERATIONS;i++){
						res=i2c_read_bytes_async(TEST_I2C_NO,TEST_I2C_ADDR,0,TEST_SIZE*size,handler);
						record_benchmark_time(2);
						loop_and_if(n*1000);
						record_benchmark_time(3);
						// wait finished
						while(!all_async_req_handled()){
						}
					}
					record_benchmark_time(1);
					get_benchmark_time(0,(uint32_t)&start);
					get_benchmark_time(1,(uint32_t)&end);
					get_benchmark_time(2,(uint32_t)&other_code_start);
					get_benchmark_time(3,(uint32_t)&other_code_end);
					get_benchmark_time(4,(uint32_t)&handler_over);
					uint64_t poll_times,profile_times,runtime_poll_times;
					get_poll_times(&poll_times);
					get_runtime_profile_times(&profile_times);
					get_runtime_poll_times(&runtime_poll_times);
					// printf("%d,%d,%llu,%llu,%llu,%llu,%lld\n",
					// 	TEST_SIZE*size,n*1000,end-start,poll_times,origin_loop_time,other_code_end-other_code_start,other_code_end-handler_over);
					// printf("%llu,%llu,%llu,%llu,%lld\n",
					// 	end-start,poll_times,profile_times,runtime_poll_times,other_code_end-handler_over);
					printf("%d,%llu,%llu,%llu\n",
						iteration*BENCHMARK_ITERATIONS,end-start,poll_times,profile_times);
				}
			}
		}
	}
    return 0;
}

