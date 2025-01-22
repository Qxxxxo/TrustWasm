/**
 * @brief wasm env monitor
 * EKF implementation and sensor fusion refer to
 * https://github.com/simondlevy/TinyEKF
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include "native_funcs.h"
#include "safe_bme280.h"

// Size of state space
#define EKF_N 4
// illuminance, pressure, temperature, humidity

// Size of observation (measurement) space
#define EKF_M 7
// bh1750fvi illuminance, bme280 pressure, bme280 temperature, bme280 humidity, MPU6050 temperature, dht11 temperature, dht11 humidity


#include <tinyekf.h>

#define DHT_PERIPH_NO	22
#define BH1750FVI_PERIPH_NO 29
#define BME280_PERIPH_NO 29
#define MPU6050_PERIPH_NO 29

#define DHT_GPIO_PIN	22
#define BH1750FVI_I2C_NO 1
#define BME280_I2C_NO 1
#define MPU6050_I2C_NO 1

#define BH1750FVI_ADDR 	0x23
#define BME280_ADDR		0x77
#define MPU6050_ADDR	0x68

#define SEUCRE_OBJECT_ID "env_monitor"
#define SECURE_OBJECT_ID_LEN 11
#define MPU6050_DATA_LEN	14
#define BME280_DATA_LEN		8
#define SAVE_STR_LEN	256

#define BENCHMARK_ITERATIONS 50
#define RUN_BENCHMARK_TIMES	1

int wait_ms = 1;

static const float EPS = 1e-4;

static const float Q[EKF_N*EKF_N] = {
	EPS,0,0,0,
	0,EPS,0,0,
	0,0,EPS,0,
	0,0,0,EPS,
};

static const float R[EKF_M*EKF_M] = {
	EPS,0,0,0,0,0,0,
	0,EPS,0,0,0,0,0,
	0,0,EPS,0,0,0,0,
	0,0,0,EPS,0,0,0,
	0,0,0,0,EPS,0,0,
	0,0,0,0,0,EPS,0,
	0,0,0,0,0,0,EPS,
};

static const float F[EKF_N*EKF_N] = {
    1,0,0,0,0,
    0,1,0,0,0,
	0,0,1,0,0,
	0,0,0,1,0,
	0,0,0,0,1,
};

static const float H[EKF_M*EKF_N] = {
    1,0,0,0, // bh1750fvi illuminance
    0,1,0,0, // bme280 pressure
	0,0,1,0, // bme280 temperature
	0,0,0,1, // bme280 humidity
	0,0,1,0, // mpu6050 temperature
	0,0,1,0, // dht11 temperature
	0,0,0,1, // dht11 humidity
};

uint16_t lux;
int16_t dht11_raw_data[2];
float dht11_temp;
float dht11_hum;
float bme280_hum;
float bme280_pressure;
float bme280_temp;
float mpu_temp;
uint8_t mpu_data[MPU6050_DATA_LEN]={0};
uint8_t bme280_data[BME280_DATA_LEN]={0};
uint32_t lux_len=sizeof(uint16_t);
uint32_t bme_len=BME280_DATA_LEN;
uint32_t mpu_len=MPU6050_DATA_LEN;
uint32_t dht_len=2*sizeof(int16_t);
bme280_calibration_data_t bme280_calib_data;
int32_t t_fine;
uint32_t signed_lux;
uint32_t signed_mpu_data;
uint32_t signed_bme280_data;
uint32_t signed_dht_data;
uint32_t signed_lux_len;
uint32_t signed_mpu_len;
uint32_t signed_bme_len;
uint32_t signed_dht_len;
uint32_t signed_save_str;
char save_str[SAVE_STR_LEN]={0};
// observation vector
float z[EKF_M]={0};
// process model is f(x)=x
float fx[EKF_N]={0};
// measurement function
float hx[EKF_M]={0};
ekf_t ekf = {0};

void env_monitor_init(ekf_t * mekf){
	const float Pdiag[EKF_N] = {1,1,1,1};
	ekf_initialize(&mekf,Pdiag);
}


void parse_mpu6050_temp(uint8_t data[MPU6050_DATA_LEN], double * temp){
    int16_t temp_tmp = (data[6]<<8)|data[7];
    *temp=(double)temp_tmp/340.0 + 36.53;
}

int mpu6050_handler(uint32_t signo, uint32_t sub_signo){
    mpu_len=MPU6050_DATA_LEN;
    signed_mpu_data=sign_ptr(mpu_data, SIGNATURE, TAG);
    signed_mpu_len=sign_ptr(&mpu_len, SIGNATURE, TAG);
    int res=get_async_data(signo,sub_signo,signed_mpu_data,signed_mpu_len);
    if(res<0){
        printf("get async data mpu6050 failed with %x\n",res);
    }else{
        parse_mpu6050_temp(mpu_data,&mpu_temp);
    }
    return 0;
}

int bme280_handler(uint32_t signo, uint32_t sub_signo){
    bme_len=BME280_DATA_LEN;
    signed_bme_len=sign_ptr(&bme_len, SIGNATURE, TAG);
    signed_bme280_data=sign_ptr(bme280_data, SIGNATURE, TAG); // need resign
    int res=get_async_data(signo,sub_signo,signed_bme280_data,signed_bme_len);
    if(res<0){
        printf("bme280 failed with %x\n",res);
        // printf("bme280 need len %d\n",bme_len);
    }else{
        bme280_temp=bme280_convert_temperature(bme280_data,&bme280_calib_data,&t_fine);
        bme280_pressure=bme280_convert_pressure(bme280_data,&bme280_calib_data,t_fine);
        bme280_hum=bme280_convert_humidity(bme280_data,&bme280_calib_data,t_fine);
    }
    return 0;
}

int dht_handler(uint32_t signo, uint32_t sub_signo){
    dht_len=2*sizeof(int16_t);
    signed_dht_len=sign_ptr(&dht_len,SIGNATURE,TAG);
    signed_dht_data=sign_ptr(dht11_raw_data, SIGNATURE, TAG);
    int res=get_async_data(signo,sub_signo,signed_dht_data,signed_dht_len);
    if(res<0){
        dht11_temp=24.3;
        dht11_hum=30.0;
    }else{
        dht11_temp=(float)dht11_raw_data[0]/10+ (float)(dht11_raw_data[0]%10)/10.0;
        dht11_hum=(float)dht11_raw_data[1]/10+ (float)(dht11_raw_data[1]%10)/10.0;
    }
    return 0;
}

int bh1750fvi_handler(uint32_t signo, uint32_t sub_signo){
    lux_len=sizeof(uint16_t);
    signed_lux_len=sign_ptr(&lux_len,SIGNATURE,TAG);
    signed_lux=sign_ptr(&lux,SIGNATURE,TAG);
    int res=get_async_data(signo,sub_signo,signed_lux,signed_lux_len);
    if(res<0){
        printf("get async data bh1750fvi failed with %x\n",res);
        // printf("need lux len %d, signo %d, sub signo %d\n",lux_len, signo, sub_signo);
    }
    // fusion data
    z[0]=(float)lux;
    z[1]=bme280_pressure;
    z[2]=bme280_temp;
    z[3]=bme280_hum;
    z[4]=mpu_temp;
    z[5]=dht11_temp;
    z[6]=dht11_hum;
    
    // process model
    fx[0]=ekf.x[0];
    fx[1]=ekf.x[1];
    fx[2]=ekf.x[2];
    fx[3]=ekf.x[3];

    ekf_predict(&ekf,fx,F,Q);
    
    hx[0]=ekf.x[0];
    hx[1]=ekf.x[1];
    hx[2]=ekf.x[2];
    hx[3]=ekf.x[3];
    hx[4]=ekf.x[2];
    hx[5]=ekf.x[2];
    hx[6]=ekf.x[3];

    ekf_update(&ekf, z, hx, H, R);

    int len=sprintf(save_str,"illumilance: %.2f lux, pressure: %.2f pa, temperature: %.2f °C, humidity %.2f\n",
            ekf.x[0],ekf.x[1],ekf.x[2],ekf.x[3]);
    
    if(len>0){
        char obj_id[SECURE_OBJECT_ID_LEN]=SEUCRE_OBJECT_ID;
        uint32_t signed_id=sign_ptr(obj_id,SIGNATURE,TAG);
        signed_save_str=sign_ptr(save_str, SIGNATURE, TAG); // need resign
        res=write_secure_object(signed_id,SECURE_OBJECT_ID_LEN,signed_save_str,len);
        if(res!=0){
            printf("write secure object failed with %x\n",res);
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
	for(int k=0;k<RUN_BENCHMARK_TIMES;k++){
		uint64_t start,end;
		int res;
        signed_lux=sign_ptr(&lux, SIGNATURE, TAG);
        signed_mpu_data=sign_ptr(mpu_data, SIGNATURE, TAG);
        signed_bme280_data=sign_ptr(bme280_data, SIGNATURE, TAG);
        signed_dht_data=sign_ptr(dht11_raw_data, SIGNATURE, TAG);
		signed_lux_len=sign_ptr(&lux_len, SIGNATURE, TAG);
		signed_mpu_len=sign_ptr(&mpu_len, SIGNATURE, TAG);
		signed_bme_len=sign_ptr(&bme_len, SIGNATURE, TAG);
		signed_dht_len=sign_ptr(&dht_len, SIGNATURE, TAG);
        signed_save_str=sign_ptr(save_str, SIGNATURE, TAG);
		
		env_monitor_init(&ekf);
		res=mpu6050_init(MPU6050_I2C_NO,MPU6050_ADDR);
		if(res!=0){
			printf("mpu6050 init failed with %x\n",res);
		}
		res=bh1750fvi_init(BH1750FVI_I2C_NO,BH1750FVI_ADDR);
		if(res!=0){
			printf("bh1750fvi init failed with %x\n",res);
		}
		res=bme280_init(BME280_I2C_NO,BME280_ADDR);
		if(res!=0){
			printf("bme280 init failed with %x\n",res);
		}
		res=dht_init(DHT_GPIO_PIN,0);
		if(res==0){
			// get bme280 calibration data
			bme280_get_calibration(BME280_I2C_NO,BME280_ADDR,&bme280_calib_data);

			record_benchmark_time(0);
			
			for(int i=0;i<BENCHMARK_ITERATIONS;i++){
				// io helper can be busy in this benchmark, so keep send async until success
				// res=mpu6050_sample_async(MPU6050_I2C_NO,MPU6050_ADDR,mpu6050_handler);
                // if(res!=0){
                //     printf("mpu6050_sample_async failed with %x\n",res);
                // }
                // res=bme280_sample_async(BME280_I2C_NO, BME280_ADDR, bme280_handler);
                // if(res!=0){
                //     printf("bme280_sample_async failed with %x\n",res);
                // }
                // res=dht_read_async(DHT_GPIO_PIN, 0,dht_handler);
                // if(res!=0){
                //     printf("dht_read_async failed with %x\n",res);
                // }
                // res=bh1750fvi_sample_async(BH1750FVI_I2C_NO,BH1750FVI_ADDR,bh1750fvi_handler);
                // if(res!=0){
                //     printf("bh1750fvi_sample_async failed with %x\n",res);
                // }
                while(mpu6050_sample_async(MPU6050_I2C_NO,MPU6050_ADDR,mpu6050_handler)!=0){}
				while(bme280_sample_async(BME280_I2C_NO, BME280_ADDR, bme280_handler)!=0){} 
				while(dht_read_async(DHT_GPIO_PIN, 0,dht_handler)!=0){}
                while(bh1750fvi_sample_async(BH1750FVI_I2C_NO,BH1750FVI_ADDR,bh1750fvi_handler)!=0){}
				native_wait(wait_ms);
			}
            // spin until all async request handled
            while(!all_async_req_handled()){
            }
			record_benchmark_time(1);
			get_benchmark_time(0,(uint32_t)&start);
			get_benchmark_time(1,(uint32_t)&end);
			printf("%llu\n",end-start);
		}else{
			printf("init failed with %x\n",res);
		}
	}
    return 0;
}

