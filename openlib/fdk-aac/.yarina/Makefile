BUILD_DIR = $(shell pwd)/build

.PHONY: all install clean
all: fdk-aac install

fdk-aac:
	mkdir -p ${BUILD_DIR}
	cd ..;\
	[ -f configure ] || \
	  autoreconf -fiv; \
	cd ${BUILD_DIR}; \
	[ -f Makefile ] || \
	  ../../configure \
		--prefix=${BUILD_DIR}; \
	${MAKE}; \
	${MAKE} install

install:
	cd ${BUILD_DIR}; \
	cp -r lib/* ${ROOT_DIR}/.yarina/share/lib; \
	cp -r include/* ${ROOT_DIR}/.yarina/share/include

clean:
	rm -rf ${BUILD_DIR}
