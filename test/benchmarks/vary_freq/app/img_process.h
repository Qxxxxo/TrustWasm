#ifndef IMG_PROCESS_H
#define IMG_PROCESS_H
#include <stdint.h>
#include <stdlib.h>
#include "mmath.h"

void generate_gaussian_kernel(uint32_t size, double sigma, double* kernel) {
    double sum = 0.0;
    int center = size / 2;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            double x = i - center;
            double y = j - center;
            int index = i * size + j;  
            kernel[index] = m_exp(-(x * x + y * y) / (2 * sigma * sigma))/2.0f*M_PI*sigma*sigma;
            sum += kernel[index];
        }
    }
    for (int i = 0; i < size * size; i++) {
        kernel[i] /= sum;
    }
}

void gaussian_blur(uint8_t * img, uint32_t width, uint32_t height, double * kernel, uint32_t kernel_size){
    int half_size = kernel_size / 2;
    uint8_t* new_data = (uint8_t *)malloc(width*height);
    // conv
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double new_pixel = 0.0;
            for (int ky = -half_size; ky <= half_size; ky++) {
                for (int kx = -half_size; kx <= half_size; kx++) {
                    int pixel_x = x + kx;
                    int pixel_y = y + ky;
                    // boundary check
                    if (pixel_x >= 0 && pixel_x < width && pixel_y >= 0 && pixel_y < height) {
                        int pixel_idx = pixel_y * width + pixel_x;
                        int kernel_idx = (ky + half_size) * kernel_size + (kx + half_size);
                        new_pixel += img[pixel_idx] * kernel[kernel_idx];
                    }
                }
            }
            int new_idx = y * width + x;
            new_data[new_idx] = (uint8_t)new_pixel;
        }
    }
    // replace
    for (int i = 0; i < width * height; i++) {
        img[i] = new_data[i];
    }
    free(new_data);
}

void otsu_threshold(uint8_t * img, uint32_t width, uint32_t height){
    int hist[256] = {0};
    int total_pixels = width * height;

    // 计算直方图
    for (int i = 0; i < total_pixels; i++) {
        hist[img[i]]++;
    }

    float sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += i * hist[i];
    }

    float sumB = 0;
    int wB = 0;
    int wF = 0;
    float max_variance = 0;
    int threshold = 0;

    // Otsu threshold
    for (int t = 0; t < 256; t++) {
        wB += hist[t];
        if (wB == 0) continue;
        wF = total_pixels - wB;
        if (wF == 0) break;

        sumB += t * hist[t];
        float mB = sumB / wB;
        float mF = (sum - sumB) / wF;

        float variance = wB * wF * (mB - mF) * (mB - mF);
        if (variance > max_variance) {
            max_variance = variance;
            threshold = t;
        }
    }

    for (int i = 0; i < total_pixels; i++) {
        img[i] = (img[i] > threshold) ? 0 : 255;
    }
}

void erode(uint8_t *img, uint32_t width, uint32_t height, int *kernel, uint32_t kernel_size) {
    int half_size = kernel_size / 2;
    uint8_t* new_data = (uint8_t *)malloc(width * height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t min_pixel = 255;
            for (int ky = -half_size; ky <= half_size; ky++) {
                for (int kx = -half_size; kx <= half_size; kx++) {
                    int pixel_x = x + kx;
                    int pixel_y = y + ky;
                    if (pixel_x >= 0 && pixel_x < width && pixel_y >= 0 && pixel_y < height) {
                        int pixel_idx = pixel_y * width + pixel_x;
                        int kernel_idx = (ky + half_size) * kernel_size + (kx + half_size);
                        if (kernel[kernel_idx] > 0) {
                            min_pixel = (img[pixel_idx] < min_pixel) ? img[pixel_idx] : min_pixel;
                        }
                    }
                }
            }
            int new_idx = y * width + x;
            new_data[new_idx] = min_pixel;
        }
    }

    for (int i = 0; i < width * height; i++) {
        img[i] = new_data[i];
    }
    free(new_data);
}

void dilate(uint8_t *img, uint32_t width, uint32_t height, int *kernel, uint32_t kernel_size) {
    int half_size = kernel_size / 2;
    uint8_t* new_data = (uint8_t *)malloc(width * height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t max_pixel = 0;
            for (int ky = -half_size; ky <= half_size; ky++) {
                for (int kx = -half_size; kx <= half_size; kx++) {
                    int pixel_x = x + kx;
                    int pixel_y = y + ky;
                    if (pixel_x >= 0 && pixel_x < width && pixel_y >= 0 && pixel_y < height) {
                        int pixel_idx = pixel_y * width + pixel_x;
                        int kernel_idx = (ky + half_size) * kernel_size + (kx + half_size);
                        if (kernel[kernel_idx] > 0) {
                            max_pixel = (img[pixel_idx] > max_pixel) ? img[pixel_idx] : max_pixel;
                        }
                    }
                }
            }
            int new_idx = y * width + x;
            new_data[new_idx] = max_pixel;
        }
    }

    for (int i = 0; i < width * height; i++) {
        img[i] = new_data[i];
    }
    free(new_data);
}

void generate_cross_kernel(int size, int *kernel) {
    int half_size = size / 2;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (i == half_size || j == half_size) {
                kernel[i * size + j] = 1;
            } else {
                kernel[i * size + j] = 0;
            }
        }
    }
}

void generate_rectangular_kernel(int size, int *kernel) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            kernel[i * size + j] = 1;
        }
    }
}

void generate_circular_kernel(int size, int *kernel) {
    int half_size = size / 2;
    double radius = half_size;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            double distance = m_sqrt(m_pow(i - half_size, 2) + m_pow(j - half_size, 2));
            if (distance <= radius) {
                kernel[i * size + j] = 1;
            } else {
                kernel[i * size + j] = 0;
            }
        }
    }
}

#endif