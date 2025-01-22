CC      ?= $(CROSS_COMPILE)gcc
LD      ?= $(CROSS_COMPILE)ld
AR      ?= $(CROSS_COMPILE)ar
NM      ?= $(CROSS_COMPILE)nm
OBJCOPY ?= $(CROSS_COMPILE)objcopy
OBJDUMP ?= $(CROSS_COMPILE)objdump
READELF ?= $(CROSS_COMPILE)readelf

OBJS = main.o secure_logging.o
# add pta io helper header
CFLAGS += -Wall -I./include -I$(TEEC_EXPORT)/include -I$(TA_DEV_KIT_DIR)/include
#Add/link other required libraries here
LDADD += -lteec -lpthread -L$(TEEC_EXPORT)/lib

BINARY = pta_io_helper

.PHONY: all
all: $(BINARY)

$(BINARY): $(OBJS)
	$(CC) -o $@ $< $(LDADD)

.PHONY: clean
clean:
	rm -f $(OBJS) $(BINARY)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
