/**
 * @brief wasm use dht native
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

__attribute__((import_name("dht_init"))) int
dht_init(uint32_t pin, uint32_t type);

__attribute__((import_name("dht_read"))) int
dht_read(uint32_t pin, uint32_t type, uint32_t dst_temp_addr, uint32_t dst_hum_addr);

__attribute__((import_name("gpio_init"))) int
gpio_init(int pin, int func_code);

__attribute__((import_name("gpio_set"))) int
gpio_set(int pin, int val);

__attribute__((import_name("gpio_get"))) int
gpio_get(int pin);

__attribute__((import_name("native_wait"))) int
native_wait(int ms);

int main(int argc, char **argv)
{
    int16_t temp = 0;
    int16_t hum = 0;

    // init as DHT11
    if (dht_init(22, 0) == 0)
    {
        for (int i = 0; i < 10; i++)
        {
            // read from DHT
            if(dht_read(22, 0, (uint32_t)&temp, (uint32_t)&hum)==0){
                printf("temperature %d.%d°C, humdity %d.%d%%\n", temp/10,temp%10, hum/10,hum%10);
            }
        }
    }
    return 0;
}

