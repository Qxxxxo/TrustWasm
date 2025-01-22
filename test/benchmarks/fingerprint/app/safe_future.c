/**
 * @brief wasm get fingerprint
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "native_funcs.h"
#include "mmath.h"
#include "img_process.h"

#define AT24CXX_PERIPH_NO 28
#define ATK301_PERIPH_NO 29

#define AT24CXX_I2C_NO 0
#define ATK301_I2C_NO 1
#define ATK301_ADDR 0x27 
#define AT24CXX_ADDR 0x50 
#define ATK301_IMG_DATA_LEN 12800
#define ATK301_IMG_EXT_DATA_LEN 25600
#define ATK301_IMG_WIDTH 	160
#define ATK301_IMG_HEIGHT	160
#define GAUSSIAN_KERNEL_SIZE	5
#define GAUSSIAN_KERNEL_SIGMA	0.8
#define ERODE_KERNEL_SIZE	3
#define DILATE_KERNEL_SIZE	3

#define BENCHMARK_ITERATIONS 5
#define RUN_BENCHMARK_TIMES 1

void print_fingerprint_image(uint8_t * buf, uint32_t len){

    for(int i=0;i<len;i++){
        printf("0x%02x ",buf[i]);
        if(i%32==31){
            printf("\n");
        }
    }
    printf("\n");
}

void print_kernel(double * buf, uint32_t len){
    for(int i=0;i<len;i++){
        printf("%.3f ",buf[i]);
        if(i%GAUSSIAN_KERNEL_SIZE==GAUSSIAN_KERNEL_SIZE-1){
            printf("\n");
        }
    }
    printf("\n");
}

void extend_fp_img(uint8_t *src_data, uint8_t * dst_data){
    for(int i=0;i<12800;i++){
        dst_data[2*i]=(uint8_t)((src_data[i]>>4)&0xF)*17;
		dst_data[2*i+1]=(uint8_t)(src_data[i]&0xF)*17;
    }
}


int main(int argc, char **argv)
{
	for(int k=0;k<RUN_BENCHMARK_TIMES;k++){
		int res;
		int get_fp_future=-1;
		int upload_fp_future=-1;
		int write_future=-1;
		uint64_t start,end;
		double kernel[GAUSSIAN_KERNEL_SIZE*GAUSSIAN_KERNEL_SIZE]={0.0};
		int erode_kernel[ERODE_KERNEL_SIZE*ERODE_KERNEL_SIZE]={0};
		int dilate_kernel[DILATE_KERNEL_SIZE*DILATE_KERNEL_SIZE]={0};
		uint8_t src_fp_img[ATK301_IMG_DATA_LEN]={0};
		uint8_t dst_fp_img[2*ATK301_IMG_DATA_LEN]={0};
		uint32_t src_fp_img_len=ATK301_IMG_DATA_LEN;
		uint32_t signed_src_fp_img=sign_ptr(src_fp_img,SIGNATURE,TAG);
		uint32_t signed_dst_fp_img=sign_ptr(dst_fp_img,SIGNATURE,TAG);
		uint32_t signed_img_len=sign_ptr(&src_fp_img_len,SIGNATURE,TAG);
		if(atk301_init(ATK301_I2C_NO, ATK301_ADDR)==0){
			record_benchmark_time(0);
			// async send
			get_fp_future=atk301_get_fingerprint_image_async_future(ATK301_I2C_NO,ATK301_ADDR);
			upload_fp_future=atk301_upload_fingerprint_image_async_future(ATK301_I2C_NO,ATK301_ADDR);
			for(int i=0;i<BENCHMARK_ITERATIONS;i++){
				if(get_fp_future>=0){
					res=get_async_data_future(ATK301_PERIPH_NO,get_fp_future,NULL,0);
				}
				if(upload_fp_future>=0){
					res=get_async_data_future(ATK301_PERIPH_NO,upload_fp_future,signed_src_fp_img,signed_img_len);
					if(res<0){
						printf("failed to upload fp img with %x\n",res);
					}
				}
				if(write_future>=0){
					res=get_async_data_future(AT24CXX_PERIPH_NO,write_future,NULL,0);
					if(res<0){
						printf("failed to write with %x\n",res);
					}
				}
				if(i!=BENCHMARK_ITERATIONS-1){
					get_fp_future=atk301_get_fingerprint_image_async_future(ATK301_I2C_NO,ATK301_ADDR);
					upload_fp_future=atk301_upload_fingerprint_image_async_future(ATK301_I2C_NO,ATK301_ADDR);
				}
				extend_fp_img(src_fp_img, dst_fp_img);
				generate_gaussian_kernel(GAUSSIAN_KERNEL_SIZE,GAUSSIAN_KERNEL_SIGMA,kernel);
				generate_circular_kernel(ERODE_KERNEL_SIZE,erode_kernel);
				generate_circular_kernel(DILATE_KERNEL_SIZE,dilate_kernel);
				gaussian_blur(dst_fp_img,ATK301_IMG_WIDTH,ATK301_IMG_HEIGHT,kernel,GAUSSIAN_KERNEL_SIZE);
				otsu_threshold(dst_fp_img,ATK301_IMG_WIDTH,ATK301_IMG_HEIGHT);
				dilate(dst_fp_img,ATK301_IMG_WIDTH,ATK301_IMG_HEIGHT,dilate_kernel,DILATE_KERNEL_SIZE);
				erode(dst_fp_img,ATK301_IMG_WIDTH,ATK301_IMG_HEIGHT,erode_kernel,ERODE_KERNEL_SIZE);
				write_future=at24cxx_write_async_future(AT24CXX_I2C_NO,AT24CXX_ADDR,0x00,signed_dst_fp_img,ATK301_IMG_EXT_DATA_LEN);
			}
			record_benchmark_time(1);
			if(write_future>=0){
				res=get_async_data_future(AT24CXX_PERIPH_NO,AT24CXX_I2C_NO,NULL,0);
			}
			get_benchmark_time(0,&start);
			get_benchmark_time(1,&end);
			printf("%llu\n",end-start);
		}else{
			printf("init failed with %x\n",res);
		}
	}
    return 0;
}

