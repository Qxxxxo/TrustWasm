#ifndef USER_TA_HEADER_DEFINES_H
#define USER_TA_HEADER_DEFINES_H

/* To get the TA UUID definition */
#include <tzio_native_benchmarks_ta.h>

#define TA_UUID				TA_TZIO_NATIVE_BENCHMARKS_UUID

/*
 * TA properties: multi-instance TA, no specific attribute
 * TA_FLAG_EXEC_DDR is meaningless but mandated.
 */
#define TA_FLAGS			TA_FLAG_EXEC_DDR

/* Provisioned stack size */
#define TA_STACK_SIZE			(50 * 1024)

/* Provisioned heap size for TEE_Malloc() and friends */
#define TA_DATA_SIZE			(20*1024*1024)

/* The gpd.ta.version property */
#define TA_VERSION	"1.0"

/* The gpd.ta.description property */
#define TA_DESCRIPTION	"Benchmark for TEE use native IO func"

/* Extra properties */
#define TA_CURRENT_TA_EXT_PROPERTIES \
    { "org.XXXXXX.optee.examples.tzio_native_benchmarks.property1", \
	USER_TA_PROP_TYPE_STRING, \
        "tzio_native_benchmarks" }, \
    { "org.XXXXXX.optee.examples.tzio_native_benchmarks.property2", \
	USER_TA_PROP_TYPE_U32, &(const uint32_t){ 0x0010 } }

#endif /* USER_TA_HEADER_DEFINES_H */
