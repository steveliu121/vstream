SOURCE_DIR := $(shell pwd)
ROOT_DIR := ${SOURCE_DIR}
BUILD_DIR := ${SOURCE_DIR}/.yarina/build
SHARE_DIR := ${SOURCE_DIR}/.yarina/share

OPENLIB := ${SOURCE_DIR}/openlib/.yarina
CONTAINER := ${SOURCE_DIR}/container/.yarina
ENCODER := ${SOURCE_DIR}/encoder/.yarina
USERSTREAM := ${SOURCE_DIR}/userstream/.yarina
RTMPCLI := ${SOURCE_DIR}/rtmpcli/.yarina
EXAMPLE := ${SOURCE_DIR}/example/.yarina

export ROOT_DIR

.PHONY: all

all:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(SHARE_DIR)/lib
	mkdir -p $(SHARE_DIR)/include
	$(MAKE) -C ${OPENLIB}
	$(MAKE) -C ${CONTAINER}
	$(MAKE) -C ${ENCODER}
	$(MAKE) -C ${RTMPCLI}
	$(MAKE) -C ${USERSTREAM}
	$(MAKE) -C ${EXAMPLE}

clean:
	$(MAKE) -C ${OPENLIB} clean
	$(MAKE) -C ${CONTAINER} clean
	$(MAKE) -C ${ENCODER} clean
	$(MAKE) -C ${RTMPCLI} clean
	$(MAKE) -C ${USERSTREAM} clean
	$(MAKE) -C ${EXAMPLE} clean

	rm -fr $(BUILD_DIR)
	rm -fr $(SHARE_DIR)
