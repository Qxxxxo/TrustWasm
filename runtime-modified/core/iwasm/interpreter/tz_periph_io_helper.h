/**
 * @brief io helper
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#ifndef _TZ_PERIPH_IO_HELPER_H
#define _TZ_PERIPH_IO_HELPER_H

#include <tee_internal_api.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GPIO_22_TA_IO_HELPER_UUID \
	{0x3f4f0011,0xa4b6,0x462f,	\
		{0xbb,0xae,0x06,0x78,0x83,0x7f,0xce,0xfd}}



// int32_t tzio_periph_to_io_helper_uuid(uint32_t periph_type,uint32_t periph_no,TEE_UUID * io_helper_uuid);
// int32_t tzio_signo_to_io_helper_uuid(uint32_t signo,TEE_UUID * io_helper_uuid);

#ifdef __cplusplus
}
#endif

#endif

// we use pta io helper now so this file don't need