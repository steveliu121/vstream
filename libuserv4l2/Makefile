CC := gcc
OBJS := user_v4l2_capture.o
TARGET_LIB := libuserv4l2.so
CFLAGS := -Wall -fPIC

${TARGET_LIB}: ${OBJS}
	$(CC) -shared -o $@ $^ -Wl,-soname,libuserv4l2.so

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@
