global-incdirs-y += include
srcs-y += trusted_flash_ta.c
srcs-y += tzio.c
srcs-y += lz4.c
srcs-y += fingerprint.c
srcs-y += math.c


# To remove a certain compiler flag, add a line like this
#cflags-template_ta.c-y += -Wno-strict-prototypes
