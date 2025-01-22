#include "math.h"

double  m_fabs(double x) {
    return (x < 0) ? -x : x;
}

double  m_sqrt(double x) {
    if (x < 0) return -1.0;
    if (x == 0 || x == 1) return x;

    double guess = x / 2.0;
    double epsilon = 1e-6;

    while ( m_fabs(guess * guess - x) > epsilon) {
        guess = (guess + x / guess) / 2.0;
    }
    return guess;
}

double  m_atan(double x) {
    double result = 0.0;
    double term = x;
    double x2 = x * x;
    int sign = 1;

    for (int i = 1; i <= 15; i += 2) {
        result += sign * term / i;
        term *= x2;
        sign = -sign;
    }
    return result;
}

double  m_atan2(double y, double x) {
    if (x > 0) {
        return  m_atan(y / x);
    } else if (x < 0 && y >= 0) {
        return  m_atan(y / x) + 3.141592653589793;
    } else if (x < 0 && y < 0) {
        return  m_atan(y / x) - 3.141592653589793;
    } else if (x == 0 && y != 0) {
        return (y > 0) ? 1.5707963267948966 : -1.5707963267948966;
    }
    return 0.0;
}

double  m_asin(double x) {
    if (x < -1 || x > 1) return -1.0;
    return  m_atan2(x,  m_sqrt(1 - x * x));
}

double m_factorial(int n) {
    double result = 1;
    for (int i = 1; i <= n; i++) {
        result *= i;
    }
    return result;
}

double m_normalize_input(double x) {
    while (x > 2 * 3.141592653589793) {
        x -= 2 * 3.141592653589793;
    }
    while (x < 0) {
        x += 2 * 3.141592653589793;
    }
    return x;
}

double m_sin(double x) {
    x=m_normalize_input(x);
    double result = 0;
    double term = x;
    int n = 1;
    while (term != 0) {
        result += term;
        n += 2;
        term = -term * x * x / (n * (n - 1)); 
    }
    return result;
}

double m_cos(double x) {
    x=m_normalize_input(x);
    double result = 1;
    double term = 1;
    int n = 2;
    while (term != 0) {
        term = -term * x * x / (n * (n - 1)); 
        result += term;
        n += 2;
    }
    return result;
}

double m_exp(double x){
    double result = 1.0;
    double term = 1.0;
    int n = 1;

    while (term > 1e-10 || term < -1e-10) {
        term *= x / n;
        result += term;
        n++;
    }

    return result;
}

double m_pow(double base, int exponent) {
    double result = 1.0;
    int exp_abs = exponent < 0 ? -exponent : exponent;

    for (int i = 0; i < exp_abs; i++) {
        result *= base;
    }
    if (exponent < 0) {
        result = 1.0 / result;
    }

    return result;
}