/**
 * @brief wasm use dht native
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SUCCESS 0
#define SHORT_BUFFER 0xFFFF0010

__attribute__((import_name("dht_init"))) int dht_init(uint32_t pin, uint32_t type);

__attribute__((import_name("dht_read"))) int
dht_read(uint32_t pin, uint32_t type, uint32_t dst_temp_addr, uint32_t dst_hum_addr);

__attribute__((import_name("record_benchmark_time"))) int record_benchmark_time(uint32_t idx);
__attribute__((import_name("get_benchmark_time"))) int get_benchmark_time(uint32_t idx,uint32_t out_time_addr);


int main(int argc, char **argv)
{
    int16_t temp = 0;
    int16_t hum = 0;
    uint64_t start_time,end_time;
    // init as DHT11, pin 22
    if (dht_init(22, 0) == SUCCESS)
    {   
        record_benchmark_time(0);
        if (dht_read(22, 0, (uint32_t)&temp, (uint32_t)&hum) == SUCCESS)
        {
            record_benchmark_time(1);
            printf("success: temperature %d.%d°C, humdity %d.%d%%\n", temp/10,temp%10, hum/10,hum%10);
        }else{
            record_benchmark_time(1);
            printf("failed\n");
        }
    }
    get_benchmark_time(0,(uint32_t)&start_time);
    get_benchmark_time(1,(uint32_t)&end_time);
    printf("start time: %llu, end time: %llu, interval: %llu\n",start_time,end_time,end_time-start_time);
    return 0;
}
