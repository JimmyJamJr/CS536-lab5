CC=gcc
CFLAGS=-Wall -std=gnu99

make: audiostreamc audiostreams

audiostreamc: audiostreamc.c audiostream.h fifo.h
	$(CC) $(CFLAGS) -o audiostreamc audiostreamc.c -lasound -lm

audiostreams: audiostreams.c audiostream.h
	$(CC) $(CFLAGS) -o audiostreams audiostreams.c -lasound

clean:
	rm -f audiostreams audiostreamc
