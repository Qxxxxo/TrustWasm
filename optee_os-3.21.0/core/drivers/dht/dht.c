/**
 * @brief driver for dht
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <periph/gpio.h>
#include <drivers/dht/dht.h>
#include <kernel/delay.h>
#include <kernel/tee_time.h>
#include <trace.h>
#include <string.h>

// /* Every pulse send by the DHT longer than 40µs is interpreted as 1 */
#define READ_THRESHOLD (40U)

/* If an expected pulse is not detected within 85µs, something is wrong */
#define SPIN_TIMEOUT_US (100U)

/* The start signal by pulling data low for at least 18 ms for DHT11, at
 * most 20 ms (AM2301 / DHT22 / DHT21). Then release the bus and the
 * sensor should respond by pulling data low for 80 µs, then release for
 * 80µs before start sending data. */
#define START_LOW_TIME (19U * 1000LU)
// #define START_THRESHOLD (75U)

// /* DHTs have to wait for power 1 or 2 seconds depending on the model */
#define POWER_WAIT_TIMEOUT (2U * 1000000LU)

enum
{
    BYTEPOS_HUMIDITY_HIGH = 0,
    BYTEPOS_HUMIDITY_LOW = 1,
    BYTEPOS_TEMPERATURE_HIGH = 2,
    BYTEPOS_TEMPERATURE_LOW = 3,
    BYTEPOS_CHECKSUM = 4,
};

struct dht_data
{
    gpio_t pin;
    gpio_mode_t in_mode;
    uint8_t data[5];
    int8_t bit_pos;
    uint8_t bit;
};


static inline uint64_t teetime_micro(TEE_Time *t)
{
    return t->seconds * 1000000 + t->micros;
}


// static int wait_for_level(gpio_t  pin, gpio_level_t expected, uint64_t timeout)
// {
//     TEE_Time start,end;
//     tee_time_get_sys_time(&start);
//     tee_time_get_sys_time(&end);
//     while (gpio_get(pin) != (int)expected)
//     {
//         if (teetime_micro(&end) - teetime_micro(&start) >= timeout)
//         {
//             return -1;
//         }
//     }
//     return 0;
// }

static TEE_Result _send_start_signal(dht_t *dev)
{
    // uint64_t start;
    // TEE_Time time;
    // TEE_Result time_res;
    // _______/-----
    // ↑>=18ms↑ sync
    // gpio_init(dev->params.pin, GPIO_OUT);
    // gpio_set(dev->params.pin, GPIO_LOW_LEVEL);
    // udelay(START_LOW_TIME);
    // // sync on device
    // gpio_set(dev->params.pin, GPIO_HIGH_LEVEL);
    // gpio_init(dev->params.pin, dev->params.in_mode);

    // check device response, 80us low then 80us high
    // -----------\_______/-------\____
    // high <=80us↑       ↑ >=80us↑
    //       low resp    high resp        
    // time_res = tee_time_get_sys_time(&time);
    // if (__builtin_expect(time_res != TEE_SUCCESS,0))
    //     return time_res;
    // start = cal_us(&time);
    // _wait_for_level(dev->params.pin, GPIO_LOW_LEVEL, start);
    // time_res = tee_time_get_sys_time(&time);
    // if (__builtin_expect(time_res != TEE_SUCCESS,0))
    //     return time_res;
    // if (cal_us(&time) - start > START_THRESHOLD)
    // {
    //     DMSG("[dht] error: response low pulse > START_THRESHOLD");
    //     return TEE_ERROR_ITEM_NOT_FOUND;
    // }
    // DMSG("[dht] low pulse (%lu us)",cal_us(&time) - start);
    // _wait_for_level(dev->params.pin, GPIO_HIGH_LEVEL, start);
    // time_res = tee_time_get_sys_time(&time);
    // if (__builtin_expect(time_res != TEE_SUCCESS,0))
    //     return time_res;
    // start = cal_us(&time);
    // _wait_for_level(dev->params.pin, GPIO_LOW_LEVEL, start);
    // time_res = tee_time_get_sys_time(&time);
    // if (__builtin_expect(time_res != TEE_SUCCESS,0))
    //     return time_res;
    
    // if (cal_us(&time) - start < START_THRESHOLD)
    // {
    //     DMSG("[dht] error: response high pulse ( %lu us) < START_THRESHOLD",cal_us(&time) - start);
    //     return TEE_ERROR_ITEM_NOT_FOUND;
    // }
    // DMSG("[dht] high pulse (%lu us)",cal_us(&time) - start);
    // return TEE_SUCCESS;

    uint16_t time_out=SPIN_TIMEOUT_US;
    gpio_init(dev->params.pin, GPIO_OUT);
    gpio_set(dev->params.pin, GPIO_LOW_LEVEL);
    udelay(START_LOW_TIME);
    // sync on device
    gpio_set(dev->params.pin, GPIO_HIGH_LEVEL);
    udelay(60);
    gpio_init(dev->params.pin, dev->params.in_mode);
    if(gpio_get(dev->params.pin)==GPIO_LOW_LEVEL){
        while(!gpio_get(dev->params.pin)&&--time_out){} // low, wait high
        if(time_out<=0) return TEE_ERROR_ITEM_NOT_FOUND;
        udelay(80);
        // time_out=SPIN_TIMEOUT;
        // while(gpio_get(dev->params.pin)&&--time_out){} // high, wait low
        // if(time_out<=0) return TEE_ERROR_ITEM_NOT_FOUND;
        return TEE_SUCCESS;
    }else{
        return TEE_ERROR_ITEM_NOT_FOUND;
    }
}

