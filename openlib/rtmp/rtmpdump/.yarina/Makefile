BUILD_DIR = $(shell pwd)/build

#include common.mk

.PHONY: all install clean

all: rtmp install

rtmp:
	mkdir -p $(BUILD_DIR)
	cd ..; \
	$(MAKE) SYS=posix INC=-I$(ROOT_DIR)/.yarina/share/include; \
	$(MAKE) prefix=$(BUILD_DIR) install

install:
	cp -r $(BUILD_DIR)/lib/* ${ROOT_DIR}/.yarina/share/lib; \
	cp -r $(BUILD_DIR)/include/* ${ROOT_DIR}/.yarina/share/include

clean:
	rm -rf $(BUILD_DIR)
	cd ..; \
	$(MAKE) clean


