CC := gcc
LIBOBJ = useraudio.o

.PHONY: all clean test

all: lib

lib: $(LIBOBJ)
	$(CC) --shared -o libuseraudio.so $^

%.o: %.c
	$(CC) -fPIC -I./ -o $@ -c $^

test:
	$(MAKE) -C test

clean:
	rm *.o
	rm lib*
	rm test/*.o
	rm test/audio_test
