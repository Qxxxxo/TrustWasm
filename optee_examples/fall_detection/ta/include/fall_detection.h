
#ifndef FALL_DETECTION_H
#define FALL_DETECTION_H
#include <stdint.h>

#define FALL_SAMPLE_FREQ	200.0f		// sample frequency in Hz
#define FALL_BETA_DEF 0.1f		// 2 * proportional gain

void madgwick_ahrs_update(float gx, float gy, float gz, float ax, float ay, float az);
void parse_mpu6050_data(uint8_t data[14], float * ax, float * ay, float * az, float * gx, float * gy, float* gz, float * temp);
void get_euler_angles(float * pitch, float* roll, float* yaw);
float get_acceleration_magnitude(float ax, float ay, float az);
uint8_t detect_fall(float prev_pitch, float prev_roll, float pitch, float roll, float acc_magnitude);

#endif