static void  _bit_parse(struct dht_data * arg){
    int8_t pos = arg->bit_pos++;
    if(arg->bit){
        arg->data[pos/8] |= (0x80U>>(pos%8));
    }
}

static TEE_Result _busy_wait_read(struct dht_data * arg){
    // TEE_Time time;
    // TEE_Result res;
    // uint64_t start;
    // res=tee_time_get_sys_time(&time);
    // if(__builtin_expect(res != TEE_SUCCESS,0)) return res;
    // start = cal_us(&time); 
    // // read 40 bits
    // while(arg->bit_pos != 40){
    //     _wait_for_level(arg->pin,GPIO_HIGH_LEVEL,start);

    //     res=tee_time_get_sys_time(&time);
    //     if(__builtin_expect(res != TEE_SUCCESS,0)) return res;
    //     start = cal_us(&time);

    //     _wait_for_level(arg->pin,GPIO_LOW_LEVEL,start);

    //     res=tee_time_get_sys_time(&time);
    //     if(__builtin_expect(res != TEE_SUCCESS,0)) return res;
    //     arg->bit=(cal_us(&time)-start>READ_THRESHOLD) ? 1:0;
    //     _bit_parse(arg);
    // }
    uint16_t time_out=SPIN_TIMEOUT_US;
    while(arg->bit_pos != 40){
        while(gpio_get(arg->pin)&&--time_out){} // high, wait low
        if(time_out<=0) return TEE_ERROR_TIMEOUT;
        time_out=SPIN_TIMEOUT_US;
        while(!gpio_get(arg->pin)&&--time_out){} // low, wait high
        if(time_out<=0) return TEE_ERROR_TIMEOUT;

        udelay(READ_THRESHOLD);
        arg->bit=(gpio_get(arg->pin)==GPIO_HIGH_LEVEL) ? 1:0;
        _bit_parse(arg);
    }
    return TEE_SUCCESS;
}

static TEE_Result _validate_checksum(uint8_t * data){
    uint8_t sum=0;
    for (uint_fast8_t i=0;i<4;i++){
        sum+=data[i];
    }
    if(sum != data[BYTEPOS_CHECKSUM]){
        return TEE_ERROR_BAD_FORMAT;
    }
    return TEE_SUCCESS;
}

static TEE_Result _parse_raw_values(dht_t * dev, uint8_t * data){
    bool is_negative;
    switch (dev->params.type){
        case DHT11:
        case DHT11_2022:
            // DMSG("[dht] parse raw values with DHT11 data format");
            dev->last_val.humidity = data[BYTEPOS_HUMIDITY_HIGH]*10
                                    +data[BYTEPOS_HUMIDITY_LOW];
            /* MSB for integral temperature byte gives sign, remaining is
            * abs() of value (beware: this is not two's complement!) */
            is_negative = data[BYTEPOS_TEMPERATURE_LOW] & 0x80;
            data[BYTEPOS_TEMPERATURE_LOW] &= ~0x80;
            /* 2022-12 aosong.com data sheet uses interprets low bits as
            * 0.01°C per LSB */
            if (dev->params.type == DHT11_2022) {
                data[BYTEPOS_TEMPERATURE_LOW] /= 10;
            }
            if (data[BYTEPOS_TEMPERATURE_LOW] >= 10) {
                return TEE_ERROR_BAD_PARAMETERS;
            }
            dev->last_val.temperature = data[BYTEPOS_TEMPERATURE_HIGH] * 10
                                    + data[BYTEPOS_TEMPERATURE_LOW];
            break;
        /* AM2301 == DHT21 == DHT22 (same value in enum),
            * so all are handled here */
        case DHT22:
            // DMSG("[dht] parse raw values with DHT22 data format");
            dev->last_val.humidity = (int16_t)(
                                        (data[BYTEPOS_HUMIDITY_HIGH] << 8)
                                        | data[BYTEPOS_HUMIDITY_LOW]);
            is_negative = data[BYTEPOS_TEMPERATURE_HIGH] & 0x80;
            data[BYTEPOS_TEMPERATURE_HIGH] &= ~0x80;
            dev->last_val.temperature = (int16_t)(
                                        (data[BYTEPOS_TEMPERATURE_HIGH] << 8)
                                        | data[BYTEPOS_TEMPERATURE_LOW]);
            break;
        default:
            return TEE_ERROR_NOT_SUPPORTED;
    }
    if(is_negative){
        dev->last_val.temperature = -dev->last_val.temperature;
    }
    return TEE_SUCCESS;
}

