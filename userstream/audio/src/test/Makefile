CC := gcc

audio_test: %: %.o
	$(CC) -o $@ $^ -L../ -luseraudio -lasound

%.o: %.c
	$(CC) -I../ -o $@ -c $^
