export V ?= 0

OUTPUT_DIR := $(CURDIR)/out

HOST_CROSS_COMPILE ?= $(CROSS_COMPILE)
LIB_CROSS_COMPILE ?= $(CROSS_COMPILE)
TA_CROSS_COMPILE ?= $(CROSS_COMPILE)

.PHONY: all
all: rootfs

.PHONY: all_wamr
all_wamr:
	$(MAKE) -C host CROSS_COMPILE="$(HOST_CROSS_COMPILE)" --no-builtin-variables
	$(MAKE) -C ta CROSS_COMPILE="$(TA_CROSS_COMPILE)" LDFLAGS=""

.PHONY: clean
clean: rootfs-clean
	$(MAKE) -C host clean
	$(MAKE) -C ta clean

rootfs: all_wamr
	@echo "Copying example CA and TA binaries to $(OUTPUT_DIR)..."
	@mkdir -p $(OUTPUT_DIR)
	@mkdir -p $(OUTPUT_DIR)/ta
	@mkdir -p $(OUTPUT_DIR)/ca
	@if [ -e host/wamr_tzio ]; then \
		cp -p host/wamr_tzio $(OUTPUT_DIR)/ca/; \
	fi;
	@cp -pr ta/*.ta $(OUTPUT_DIR)/ta/; \

rootfs-clean:
	@rm -rf $(OUTPUT_DIR)/ta
	@rm -rf $(OUTPUT_DIR)/ca
	@rmdir --ignore-fail-on-non-empty $(OUTPUT_DIR) || test ! -e $(OUTPUT_DIR)