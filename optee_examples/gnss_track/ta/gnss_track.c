#include "gnss_track.h"
#include "math.h"

// positioning interval
const double GNSS_TRACK_T = 1;

// initial covariances of state noise, measurement noise
const double P_pos = 1000.0;
const double P_vel = 10.0;
const double P_ori = 1e-2;
const double P_acc_bias = 1e-6;
const double P_gyr_bias = 1e-8;
const double R0 = 36;


// Set fixed process-noise covariance matrix Q
const double kDegreeToRadian = M_PI / 180.;
const double sigma_pv = 10;
const double sigma_rp = 10 * kDegreeToRadian;
const double sigma_yaw = 100 * kDegreeToRadian;
const double q_p=sigma_pv*sigma_pv;
const double q_v=sigma_pv*sigma_pv;
const double q_rp=sigma_rp*sigma_rp;
const double q_yaw=sigma_yaw*sigma_yaw;
const double q_ba=0.02*0.02;
const double q_bg=0.02*0.02;

const double Q[15*15] = {
    q_p,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,q_p,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,q_p,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,q_v,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,q_v,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,q_v,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,q_rp,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,q_rp,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,q_yaw,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,q_ba,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,q_ba,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,q_ba,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,q_bg,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,q_bg,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,q_bg
};

// Set fixed measurement noise covariance matrix R ----------------------------
const double R[3*3] = {
    R0, 0, 0,
    0, R0, 0,
    0, 0, R0,
};

#define WGS84_A 6378137.0   // WGS84 ellipsoid semi-major axis (meters)
#define WGS84_F 1.0 / 298.257223563  // WGS84 ellipsoid flattening
#define WGS84_E2 (2 * WGS84_F - WGS84_F * WGS84_F) // WGS84 first eccentricity squared

// Convert geodetic coordinates (latitude, longitude, altitude) to ECEF (Earth-Centered, Earth-Fixed) Cartesian coordinates (x, y, z)
void gnss_track_latlon_to_ecef(double lat, double lon, double alt, double pos[3]) {
    // Convert latitude and longitude from degrees to radians
    double lat_rad = lat * M_PI / 180.0;
    double lon_rad = lon * M_PI / 180.0;

    // Precompute trigonometric functions of latitude and longitude
    double sinLat = m_sin(lat_rad);
    double cosLat = m_cos(lat_rad);
    double sinLon = m_sin(lon_rad);
    double cosLon = m_cos(lon_rad);

    // Calculate the radius of curvature in the prime vertical (N)
    double N = WGS84_A / m_sqrt(1 - WGS84_E2 * sinLat * sinLat);

    // Compute ECEF coordinates (x, y, z)
    pos[0] = (N + alt) * cosLat * cosLon;  // X coordinate in ECEF
    pos[1] = (N + alt) * cosLat * sinLon;  // Y coordinate in ECEF
    pos[2] = ((1 - WGS84_E2) * N + alt) * sinLat;  // Z coordinate in ECEF
}

