SOURCE_DIR := $(shell pwd)
BUILD_DIR := ${SOURCE_DIR}/build

.PHONY: all

all:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR); \
	cmake .. \
		-DCMAKE_INSTALL_PREFIX=$(BUILD_DIR); \
	$(MAKE); \
	$(MAKE) install

clean:
	rm -fr $(BUILD_DIR)
