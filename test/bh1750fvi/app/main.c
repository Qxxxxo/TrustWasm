/**
 * @brief wasm use native bh1750fvi 
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

__attribute__((import_name("bh1750fvi_init"))) int
bh1750fvi_init(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("bh1750fvi_sample"))) int
bh1750fvi_sample(uint32_t i2c_no, uint32_t addr, uint32_t dst_lux_addr);

int main(int argc, char **argv)
{
    uint16_t lux = 0;
    // init
    if (bh1750fvi_init(1, 0x23) == 0)
    {
        // while(1){
            // read from bh1750fvi
            if(bh1750fvi_sample(1, 0x23, (uint32_t)&lux)==0){
                printf("Illuminance %d lx", lux);
            }
        // }
    }
    return 0;
}