TEE_Result dht_init(dht_t * dev, const dht_params_t * params){
    int16_t timeout;
    // DMSG("[dht] dht_init");
    if(dev==NULL||params==NULL) return TEE_ERROR_BAD_PARAMETERS;
    if(params->type!=DHT11&&params->type!=DHT11_2022&&params->type!=DHT22) return TEE_ERROR_BAD_PARAMETERS;
    memset(dev,0,sizeof(dht_t));
    dev->params=*params;

    /* The 2-second delay mentioned in the datasheet is only required
     * after a power cycle. */
    timeout = POWER_WAIT_TIMEOUT / 1000UL;
    gpio_init(dev->params.pin,GPIO_IN);
    while((gpio_get(dev->params.pin)<=0)&&timeout--){
        udelay(1000UL);
    }
    if(timeout<=0){
        DMSG("[dht] error: Invalid cross-device link");
        return TEE_ERROR_BAD_STATE;
    }
    //else{
        //  DMSG("[dht] power-up duration: %d ms",(int16_t)(POWER_WAIT_TIMEOUT/1000UL-timeout));
    // }
    /* The previous test does not ensure the sensor presence in case an
     * external pull-up resistor is used. */
    while(_send_start_signal(dev)!=TEE_SUCCESS
        &&(timeout-=START_LOW_TIME/1000UL)>0){}
    if(timeout<=0){
        DMSG("[dht] error: No such device");
        return TEE_ERROR_ITEM_NOT_FOUND;
    }
    // else{
    //     DMSG("[dht] presence check duration %d ms",
    //         (int16_t)(POWER_WAIT_TIMEOUT/1000UL-timeout));
    // }

    // DMSG("[dht] success");
    return TEE_SUCCESS;
}

TEE_Result dht_read(dht_t *dev, int16_t * temp, int16_t * hum){
    TEE_Result ret;
    if(dev==NULL) return TEE_ERROR_BAD_PARAMETERS;
    struct dht_data data={
        .pin=dev->params.pin,
        .in_mode=dev->params.in_mode,
        .bit_pos = 0,
    };
    // DMSG("[dht] in_mode %d",data.in_mode);

    if(_send_start_signal(dev) != TEE_SUCCESS){
        DMSG("[dht] error: No response from device");
        return TEE_ERROR_ITEM_NOT_FOUND;
    }

    // read the data
    if(_busy_wait_read(&data)!=TEE_SUCCESS){
        DMSG("[dht] error: wait read data failed");
        return TEE_ERROR_ITEM_NOT_FOUND;
    }

    if(_validate_checksum(data.data)!=TEE_SUCCESS){
        DMSG("[dht] error: checksum doesn't match");
        DMSG("[dht] RAW data: 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x",
              (unsigned)data.data[0], (unsigned)data.data[1],
              (unsigned)data.data[2], (unsigned)data.data[3],
              (unsigned)data.data[4]);
        return TEE_ERROR_BAD_FORMAT;
    }

    ret=_parse_raw_values(dev,data.data);
    if(ret!=TEE_SUCCESS){
        if(ret==TEE_ERROR_NOT_SUPPORTED){
            DMSG("[dht] error: data format not supported");
        }else if(ret==TEE_ERROR_BAD_PARAMETERS){
            DMSG("[dht] error: invalid temperature low byte");
        }
        return ret;
    }
    

    if(hum!=NULL){
        *hum=dev->last_val.humidity;
    }
    if(temp!=NULL){
        *temp=dev->last_val.temperature;
    }
    return TEE_SUCCESS;
}