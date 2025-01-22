#include "tz_periph_io_helper.h"
#include "wasm_export.h"
#include "pta_io_helper.h"

int32_t tzio_periph_to_io_helper_uuid(uint32_t periph_type,uint32_t periph_no,TEE_UUID * io_helper_uuid){
    if(periph_type>(uint32_t)PERIPH_SPI) return -1;
    if((periph_type_t)periph_type==PERIPH_GPIO){
        if(periph_no==22){
            TEE_UUID uuid=GPIO_22_TA_IO_HELPER_UUID;
            *io_helper_uuid=uuid; // copy uuid
            return 0;
        }
    }
    return -1;
}

int32_t tzio_signo_to_io_helper_uuid(uint32_t signo,TEE_UUID * io_helper_uuid){
    if(signo==22){
        TEE_UUID uuid=GPIO_22_TA_IO_HELPER_UUID;
        *io_helper_uuid=uuid; // copy uuid
        return 0;
    }
    return -1;
}