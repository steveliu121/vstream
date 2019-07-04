BUILD_DIR := $(shell pwd)/build

.PHONY: all install

all: example install

example:
	mkdir -p ${BUILD_DIR}
	cd ${BUILD_DIR}; \
	cmake ../.. \
		-DCMAKE_INSTALL_PREFIX=$(BUILD_DIR); \
	${MAKE}; \
	${MAKE} install

install:
	cd ${BUILD_DIR}; \
	cp ./bin/* ${ROOT_DIR}/.yarina/build

clean:
	rm -rf ${BUILD_DIR}
