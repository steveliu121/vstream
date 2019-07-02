CC:= gcc
CFLAGS := -Wall
TARGET := video_capture

${TARGET}: %: %.o
	${CC} -o $@ $^

%.o: %.c
	${CC} ${CFLAGS} -c $^ -o $@

