global-incdirs-y += include
srcs-y += tzio_native_benchmarks_ta.c
srcs-y += tzio.c
srcs-y += math.c
srcs-y += fall_detection.c
srcs-y += gnss_track.c
srcs-y += env_monitor.c
srcs-y += fingerprint.c
srcs-y += lz4.c

# To remove a certain compiler flag, add a line like this
#cflags-template_ta.c-y += -Wno-strict-prototypes
