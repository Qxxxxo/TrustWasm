/**
 * @brief wasm use native tel0157
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct{
    uint16_t year; 
    uint8_t month;
    uint8_t date;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
}tel0157_time_t;

typedef struct{
    uint32_t lonMMMMM;
    uint32_t latMMMMM;
    uint8_t lonDD;
    uint8_t lonMM;
    uint8_t latDD;
    uint8_t latMM;
    char lonDirection;
    char latDirection;
}tel0157_lon_lat_t;

__attribute__((import_name("tel0157_init"))) int
tel0157_init(uint32_t i2c_no, uint32_t addr, uint32_t gnss_mode, uint32_t rgb);

__attribute__((import_name("tel0157_deinit"))) int
tel0157_deinit(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("tel0157_get_gnss_len"))) int
tel0157_get_gnss_len(uint32_t i2c_no, uint32_t addr, uint32_t len_dst_addr);

__attribute__((import_name("tel0157_get_all_gnss"))) int
tel0157_get_all_gnss(uint32_t i2c_no, uint32_t addr, uint32_t dst_buf_addr,uint32_t len);

__attribute__((import_name("tel0157_get_utc_time"))) int
tel0157_get_utc_time(uint32_t i2c_no, uint32_t addr, uint32_t utc_dst_addr);

__attribute__((import_name("tel0157_get_gnss_mode"))) int
tel0157_get_gnss_mode(uint32_t i2c_no, uint32_t addr, uint32_t mode_dst_addr);

__attribute__((import_name("tel0157_get_num_sat_used"))) int
tel0157_get_num_sat_used(uint32_t i2c_no, uint32_t addr, uint32_t num_dst_addr);

__attribute__((import_name("tel0157_get_lon"))) int
tel0157_get_lon(uint32_t i2c_no, uint32_t addr, uint32_t lon_dst_addr);

__attribute__((import_name("tel0157_get_lat"))) int
tel0157_get_lat(uint32_t i2c_no, uint32_t addr, uint32_t lat_dst_addr);

__attribute__((import_name("tel0157_get_alt"))) int
tel0157_get_alt(uint32_t i2c_no, uint32_t addr, uint32_t alt_dst_addr);

__attribute__((import_name("tel0157_get_sog"))) int
tel0157_get_sog(uint32_t i2c_no, uint32_t addr, uint32_t sog_dst_addr);

__attribute__((import_name("tel0157_get_cog"))) int
tel0157_get_cog(uint32_t i2c_no, uint32_t addr, uint32_t cog_dst_addr);

__attribute__((import_name("tel0157_get_gnss_len_async_future"))) int
tel0157_get_gnss_len_async_future(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("tel0157_get_all_gnss_async_future"))) int
tel0157_get_all_gnss_async_future(uint32_t i2c_no, uint32_t addr, uint32_t len);

__attribute__((import_name("tel0157_get_utc_time_async_future"))) int
tel0157_get_utc_time_async_future(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("tel0157_get_gnss_mode_async_future"))) int
tel0157_get_gnss_mode_async_future(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("tel0157_get_num_sat_used_async_future"))) int
tel0157_get_num_sat_used_async_future(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("tel0157_get_lon_async_future"))) int
tel0157_get_lon_async_future(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("tel0157_get_lat_async_future"))) int
tel0157_get_lat_async_future(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("tel0157_get_alt_async_future"))) int
tel0157_get_alt_async_future(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("tel0157_get_sog_async_future"))) int
tel0157_get_sog_async_future(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("tel0157_get_cog_async_future"))) int
tel0157_get_cog_async_future(uint32_t i2c_no, uint32_t addr);

__attribute__((import_name("get_async_data_future"))) int get_async_data_future(uint32_t signo,
                                                                                uint32_t sub_signo,
                                                                                uint32_t dst_buf_addr,
                                                                                uint32_t dst_buf_len_addr);

__attribute__((import_name("native_sleep"))) int 
native_sleep(int ms);

void print_all_gnss(uint8_t * buf, uint32_t len){
    for(int i=0;i<len;i++){
        printf("%c",buf[i]);
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    uint16_t len;
    uint8_t * buf;
    tel0157_time_t time;
    tel0157_lon_lat_t lon_lat;
    double longitude;
    double longitudeDegree;
    double latitude; 
    double latitudeDegree;
    uint8_t gnss_mode;
    uint8_t num_sat_used;
    uint8_t alt[3];
    uint8_t sog[3];
    uint8_t cog[3];
    double alt_value;
    double sog_value;
    double cog_value;

    if (tel0157_init(1, 0x20,7,1) == 0)
    {
        // for(int i=0;i<10;i++){
        //     if(tel0157_get_utc_time(1,0x20,(uint32_t)&time)==0){
        //         printf("utc time: %d/%d/%d %d:%d:%d\n",time.year,time.month,time.date,time.hour,time.minute,time.second);
        //     }

        //     if(tel0157_get_gnss_mode(1,0x20,(uint32_t)&gnss_mode)==0){
        //         printf("gnss mode: %d\n",gnss_mode);
        //     }

        //     if(tel0157_get_num_sat_used(1,0x20,(uint32_t)&num_sat_used)==0){
        //         printf("num sat used: %d\n",num_sat_used);
        //     }

        //     if(tel0157_get_lon(1,0x20,(uint32_t)&lon_lat)==0){
        //         printf("lonDirection: %c, lonMMMMM: %d, lonDD: %d, lonMM %d\n",lon_lat.lonDirection, lon_lat.lonMMMMM, lon_lat.lonDD, lon_lat.lonMM);
        //         longitude=(double)lon_lat.lonDD*100.0 + ((double)lon_lat.lonMM) + (double)lon_lat.lonMMMMM/100000.0;
        //         longitudeDegree=(double)lon_lat.lonDD + (double)lon_lat.lonMM/60.0 + (double)lon_lat.lonMMMMM/100000.0/60.0;
        //         printf("longitude: %.2f, longitudeDegree: %.2f\n",longitude,longitudeDegree);
        //     }

        //     if(tel0157_get_lat(1,0x20,(uint32_t)&lon_lat)==0){
        //         printf("latDirection: %c, latMMMMM: %d, latDD: %d, latMM %d\n",lon_lat.latDirection, lon_lat.latMMMMM, lon_lat.latDD, lon_lat.latMM);
        //         latitude=(double)lon_lat.latDD*100.0 + ((double)lon_lat.latMM) + (double)lon_lat.latMMMMM/100000.0;
        //         latitudeDegree=(double)lon_lat.latDD + (double)lon_lat.latMM/60.0 + (double)lon_lat.latMMMMM/100000.0/60.0;
        //         printf("latitude: %.2f, latitudeDegree: %.2f\n",latitude,latitudeDegree);
        //     }

        //     if(tel0157_get_alt(1,0x20,(uint32_t)alt)==0){
        //         alt_value = (double)((uint16_t)(alt[0]&0x7F)<<8|alt[1])+(double)alt[2]/100.0;
        //         if(alt[0]&0x80){
        //             alt_value=-(alt_value);
        //         }
        //         printf("alt: %.2f\n",alt_value);
        //     }
        //     if(tel0157_get_sog(1,0x20,(uint32_t)sog)==0){
        //         sog_value = (double)((uint16_t)(sog[0]&0x7F)<<8|sog[1])+(double)sog[2]/100.0;
        //         if(sog[0]&0x80){
        //             *sog=-(*sog);
        //         }
        //         printf("sog: %.2f\n",sog_value);
        //     }
        //     if(tel0157_get_cog(1,0x20,(uint32_t)cog)==0){
        //         cog_value = (double)((uint16_t)(cog[0]&0x7F)<<8|cog[1])+(double)cog[2]/100.0;
        //         if(cog[0]&0x80){
        //             *cog=-(*cog);
        //         }
        //         printf("cog: %.2f\n",cog_value);
        //     }
            
        //     if(tel0157_get_gnss_len(1,0x20,(uint32_t)&len)!=0){
        //         printf("Failed to get gnss len\n");
        //         tel0157_deinit(1,0x20);
        //         return 0;
        //     }
        //     printf("get len %d\n",len);
        //     buf=(uint8_t*)malloc(len);
        //     if(tel0157_get_all_gnss(1,0x20,(uint32_t)buf,len)==0){
        //         print_all_gnss(buf,len);
        //     }
        //     native_sleep(1000);
        // }
        int res=0;
        uint32_t len=sizeof(tel0157_time_t);
        int future=tel0157_get_utc_time_async_future(1,0x20);
        if(future>=0){
            res=get_async_data_future(29,future,&time,&len);
            if(res>=0){
                printf("utc time: %d/%d/%d %d:%d:%d\n",time.year,time.month,time.date,time.hour,time.minute,time.second);
            }else{
                printf("get data failed with %x\n",res);
            }
        }else{
            printf("send async read failed with %x\n",future);
        }

        len=sizeof(tel0157_lon_lat_t);
        future=tel0157_get_lon_async_future(1,0x20);
        if(future>=0){
            res=get_async_data_future(29,future,&lon_lat,&len);
            if(res>=0){
                printf("lonDirection: %c, lonMMMMM: %d, lonDD: %d, lonMM %d\n",lon_lat.lonDirection, lon_lat.lonMMMMM, lon_lat.lonDD, lon_lat.lonMM);
            }else{
                printf("get data failed with %x\n",res);
            }
        }else{
            printf("send async read failed with %x\n",future);
        }

        tel0157_deinit(1, 0x20);
    }
    return 0;
}

