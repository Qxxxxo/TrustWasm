#ifndef GNSS_TRACK_H
#define GNSS_TRACK_H
#include <stdint.h>
// Size of state space
#define EKF_N 15

// Size of observation (measurement) space
#define EKF_M 3

// We need double precision to replicate the published results
#define _float_t double

#include "tinyekf.h"

// positioning interval
extern const double GNSS_TRACK_T;

// initial covariances of state noise, measurement noise
extern const double P_pos;
extern const double P_vel;
extern const double P_ori;
extern const double P_acc_bias;
extern const double P_gyr_bias;
extern const double R0;


// Set fixed process-noise covariance matrix Q
extern const double kDegreeToRadian;
extern const double sigma_pv;
extern const double sigma_rp;
extern const double sigma_yaw;
extern const double q_p;
extern const double q_v;
extern const double q_rp;
extern const double q_yaw;
extern const double q_ba;
extern const double q_bg;

extern const double Q[15*15];

// Set fixed measurement noise covariance matrix R ----------------------------
extern const double R[3*3];

void gnss_track_parse_mpu6050_data(uint8_t data[14], double * ax, double * ay, double * az, double * gx, double * gy, double * gz, double * temp);
void gnss_track_run_model(
        ekf_t * ekf, 
        double ax,
		double ay,
		double az,
		double gx,
		double gy,
		double gz,
        double fx[15],
        double F[15*15],
        double hx[6],
        double H[6*15]);
void gnss_track_init(ekf_t * ekf);
void gnss_track_latlon_to_ecef(double lat, double lon, double alt, double pos[3]);

#endif