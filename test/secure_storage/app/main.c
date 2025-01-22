/**
 * @brief wasm use i2c native
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define PERIPH_NO 31

__attribute__((import_name("write_secure_object"))) int
write_secure_object(uint32_t id_addr, uint32_t id_len, uint32_t data_addr, uint32_t data_len);

__attribute__((import_name("read_secure_object"))) int
read_secure_object(uint32_t id_addr, uint32_t id_len, uint32_t data_addr, uint32_t data_len_addr);

__attribute__((import_name("delete_secure_object"))) int
delete_secure_object(uint32_t id_addr, uint32_t id_len);

__attribute__((import_name("write_secure_object_async_future"))) int
write_secure_object_async_future(uint32_t id_addr, uint32_t id_len, uint32_t data_addr, uint32_t data_len);

__attribute__((import_name("read_secure_object_async_future"))) int
read_secure_object_async_future(uint32_t id_addr, uint32_t id_len);

__attribute__((import_name("get_async_data_future"))) int get_async_data_future(uint32_t signo,
                                                                                uint32_t sub_signo,
                                                                                uint32_t dst_buf_addr,
                                                                                uint32_t dst_buf_len_addr);

__attribute__((import_name("native_wait"))) int 
native_wait(int ms);


int main(int argc, char **argv)
{
    int future;
    int res=0;
    char id[3]="key";
    char value[5]={0x54,0x56,0x57,0x50,0x48};
    char buf[10]={0};
    uint32_t len[1]={10};

    res=write_secure_object((uint32_t)id,3,(uint32_t)value,5);
    if(res<0){
        printf("failed to write secure object with %x\n",res);
    }else{
        printf("write secure object %s\n",value);
    }
    res=read_secure_object((uint32_t)id,3,(uint32_t)buf,len);
    if(res<0){
        printf("failed to read secure object with %x, need len %d\n",res,*len);
    }else{
        printf("read secure object '%s', len %d\n",buf,*len);
    }
    res=delete_secure_object((uint32_t)id,3);
    if(res<0){
        printf("failed to delete secure object with %x\n",res);
    }else{
        printf("delete secure object %s\n",id);
    }
    res=read_secure_object((uint32_t)id,3,(uint32_t)buf,len);
    if(res<0){
        printf("failed to read secure object with %x\n",res,*len);
    }else{
        printf("read secure object '%s', len %d\n",buf,*len);
    }
    // async secure object read write failed!
    
    // future=write_secure_object_async_future((uint32_t)id,3,(uint32_t)value,5);
    // if(future<0){
    //     printf("failed to send async write secure object with %x\n",future);
    // }else{
    //     res=get_async_data_future(PERIPH_NO,future,NULL,0);
    //     if(res<0){
    //         printf("failed to async write secure object with %x\n",res);
    //     }else{
    //         printf("async write secure object success\n");
    //     }
    // }

    // future=read_secure_object_async_future((uint32_t)id,3);
    // if(future<0){
    //     printf("failed to send async read secure object with %x\n",future);
    // }else{
    //     res=get_async_data_future(PERIPH_NO,future,buf,*len);
    //     if(res<0){
    //         printf("failed to async read secure object with %x\n",res);
    //     }else{
    //         printf("async write secure object success '%s', len %d\n",buf,*len);
    //     }
    // }

    // res=delete_secure_object((uint32_t)id,3);
    // if(res<0){
    //     printf("failed to delete secure object with %x\n",res);
    // }else{
    //     printf("delete secure object %s\n",id);
    // }
}

