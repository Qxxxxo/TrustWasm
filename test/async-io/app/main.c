/**
 * @brief wasm use dht native async
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SUCCESS 0
#define SHORT_BUFFER 0xFFFF0010

__attribute__((import_name("dht_init"))) int dht_init(uint32_t pin, uint32_t type);

__attribute__((import_name("dht_read_async"))) int dht_read_async(uint32_t pin, uint32_t type, void (*handler)(uint32_t, uint32_t));

__attribute__((import_name("native_wait"))) int native_wait(int ms);

__attribute__((import_name("get_async_data"))) int get_async_data(uint32_t signo,
                                                                  uint32_t sub_signo,
                                                                  uint32_t dst_buf_addr,
                                                                  uint32_t dst_buf_len_addr);
__attribute__((import_name("record_benchmark_time"))) int record_benchmark_time(uint32_t idx);
__attribute__((import_name("get_benchmark_time"))) int get_benchmark_time(uint32_t idx,uint32_t out_time_addr);

void handler(uint32_t signo, uint32_t sub_signo)
{
    int16_t *buf = (int16_t *)malloc(2 * sizeof(int16_t));
    uint32_t len = 2 * sizeof(int16_t);
    record_benchmark_time(2);
    int ret = get_async_data(signo, sub_signo, (uint32_t)buf, (uint32_t)&len);
    record_benchmark_time(1);
    // if (ret == SHORT_BUFFER)
    // {
    //     // need buf for for suggest len
    //     int16_t *new_buf = (int16_t *)realloc(buf, len);
    //     // try again
    //     ret = get_async_data(signo, sub_signo, (uint32_t)new_buf, (uint32_t)&len);
    //     if (ret > 0)
    //     {
    //         printf("signo %d, sub_signo %d success: temperature %d.%d°C, humdity %d.%d%%\n",
    //                signo,sub_signo,new_buf[0] / 10, new_buf[0] % 10, new_buf[1] / 10, new_buf[1] % 10);
    //     }
    // }
    // else if (ret > 0)
    // {
    //     printf("signo %d, sub_signo %d success: temperature %d.%d°C, humdity %d.%d%%\n",
    //            signo,sub_signo,buf[0] / 10, buf[0] % 10, buf[1] / 10, buf[1] % 10);
    // }else{
    //     printf("signo %d, sub_signo %d failed\n",signo,sub_signo);
    // }
}

int64_t do_something(int n)
{
    int64_t a=0;
    for (int i = 0; i < n;)
    {
        a+=1;
        i++;
    }
    return a;
}

int main(int argc, char **argv)
{
    uint64_t start_time,end_time,mid;
    // init as DHT11, pin 22
    if (dht_init(22, 0) == SUCCESS)
    {   
        record_benchmark_time(0);
        if (dht_read_async(22, 0, handler) == SUCCESS)
        {
            printf("async read send\n");
            do_something(INT32_MAX);
        }
    }
    get_benchmark_time(0,(uint32_t)&start_time);
    get_benchmark_time(1,(uint32_t)&end_time);
    get_benchmark_time(2,(uint32_t)&mid);
    printf("start time: %llu, end time: %llu, interval: %llu\n",start_time,end_time,end_time-start_time);
    printf("mid-end interval: %llu\n",end_time-mid);
    return 0;
}
