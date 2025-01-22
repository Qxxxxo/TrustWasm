#ifndef FINGERPRINT_H
#define FINGERPRINT_H   
#include <stdint.h>

#define FINGERPRINT_ATK301_I2C_NO   1
#define ATK301_IMG_DATA_LEN 12800
#define ATK301_IMG_EXT_DATA_LEN 25600
#define ATK301_IMG_WIDTH 	160
#define ATK301_IMG_HEIGHT	160
#define GAUSSIAN_KERNEL_SIZE	5
#define GAUSSIAN_KERNEL_SIGMA	0.8
#define ERODE_KERNEL_SIZE	3
#define DILATE_KERNEL_SIZE	3

void generate_gaussian_kernel(uint32_t size, double sigma, double* kernel);
void gaussian_blur(uint8_t * img, uint32_t width, uint32_t height, double * kernel, uint32_t kernel_size);
void otsu_threshold(uint8_t * img, uint32_t width, uint32_t height);
void erode(uint8_t *img, uint32_t width, uint32_t height, int *kernel, uint32_t kernel_size);
void dilate(uint8_t *img, uint32_t width, uint32_t height, int *kernel, uint32_t kernel_size);
void generate_cross_kernel(int size, int *kernel);
void generate_rectangular_kernel(int size, int *kernel);
void generate_circular_kernel(int size, int *kernel);
void extend_fp_img(uint8_t *src_data, uint8_t * dst_data);

#endif