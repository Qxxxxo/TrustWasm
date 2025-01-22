#include "env_monitor.h"

const float EPS = 1e-4;

const float _Q[_EKF_N*_EKF_N]={
    EPS,0,0,0,
    0,EPS,0,0,
    0,0,EPS,0,
    0,0,0,EPS,
};

const float _R[_EKF_M*_EKF_M] = {
	EPS,0,0,0,0,0,0,
	0,EPS,0,0,0,0,0,
	0,0,EPS,0,0,0,0,
	0,0,0,EPS,0,0,0,
	0,0,0,0,EPS,0,0,
	0,0,0,0,0,EPS,0,
	0,0,0,0,0,0,EPS,
};

const float _F[_EKF_N*_EKF_N] = {
    1,0,0,0,0,
    0,1,0,0,0,
	0,0,1,0,0,
	0,0,0,1,0,
	0,0,0,0,1,
};

const float _H[_EKF_M*_EKF_N] = {
    1,0,0,0, // bh1750fvi illuminance
    0,1,0,0, // bme280 pressure
	0,0,1,0, // bme280 temperature
	0,0,0,1, // bme280 humidity
	0,0,1,0, // mpu6050 temperature
	0,0,1,0, // dht11 temperature
	0,0,0,1, // dht11 humidity
};

void env_monitor_init(_ekf_t * ekf){
	const float Pdiag[_EKF_N] = {1,1,1,1};
	_ekf_initialize(&ekf,Pdiag);
}

void env_monitor_parse_mpu6050_temp(uint8_t data[14], double * temp){
    int16_t temp_tmp = (data[6]<<8)|data[7];
    *temp=(double)temp_tmp/340.0 + 36.53;
}