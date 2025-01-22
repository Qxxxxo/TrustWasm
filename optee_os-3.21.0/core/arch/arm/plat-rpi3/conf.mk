PLATFORM_FLAVOR ?= rpi3_b_plus
include core/arch/arm/cpu/cortex-armv8-0.mk

$(call force,CFG_TEE_CORE_NB_CORE,4)

CFG_SHMEM_START ?= 0x08000000
# CFG_SHMEM_SIZE ?= 0x00400000
CFG_SHMEM_SIZE ?= 0x00900000
CFG_TZDRAM_START ?= 0x10100000
# CFG_TZDRAM_SIZE ?= 0x00F00000
CFG_TZDRAM_SIZE ?= 0x08000000 # 128MB
CFG_TEE_RAM_VA_SIZE ?= 0x00800000 # 4MB

CFG_CORE_HEAP_SIZE ?= 1048576 # added to 512K for PTA IO Helper
CFG_CORE_BGET_BESTFIT ?= y

$(call force,CFG_8250_UART,y)
$(call force,CFG_SECURE_TIME_SOURCE_CNTPCT,y)
$(call force,CFG_WITH_ARM_TRUSTED_FW,y)

# CFG_NUM_THREADS ?= 4
CFG_NUM_THREADS ?= 32
CFG_CRYPTO_WITH_CE ?= n

CFG_TEE_CORE_EMBED_INTERNAL_TESTS ?= y
CFG_WITH_STACK_CANARIES ?= y
CFG_WITH_STATS ?= y

arm32-platform-cflags += -Wno-error=cast-align
arm64-platform-cflags += -Wno-error=cast-align

$(call force,CFG_CRYPTO_SHA256_ARM32_CE,n)
$(call force,CFG_CRYPTO_SHA256_ARM64_CE,n)
$(call force,CFG_CRYPTO_SHA1_ARM32_CE,n)
$(call force,CFG_CRYPTO_SHA1_ARM64_CE,n)
$(call force,CFG_CRYPTO_AES_ARM64_CE,n)

ifeq ($(PLATFORM_FLAVOR),rpi3_b_plus)
$(call force,CFG_RPI3_GPIO,y)
$(call force,CFG_RPI3_I2C,y) # BSC I2C 
$(call force,CFG_RPI3_SIM_I2C,n) # GPIO SIM I2C
$(call force,CFG_RPI3_SPI,y) # SPI0
$(call force,CFG_DHT,y)
$(call force,CFG_BH1750FVI,y)
$(call force,CFG_W25QXX,y)
$(call force,CFG_MPU6050,y)
$(call force,CFG_TEL0157,y)
$(call force,CFG_AT24CXX,y)
$(call force,CFG_ATK301,y)
$(call force,CFG_OVERHEAD_BENCHMARK,y)
endif
