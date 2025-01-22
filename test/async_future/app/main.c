/**
 * @brief wasm use native dht async future
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SUCCESS 0
#define SHORT_BUFFER 0xFFFF0010
#define REQ_FAILED 0xFFFF0000

__attribute__((import_name("dht_init"))) int dht_init(uint32_t pin, uint32_t type);

__attribute__((import_name("dht_read_async_future"))) int dht_read_async_future(uint32_t pin, uint32_t type);

__attribute__((import_name("native_wait"))) int native_wait(int ms);

__attribute__((import_name("get_async_data_future"))) int get_async_data_future(uint32_t signo,
                                                                                uint32_t sub_signo,
                                                                                uint32_t dst_buf_addr,
                                                                                uint32_t dst_buf_len_addr);

__attribute__((import_name("record_benchmark_time"))) int record_benchmark_time(uint32_t idx);
__attribute__((import_name("get_benchmark_time"))) int get_benchmark_time(uint32_t idx, uint32_t out_time_addr);

int main(int argc, char **argv)
{
    uint64_t start,end,mid,get_data_mid;
    int subsigno;
    int ret;
    int16_t *buf = (int16_t *)malloc(2 * sizeof(int16_t));
    uint32_t len = 2 * sizeof(int16_t);
    // init as DHT11, pin 22
    if (dht_init(22, 0) == SUCCESS)
    {
        record_benchmark_time(0);
        subsigno = dht_read_async_future(22, 0);
        if (subsigno < 0)
        {
            // printf("async read future failed\n");
            return -1;
        }
        record_benchmark_time(2);
        // do something
        ret = get_async_data_future(22, subsigno, (uint32_t)buf, (uint32_t)&len);
        record_benchmark_time(1);
        printf("ret %d\n",ret);
        if (ret == SHORT_BUFFER)
        {
            // need long buffer
            int16_t *new_buf = (int16_t *)realloc(buf, len);
            ret = get_async_data_future(22, subsigno, (uint32_t)new_buf, (uint32_t)&len);
            if (ret > 0)
            {
                // printf("success: temperature %d.%d°C, humdity %d.%d%%\n",
                //        new_buf[0] / 10, new_buf[0] % 10, new_buf[1] / 10, new_buf[1] % 10);
            }
        }
        else if(ret == REQ_FAILED){
            // printf("dht read req return failed");
        }
        else if (ret > 0)
        {
            // printf("success: temperature %d.%d°C, humdity %d.%d%%\n",
            //        buf[0] / 10, buf[0] % 10, buf[1] / 10, buf[1] % 10);
        }
        else
        {
            // printf("get async data future failed\n");
        }
    }
    get_benchmark_time(0,(uint32_t)&start);
    get_benchmark_time(1,(uint32_t)&end);
    get_benchmark_time(2,(uint32_t)&mid);
    get_benchmark_time(3,(uint32_t)&get_data_mid);
    printf("start %llu, end %llu, interval %llu\n",start,end,end-start);
    printf("start-mid interval %llu\n",mid-start);
    printf("mid-end interval %llu\n",end-mid);
    printf("get_data_mid-end interval %llu\n",end-get_data_mid);
    return 0;
}