void gnss_track_init(ekf_t * ekf)
{
    const double pdiag[15] = { P_pos, P_pos, P_pos, P_vel, P_vel, P_vel, P_ori, P_ori,P_ori,P_acc_bias,P_gyr_bias };

    ekf_initialize(ekf, pdiag);

    // position
    ekf->x[0] = -2.168816181271560e+006;
    ekf->x[1] =  4.386648549091666e+006;
    ekf->x[2] =  4.077161596428751e+006;

    // velocity
    ekf->x[3] = 0;
    ekf->x[4] = 0;
    ekf->x[5] = 0;
	
	// pitch/roll/yaw
	ekf->x[6] = 0;
    ekf->x[7] = 0;
    ekf->x[8] = 0;

    // acc bias
    ekf->x[9] = 0;
	ekf->x[10] = 0;
	ekf->x[11] = 9.8;

    // gyrs bias
    ekf->x[12] = 0;
	ekf->x[13] = 0;
	ekf->x[14] = 0;
}

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
        double H[6*15])
{ 
	double dt = GNSS_TRACK_T;
	// Extract the current state from the EKF structure
    double pos[3] = {ekf->x[0], ekf->x[1], ekf->x[2]};  // Position [x, y, z]
    double vel[3] = {ekf->x[3], ekf->x[4], ekf->x[5]};  // Velocity [vx, vy, vz]
    double ori[3] = {ekf->x[6], ekf->x[7], ekf->x[8]};  // Orientation [pitch, roll, yaw]
    double acc_bias[3] = {ekf->x[9], ekf->x[10], ekf->x[11]};  // Accelerometer bias
    double gyr_bias[3] = {ekf->x[12], ekf->x[13], ekf->x[14]};  // Gyroscope bias

    // Apply biases to accelerometer and gyroscope data
    double ax_corr = ax - acc_bias[0];
    double ay_corr = ay - acc_bias[1];
    double az_corr = az - acc_bias[2];
    double gx_corr = gx - gyr_bias[0];
    double gy_corr = gy - gyr_bias[1];
    double gz_corr = gz - gyr_bias[2];

    // State prediction function f(x), include time step (dt)
    fx[0] = pos[0] + vel[0] * dt;  // Position update: x = x + v * dt
    fx[1] = pos[1] + vel[1] * dt;
    fx[2] = pos[2] + vel[2] * dt;

    fx[3] = vel[0] + ax_corr * dt;  // Velocity update: v = v + a * dt
    fx[4] = vel[1] + ay_corr * dt;
    fx[5] = vel[2] + az_corr * dt;

    fx[6] = ori[0] + gx_corr * dt;  // Orientation update: yaw = yaw + ω * dt
    fx[7] = ori[1] + gy_corr * dt;
    fx[8] = ori[2] + gz_corr * dt;

    // Accelerometer and gyroscope bias remain constant in this model (no dynamic update)
    fx[9] = acc_bias[0];  // Accelerometer bias
    fx[10] = acc_bias[1]; 
    fx[11] = acc_bias[2];

    fx[12] = gyr_bias[0];  // Gyroscope bias
    fx[13] = gyr_bias[1];  
    fx[14] = gyr_bias[2];

    // Initialize the Jacobian matrix F (size 15x15), it describes the linearized model of the system
    memset(F, 0, 15 * 15 * sizeof(double));

    // F matrix: velocity affects position
    F[0 + 3] = dt;  // Position changes with velocity: x = x + v * dt
    F[1 + 4] = dt;  // Same for y
    F[2 + 5] = dt;  // Same for z

    // F matrix: acceleration affects velocity
    F[3 + 0] = dt;  // Velocity changes with acceleration: v = v + a * dt
    F[4 + 1] = dt;  // Same for vy
    F[5 + 2] = dt;  // Same for vz

    // F matrix: gyroscope affects orientation (yaw, pitch, roll)
    F[6 + 3] = dt;  // Orientation changes with angular velocity: yaw = yaw + ω * dt
    F[7 + 4] = dt;  // Same for pitch
    F[8 + 5] = dt;  // Same for roll

    // F matrix: bias remains constant (no change in bias)
    F[9 + 9] = 1;  // Bias in accelerometer is constant
    F[10 + 10] = 1;
    F[11 + 11] = 1;

    F[12 + 12] = 1;  // Gyroscope bias is constant
    F[13 + 13] = 1;
    F[14 + 14] = 1;

    // Observation model hx(x), assuming we're observing the position (x, y, z)
    hx[0] = pos[0];  // Position measurement: hx = position (x)
    hx[1] = pos[1];  // Position measurement: hx = position (y)
    hx[2] = pos[2];  // Position measurement: hx = position (z)
    // Initialize observation matrix H (size 6x15), it maps the state to the observation
    memset(H, 0, 6 * 15 * sizeof(double));
    // Observation matrix H: position directly relates to the state position
    H[0 + 0] = 1;  // x position measurement is directly related to x state
    H[1 + 1] = 1;  // y position measurement is directly related to y state
    H[2 + 2] = 1;  // z position measurement is directly related to z state
}

void gnss_track_parse_mpu6050_data(uint8_t data[14], double * ax, double * ay, double * az, double * gx, double * gy, double * gz, double * temp){
    int16_t ax_tmp=(data[0]<<8)|data[1];
    int16_t ay_tmp=(data[2]<<8)|data[3];
    int16_t az_tmp=(data[4]<<8)|data[5];
    int16_t temp_tmp = (data[6]<<8)|data[7];
    int16_t gx_tmp=(data[8]<<8)|data[9];
    int16_t gy_tmp=(data[10]<<8)|data[11];
    int16_t gz_tmp=(data[12]<<8)|data[13];
    *temp=(double)temp_tmp/340.0 + 36.53;
    *ax = (double)ax_tmp / 1638.4;
    *ay = (double)ay_tmp / 1638.4;
    *az = (double)az_tmp / 1638.4;
    *gx = (double)gx_tmp / 131.0; 
    *gy = (double)gy_tmp / 131.0;
    *gz = (double)gz_tmp / 131.0;
}