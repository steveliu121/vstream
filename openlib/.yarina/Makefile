SOURCE_DIR := $(shell pwd)/..
FDK_AAC := ${SOURCE_DIR}/fdk-aac/.yarina
MP4V2 := ${SOURCE_DIR}/mp4v2/.yarina
#ZLIB := ${SOURCE_DIR}/zlib/.yarina
#OPENSSL := ${SOURCE_DIR}/openssl/.yarina
RTMP:= ${SOURCE_DIR}/rtmp/.yarina

.PHONY: all clean

all:
	${MAKE} -C ${FDK_AAC}
	${MAKE} -C ${MP4V2}
#	${MAKE} -C ${ZLIB}
#	${MAKE} -C ${OPENSSL}
	${MAKE} -C ${RTMP}

clean:
	${MAKE} -C ${FDK_AAC} clean
	${MAKE} -C ${MP4V2} clean
#	${MAKE} -C ${ZLIB} clean
#	${MAKE} -C ${OPENSSL} clean
	${MAKE} -C ${RTMP} clean
