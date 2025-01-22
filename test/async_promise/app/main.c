/**
 * @brief wasm use dht native async promise
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

__attribute__((import_name("dht_read_async_promise"))) int dht_read_async_promise(uint32_t pin, uint32_t type);
__attribute__((import_name("promise_set_callback"))) int promise_set_callback(int promise,void (*handler)(uint32_t, uint32_t));
__attribute__((import_name("get_async_data_promise"))) int get_async_data_promise(
                                                                  int promise,
                                                                  uint32_t dst_buf_addr,
                                                                  uint32_t dst_buf_len_addr);
__attribute__((import_name("get_async_data"))) int get_async_data(uint32_t signo,
                                                                  uint32_t sub_signo,
                                                                  uint32_t dst_buf_addr,
                                                                  uint32_t dst_buf_len_addr);


void handler(uint32_t signo, uint32_t sub_signo)
{
    int16_t *buf = (int16_t *)malloc(2 * sizeof(int16_t));
    uint32_t len = 2 * sizeof(int16_t);
    // successfully trigger the callback
    int res=get_async_data(signo,sub_signo,buf,&len);
    printf("successfully trigger callback and get async data res %x\n",res);
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
    int res;
    // callback mode usage
    int promise=dht_read_async_promise(22,0);
    if(promise>=0){
        printf("get promise %x, periph_no %d, sub signo %d\n",promise, promise>>16,promise&0xFFFF);        
        do_something(1000);
        res=promise_set_callback(promise,handler);
        printf("set call back res %x\n",res);
        do_something(1000);
    }else{
        printf("get promise failed with %x\n",promise);
    }

    // directly wait get data
    int another_promise=dht_read_async_promise(22,0);
    printf("get another promise %x, periph_no %d, sub signo %d\n",another_promise, another_promise>>16,another_promise&0xFFFF);
    int16_t *buf = (int16_t *)malloc(2 * sizeof(int16_t));
    uint32_t len = 2 * sizeof(int16_t);
    res=get_async_data_promise(another_promise,buf,&len);
    printf("get async data promise res %x\n",res);
}
