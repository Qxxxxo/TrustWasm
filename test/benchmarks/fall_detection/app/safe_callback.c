/**
 * @brief wasm fall detection app
 * reference to 
 * https://github.com/kdienes/madgwick-ahrs/blob/master/MadgwickAHRS/MadgwickAHRS.c
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "mmath.h"

#define AT24CXX_I2C_NO 0
#define MPU6050_I2C_NO 1
#define MPU6050_PERIPH_NO 29
#define AT24CXX_PERIPH_NO 28
#define MPU6050_ADDR 0x68  
#define AT24CXX_ADDR 0x50  
#define MPU6050_DATA_LEN 14    
#define SAMPLE_FREQ	200.0f		// sample frequency in Hz
#define BETA_DEF 0.1f		// 2 * proportional gain
#define BENCHMARK_ITERATIONS 50
#define RUN_BENCHMARK_TIMES 1

// for test only
#define SIGNATURE 0xE5
#define TAG 0x12

__attribute__((import_name("mpu6050_init"))) int
mpu6050_init(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("mpu6050_deinit"))) int
mpu6050_deinit(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("mpu6050_sample"))) int
mpu6050_sample(uint32_t i2c_no, uint32_t addr, uint32_t dst_addr);

__attribute__((import_name("all_async_req_handled"))) int
all_async_req_handled();

__attribute__((import_name("get_async_data"))) int get_async_data(uint32_t signo,
                                                                  uint32_t sub_signo,
                                                                  uint32_t dst_buf_addr,
                                                                  uint32_t dst_buf_len_addr);

__attribute__((import_name("mpu6050_sample_async"))) int
mpu6050_sample_async(uint32_t i2c_no, uint32_t addr, uint32_t handler_func_ptr);

__attribute__((import_name("at24cxx_write_async"))) int
at24cxx_write_async(uint32_t i2c_no,uint32_t i2c_addr, uint32_t write_addr, uint32_t src_buf_addr, uint32_t len, uint32_t handler_func_ptr);


__attribute__((import_name("sign_ptr")))
uint32_t sign_ptr(uint32_t original_ptr, uint32_t signature, uint32_t tag);

__attribute__((import_name("native_wait"))) int 
native_wait(int ms);

__attribute__((import_name("record_benchmark_time"))) int record_benchmark_time(uint32_t idx);
__attribute__((import_name("get_benchmark_time"))) int get_benchmark_time(uint32_t idx, uint32_t out_time_addr);



int wait_ms = 1000/(int)SAMPLE_FREQ;
volatile float sampleFreq=SAMPLE_FREQ;
volatile float beta = BETA_DEF;								// 2 * proportional gain (Kp)
volatile float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;	// quaternion of sensor frame relative to auxiliary frame

float inv_sqrt(float x) {
	float halfx = 0.5f * x;
	float y = x;
	long i = *(long*)&y;
	i = 0x5f3759df - (i>>1);
	y = *(float*)&i;
	y = y * (1.5f - (halfx * y * y));
	return y;
}

void madgwick_ahrs_update(float gx, float gy, float gz, float ax, float ay, float az){
    float recipNorm;
	float s0, s1, s2, s3;
	float qDot1, qDot2, qDot3, qDot4;
	float _2q0, _2q1, _2q2, _2q3, _4q0, _4q1, _4q2 ,_8q1, _8q2, q0q0, q1q1, q2q2, q3q3;

	// Rate of change of quaternion from gyroscope
	qDot1 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
	qDot2 = 0.5f * (q0 * gx + q2 * gz - q3 * gy);
	qDot3 = 0.5f * (q0 * gy - q1 * gz + q3 * gx);
	qDot4 = 0.5f * (q0 * gz + q1 * gy - q2 * gx);

	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
	if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {

		// Normalise accelerometer measurement
		recipNorm = inv_sqrt(ax * ax + ay * ay + az * az);
		ax *= recipNorm;
		ay *= recipNorm;
		az *= recipNorm;   

		// Auxiliary variables to avoid repeated arithmetic
		_2q0 = 2.0f * q0;
		_2q1 = 2.0f * q1;
		_2q2 = 2.0f * q2;
		_2q3 = 2.0f * q3;
		_4q0 = 4.0f * q0;
		_4q1 = 4.0f * q1;
		_4q2 = 4.0f * q2;
		_8q1 = 8.0f * q1;
		_8q2 = 8.0f * q2;
		q0q0 = q0 * q0;
		q1q1 = q1 * q1;
		q2q2 = q2 * q2;
		q3q3 = q3 * q3;

		// Gradient decent algorithm corrective step
		s0 = _4q0 * q2q2 + _2q2 * ax + _4q0 * q1q1 - _2q1 * ay;
		s1 = _4q1 * q3q3 - _2q3 * ax + 4.0f * q0q0 * q1 - _2q0 * ay - _4q1 + _8q1 * q1q1 + _8q1 * q2q2 + _4q1 * az;
		s2 = 4.0f * q0q0 * q2 + _2q0 * ax + _4q2 * q3q3 - _2q3 * ay - _4q2 + _8q2 * q1q1 + _8q2 * q2q2 + _4q2 * az;
		s3 = 4.0f * q1q1 * q3 - _2q1 * ax + 4.0f * q2q2 * q3 - _2q2 * ay;
		recipNorm = inv_sqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3); // normalise step magnitude
		s0 *= recipNorm;
		s1 *= recipNorm;
		s2 *= recipNorm;
		s3 *= recipNorm;

		// Apply feedback step
		qDot1 -= beta * s0;
		qDot2 -= beta * s1;
		qDot3 -= beta * s2;
		qDot4 -= beta * s3;
	}

	// Integrate rate of change of quaternion to yield quaternion
	q0 += qDot1 * (1.0f / sampleFreq);
	q1 += qDot2 * (1.0f / sampleFreq);
	q2 += qDot3 * (1.0f / sampleFreq);
	q3 += qDot4 * (1.0f / sampleFreq);

	// Normalise quaternion
	recipNorm = inv_sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
	q0 *= recipNorm;
	q1 *= recipNorm;
	q2 *= recipNorm;
	q3 *= recipNorm;
}

void parse_mpu6050_data(uint8_t data[MPU6050_DATA_LEN], float * ax, float * ay, float * az, float * gx, float * gy, float* gz, float * temp){
    int16_t ax_tmp=(data[0]<<8)|data[1];
    int16_t ay_tmp=(data[2]<<8)|data[3];
    int16_t az_tmp=(data[4]<<8)|data[5];
    int16_t temp_tmp = (data[6]<<8)|data[7];
    int16_t gx_tmp=(data[8]<<8)|data[9];
    int16_t gy_tmp=(data[10]<<8)|data[11];
    int16_t gz_tmp=(data[12]<<8)|data[13];
    *temp=(float)temp_tmp/340.0 + 36.53;
    *ax = (float)ax_tmp / 1638.4;
    *ay = (float)ay_tmp / 1638.4;
    *az = (float)az_tmp / 1638.4;
    *gx = (float)gx_tmp / 131.0; 
    *gy = (float)gy_tmp / 131.0;
    *gz = (float)gz_tmp / 131.0;
}

void get_euler_angles(float * pitch, float* roll, float* yaw){
	*pitch = m_atan2(2.0f * (q0 * q1 + q2 * q3), 1.0f - 2.0f * (q1 * q1 + q2 * q2)) * 57.2958f;
    *roll = m_asin(2.0f * (q0 * q2 - q3 * q1)) * 57.2958f;
    *yaw = m_atan2(2.0f * (q0 * q3 + q1 * q2), 1.0f - 2.0f * (q2 * q2 + q3 * q3)) * 57.2958f;
}

float get_acceleration_magnitude(float ax, float ay, float az) {
    return m_sqrt(ax * ax + ay * ay + az * az);
}

uint8_t detect_fall(float prev_pitch, float prev_roll, float pitch, float roll, float acc_magnitude) {
    float pitch_rate = m_fabs(pitch - prev_pitch);
    float roll_rate = m_fabs(roll - prev_roll);
    if ((pitch_rate > 30.0f || roll_rate > 30.0f) && acc_magnitude > 19.6f) {
        return 1; 
    }
    return 0;
}

uint32_t len=MPU6050_DATA_LEN;
uint8_t data[MPU6050_DATA_LEN+1]={0};
float ax, ay, az, gx, gy, gz,temp;
float pitch, roll, yaw;
float prev_pitch=0.0f, prev_roll=0.0f;
float acc_magnitude;

// sign pointers
uint32_t signed_data;
uint32_t signed_null;
uint32_t signed_len;

int at24cxx_handler(uint32_t signo, uint32_t sub_signo){
    int res=get_async_data(signo,sub_signo,NULL,0);
    if(res<0){
        printf("write failed with %x\n",res);
    }
    return 0;
}

int mpu6050_handler(uint32_t signo, uint32_t sub_signo){
    len=MPU6050_DATA_LEN;
    signed_data=sign_ptr(data,SIGNATURE,TAG);
    signed_len=sign_ptr(&len,SIGNATURE,TAG);
    int res=get_async_data(signo,sub_signo,signed_data,signed_len);
    if(res<0){
        printf("get mpu6050 sample data failed with %x\n",res);
    }else{
        parse_mpu6050_data(data,&ax,&ay,&az,&gx,&gy,&gz,&temp);
    }
    madgwick_ahrs_update(gx,gy,gz,ax,ay,az);
    get_euler_angles(&pitch, &roll, &yaw);
    data[MPU6050_DATA_LEN]=detect_fall(prev_pitch,prev_roll,pitch,roll,get_acceleration_magnitude(ax,ay,az));
    res=at24cxx_write_async(AT24CXX_I2C_NO,AT24CXX_ADDR,0x00,signed_data,MPU6050_DATA_LEN+1,at24cxx_handler);
    if(res<0){
        printf("at24cxx_write_async failed with %x\n",res);
    }
    return 0;
}

int main(int argc, char **argv)
{
	for(int k=0;k<RUN_BENCHMARK_TIMES;k++){
		uint64_t start,end;
		int res;
		// sign pointers
		signed_data=sign_ptr(data,SIGNATURE,TAG);
		signed_len=sign_ptr(&len,SIGNATURE,TAG);
		
		if (mpu6050_init(MPU6050_I2C_NO, MPU6050_ADDR) == 0)
		{
			// send async sample first
			record_benchmark_time(0);
			
			for(int i=0;i<BENCHMARK_ITERATIONS;i++){
				
				// send async read for next detect
				res=mpu6050_sample_async(MPU6050_I2C_NO,MPU6050_ADDR,mpu6050_handler); 
				if(res<0){
                    printf("mpu6050_sample_async failed with %x\n",res);
                }
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
			printf("mpu6050 init failed\n");
		}
	}
    return 0;
}

