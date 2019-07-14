SOURCE_DIR = $(shell pwd)/..
BUILD_DIR = ${SOURCE_DIR}/.yarina/build

.PHONY: all clean

all: mp4v2 install

mp4v2:
	mkdir -p ${BUILD_DIR}
	cd ${SOURCE_DIR}; \
	[ -f GNUmakefile ] || \
	 ./configure \
		--prefix=${BUILD_DIR} \
		--enable-shared \
		--disable-debug \
		--disable-util; \
	${MAKE}; \
	${MAKE} install

install:
	cd ${BUILD_DIR}; \
	cp -r ./lib/* ${ROOT_DIR}/.yarina/share/lib; \
	cp -r ./include/* ${ROOT_DIR}/.yarina/share/include

clean:
	rm -rf ${BUILD_DIR}
	if [ -e ${SOURCE_DIR}/GNUmakefile ]; then \
		${MAKE} -C ${SOURCE_DIR} distclean; \
		rm -f ${SOURCE_DIR}/GNUmakefile; \
	fi
