/*
 * Extended Kalman Filter for embedded processors
 *
 * Copyright (C) 2024 Simon D. Levy
 *
 * MIT License
 */

#include "math.h"
#include <stdbool.h>
#include <string.h>

/**
  * Floating-point precision defaults to single but can be made double via
    <tt><b>#define _float_t double</b></tt> before <tt>#include <tinyekf.h></tt>
  */
#ifndef __float_t
#define __float_t float
#endif

// Linear alegbra ////////////////////////////////////////////////////////////

/// @private
static void __mulmat(
        const __float_t * a, 
        const __float_t * b, 
        __float_t * c, 
        const int arows, 
        const int acols, 
        const int bcols)
{
    for (int i=0; i<arows; ++i) {
        for (int j=0; j<bcols; ++j) {
            c[i*bcols+j] = 0;
            for (int k=0; k<acols; ++k) {
                c[i*bcols+j] += a[i*acols+k] * b[k*bcols+j];
            }
        }
    }
}

/// @private
static void __mulvec(
        const __float_t * a, 
        const __float_t * x, 
        __float_t * y, 
        const int m, 
        const int n)
{
    for (int i=0; i<m; ++i) {
        y[i] = 0;
        for (int j=0; j<n; ++j)
            y[i] += x[j] * a[i*n+j];
    }
}

/// @private
static void __transpose(
        const __float_t * a, __float_t * at, const int m, const int n)
{
    for (int i=0; i<m; ++i)
        for (int j=0; j<n; ++j) {
            at[j*m+i] = a[i*n+j];
        }
}

/// @private
static void __addmat(
        const __float_t * a, const __float_t * b, __float_t * c, 
        const int m, const int n)
{
    for (int i=0; i<m; ++i) {
        for (int j=0; j<n; ++j) {
            c[i*n+j] = a[i*n+j] + b[i*n+j];
        }
    }
}

/// @private
static void __negate(__float_t * a, const int m, const int n)
{        
    for (int i=0; i<m; ++i) {
        for (int j=0; j<n; ++j) {
            a[i*n+j] = -a[i*n+j];
        }
    }
}

/// @private
static void __addeye(__float_t * a, const int n)
{
    for (int i=0; i<n; ++i) {
        a[i*n+i] += 1;
    }
}


/* Cholesky-decomposition matrix-inversion code, adapated from
http://jean-pierre.moreau.pagesperso-orange.fr/Cplus/_choles_cpp.txt */

/// @private
static int __choldc1(__float_t * a, __float_t * p, const int n) 
{
    for (int i = 0; i < n; i++) {
        for (int j = i; j < n; j++) {
            __float_t sum = a[i*n+j];
            for (int k = i - 1; k >= 0; k--) {
                sum -= a[i*n+k] * a[j*n+k];
            }
            if (i == j) {
                if (sum <= 0) {
                    return 1; /* error */
                }
                p[i] = m_sqrt(sum);
            }
            else {
                a[j*n+i] = sum / p[i];
            }
        }
    }

    return 0; // success:w
}

/// @private
static int __choldcsl(const __float_t * A, __float_t * a, __float_t * p, const int n) 
{
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            a[i*n+j] = A[i*n+j];
        }
    }
    if (__choldc1(a, p, n)) {
        return 1;
    }
    for (int i = 0; i < n; i++) {
        a[i*n+i] = 1 / p[i];
        for (int j = i + 1; j < n; j++) {
            __float_t sum = 0;
            for (int k = i; k < j; k++) {
                sum -= a[j*n+k] * a[k*n+i];
            }
            a[j*n+i] = sum / p[j];
        }
    }

    return 0; // success
}

/// @private
static int __cholsl(const __float_t * A, __float_t * a, __float_t * p, const int n) 
{
    if (__choldcsl(A,a,p,n)) {
        return 1;
    }

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            a[i*n+j] = 0.0;
        }
    }
    for (int i = 0; i < n; i++) {
        a[i*n+i] *= a[i*n+i];
        for (int k = i + 1; k < n; k++) {
            a[i*n+i] += a[k*n+i] * a[k*n+i];
        }
        for (int j = i + 1; j < n; j++) {
            for (int k = j; k < n; k++) {
                a[i*n+j] += a[k*n+i] * a[k*n+j];
            }
        }
    }
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < i; j++) {
            a[i*n+j] = a[j*n+i];
        }
    }

    return 0; // success
}

/// @private
static void __addvec(
        const __float_t * a, const __float_t * b, __float_t * c, const int n)
{
    for (int j=0; j<n; ++j) {
        c[j] = a[j] + b[j];
    }
}

/// @private
static void __sub(
        const __float_t * a, const __float_t * b, __float_t * c, const int n)
{
    for (int j=0; j<n; ++j) {
        c[j] = a[j] - b[j];
    }
}

/// @private
static bool _invert(const __float_t * a, __float_t * ainv)
{
    __float_t tmp[_EKF_M];

    return __cholsl(a, ainv, tmp, _EKF_M) == 0;
}


