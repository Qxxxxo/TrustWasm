subdirs-$(CFG_TEE_CORE_EMBED_INTERNAL_TESTS) += tests

srcs-$(CFG_ATTESTATION_PTA) += attestation.c
srcs-$(CFG_TEE_BENCHMARK) += benchmark.c
srcs-$(CFG_DEVICE_ENUM_PTA) += device.c
srcs-$(CFG_TA_GPROF_SUPPORT) += gprof.c
ifeq ($(CFG_WITH_USER_TA),y)
srcs-$(CFG_SECSTOR_TA_MGMT_PTA) += secstor_ta_mgmt.c
endif
srcs-$(CFG_WITH_STATS) += stats.c
srcs-$(CFG_SYSTEM_PTA) += system.c
srcs-$(CFG_SCP03_PTA) += scp03.c
srcs-$(CFG_APDU_PTA) += apdu.c
srcs-$(CFG_SCMI_PTA) += scmi.c
srcs-$(CFG_HWRNG_PTA) += hwrng.c
srcs-$(CFG_RTC_PTA) += rtc.c

subdirs-y += bcm
subdirs-y += stm32mp
subdirs-y += imx
subdirs-y += k3

cflags-$(CFG_OVERHEAD_BENCHMARK) += -DOVERHEAD_BENCHMARK
srcs-y += gpio_service.c
srcs-y += i2c_service.c
srcs-y += spi_service.c
srcs-$(CFG_DHT) += dht_service.c
srcs-$(CFG_BH1750FVI) += bh1750fvi_service.c
srcs-$(CFG_W25QXX) += w25qxx_service.c
srcs-$(CFG_MPU6050) += mpu6050_service.c
srcs-$(CFG_TEL0157) += tel0157_service.c
srcs-$(CFG_AT24CXX) += at24cxx_service.c
srcs-$(CFG_ATK301) += atk301_service.c

subdirs-y += io_helper
