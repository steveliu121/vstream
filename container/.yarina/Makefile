SOURCE_DIR := $(shell pwd)/..
FLVMUXER := ${SOURCE_DIR}/flvmuxer/.yarina
MP4MUXER := ${SOURCE_DIR}/mp4muxer/.yarina

.PHONY: all clean

all:
	${MAKE} -C ${FLVMUXER}
	${MAKE} -C ${MP4MUXER}

clean:
	${MAKE} -C ${FLVMUXER} clean
	${MAKE} -C ${MP4MUXER} clean
