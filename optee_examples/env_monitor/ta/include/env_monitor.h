#ifndef ENV_MONITOR_H
#define ENV_MONITOR_H
#include <stdint.h>

#define _EKF_N  4
#define _EKF_M  7

#include <_tinyekf.h>

#define ENV_MONITOR_SEUCRE_OBJECT_ID "env_monitor"
#define ENV_MONITOR_SECURE_OBJECT_ID_LEN 11

extern const float _Q[_EKF_N*_EKF_N];
extern const float _R[_EKF_M*_EKF_M];
extern const float _F[_EKF_N*_EKF_N];
extern const float _H[_EKF_M*_EKF_N];

void env_monitor_init(_ekf_t * ekf);
void env_monitor_parse_mpu6050_temp(uint8_t data[14], double * temp);

#endif