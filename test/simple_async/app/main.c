/**
 * @brief wasm use dht native async
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

__attribute__((import_name("dht_read_async"))) int dht_read_async(uint32_t pin, uint32_t type, void (*handler)(uint32_t, uint32_t));


__attribute__((import_name("get_async_data"))) int get_async_data(uint32_t signo,
                                                                  uint32_t sub_signo,
                                                                  uint32_t dst_buf_addr,
                                                                  uint32_t dst_buf_len_addr);


void handler(uint32_t signo, uint32_t sub_signo)
{
    int16_t *buf = (int16_t *)malloc(2 * sizeof(int16_t));
    uint32_t len = 2 * sizeof(int16_t);
    int ret = get_async_data(signo, sub_signo, (uint32_t)buf, (uint32_t)&len);
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
    if (dht_read_async(22, 0, handler) == 0)
    {
        printf("async read send\n");
        do_something(INT32_MAX);
    }
}
