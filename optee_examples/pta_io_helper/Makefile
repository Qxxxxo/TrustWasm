export V?=0

HOST_CROSS_COMPILE ?= $(CROSS_COMPILE)

.PHONY: all
all:
	$(MAKE) -C host CROSS_COMPILE="$(HOST_CROSS_COMPILE)" --no-builtin-variables

.PHONY: clean
clean:
	$(MAKE) -C host clean
