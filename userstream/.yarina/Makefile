BUILD_DIR := $(shell pwd)/build

.PHONY: all install

all: userv4l2 install

userv4l2:
	mkdir -p ${BUILD_DIR}
	cd ${BUILD_DIR}; \
	cmake ../.. \
		-DCMAKE_INSTALL_PREFIX=$(BUILD_DIR); \
	${MAKE}; \
	${MAKE} install

install:
	cd ${BUILD_DIR}; \
	cp ./lib/* ${ROOT_DIR}/.yarina/share/lib; \
	cp ./include/* ${ROOT_DIR}/.yarina/share/include

clean:
	rm -rf ${BUILD_DIR}