// EKF ///////////////////////////////////////////////////////////////////////

typedef struct {

    /** State vector **/
    __float_t x[_EKF_N];        

    /** Prediction error covariance **/
    __float_t P[_EKF_N*_EKF_N];  

} _ekf_t;

/**
 * Initializes the EKF
 * @param ekf pointer to an ekf_t structure
 * @param pdiag a vector of length _EKF_N containing the initial values for the
 * covariance matrix diagonal
 */
static void _ekf_initialize(_ekf_t * ekf, const __float_t pdiag[_EKF_N])
{
    for (int i=0; i<_EKF_N; ++i) {

        for (int j=0; j<_EKF_N; ++j) {

            ekf->P[i*_EKF_N+j] = i==j ? pdiag[i] : 0;
        }

        ekf->x[i] = 0;
    }
}

/**
  * Runs the EKF prediction step
  * @param ekf pointer to an ekf_t structure
  * @param fx predicted values
  * @param F Jacobian of state-transition function
  * @param Q process noise matrix
  * 
  */static void _ekf_predict(
        _ekf_t * ekf, 
        const __float_t fx[_EKF_N],
        const __float_t F[_EKF_N*_EKF_N],
        const __float_t Q[_EKF_N*_EKF_N])
{        
    // \hat{x}_k = f(\hat{x}_{k-1}, u_k)
    memcpy(ekf->x, fx, _EKF_N*sizeof(__float_t));

    // P_k = F_{k-1} P_{k-1} F^T_{k-1} + Q_{k-1}

    __float_t FP[_EKF_N*_EKF_N] = {};
    __mulmat(F, ekf->P,  FP, _EKF_N, _EKF_N, _EKF_N);

    __float_t Ft[_EKF_N*_EKF_N] = {};
    __transpose(F, Ft, _EKF_N, _EKF_N);

    __float_t FPFt[_EKF_N*_EKF_N] = {};
    __mulmat(FP, Ft, FPFt, _EKF_N, _EKF_N, _EKF_N);

    __addmat(FPFt, Q, ekf->P, _EKF_N, _EKF_N);
}

/// @private
static void _ekf_update_step3(_ekf_t * ekf, __float_t GH[_EKF_N*_EKF_N])
{
    __negate(GH, _EKF_N, _EKF_N);
    __addeye(GH, _EKF_N);
    __float_t GHP[_EKF_N*_EKF_N];
    __mulmat(GH, ekf->P, GHP, _EKF_N, _EKF_N, _EKF_N);
    memcpy(ekf->P, GHP, _EKF_N*_EKF_N*sizeof(__float_t));
}

/**
  * Runs the EKF update step
  * @param ekf pointer to an ekf_t structure
  * @param z observations
  * @param hx predicted values
  * @param H sensor-function Jacobian matrix
  * @param R measurement-noise matrix
  * 
  */
static bool _ekf_update(
        _ekf_t * ekf, 
        const __float_t z[_EKF_M], 
        const __float_t hx[_EKF_N],
        const __float_t H[_EKF_M*_EKF_N],
        const __float_t R[_EKF_M*_EKF_M])
{        
    // G_k = P_k H^T_k (H_k P_k H^T_k + R)^{-1}
    __float_t G[_EKF_N*_EKF_M];
    __float_t Ht[_EKF_N*_EKF_M];
    __transpose(H, Ht, _EKF_M, _EKF_N);
    __float_t PHt[_EKF_N*_EKF_M];
    __mulmat(ekf->P, Ht, PHt, _EKF_N, _EKF_N, _EKF_M);
    __float_t HP[_EKF_M*_EKF_N];
    __mulmat(H, ekf->P, HP, _EKF_M, _EKF_N, _EKF_N);
    __float_t HpHt[_EKF_M*_EKF_M];
    __mulmat(HP, Ht, HpHt, _EKF_M, _EKF_N, _EKF_M);
    __float_t HpHtR[_EKF_M*_EKF_M];
    __addmat(HpHt, R, HpHtR, _EKF_M, _EKF_M);
    __float_t HPHtRinv[_EKF_M*_EKF_M];
    if (!_invert(HpHtR, HPHtRinv)) {
        return false;
    }
    __mulmat(PHt, HPHtRinv, G, _EKF_N, _EKF_M, _EKF_M);

    // \hat{x}_k = \hat{x_k} + G_k(z_k - h(\hat{x}_k))
    __float_t z_hx[_EKF_M];
    __sub(z, hx, z_hx, _EKF_M);
    __float_t Gz_hx[_EKF_M*_EKF_N];
    __mulvec(G, z_hx, Gz_hx, _EKF_N, _EKF_M);
    __addvec(ekf->x, Gz_hx, ekf->x, _EKF_N);

    // P_k = (I - G_k H_k) P_k
    __float_t GH[_EKF_N*_EKF_N];
    __mulmat(G, H, GH, _EKF_N, _EKF_M, _EKF_N);
    _ekf_update_step3(ekf, GH);

    // success
    return true;
}