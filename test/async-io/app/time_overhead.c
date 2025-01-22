/**
 * @brief wasm use dht native
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SUCCESS 0
#define SHORT_BUFFER 0xFFFF0010

__attribute__((import_name("record_benchmark_time"))) int record_benchmark_time(uint32_t idx);
__attribute__((import_name("get_benchmark_time"))) int get_benchmark_time(uint32_t idx,uint32_t out_time_addr);
__attribute__((import_name("test_tee_get_sys_time_overhead"))) int test_tee_get_sys_time_overhead();

int main(int argc, char **argv)
{
    uint64_t start_time,end_time;
    record_benchmark_time(0);
    printf("this is a line to test printf overhead %d %d %d\n",42,42,42);
    record_benchmark_time(1);
    get_benchmark_time(0,(uint32_t)&start_time);
    get_benchmark_time(1,(uint32_t)&end_time);
    printf("start time: %llu, end time: %llu, interval: %llu\n",start_time,end_time,end_time-start_time);
    test_tee_get_sys_time_overhead();
    return 0;
}
