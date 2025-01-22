global-incdirs-y += include
global-incdirs-y += ../../../../../runtime-modified/core/iwasm/include
srcs-y += tzio_lib.c
cflags-$(CFG_USE_PTA_IO_HELPER) += -DUSE_PTA_IO_HELPER
cflags-$(CFG_USE_FAT_PTR_CHECK) += -DUSE_FAT_PTR_CHECK
# cflags-$(CFG_OVERHEAD_BENCHMARK) += -DOVERHEAD_BENCHMARK