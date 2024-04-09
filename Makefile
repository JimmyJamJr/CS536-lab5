CC=gcc
CFLAGS=-Wall -std=gnu99

make: audiostreamc audiostreams

audiostreamc: audiostreamc.c audiostream.h
	$(CC) $(CFLAGS) -o audiostreamc audiostreamc.c

audiostreams: audiostreams.c audiostream.h
	$(CC) $(CFLAGS) -o audiostreams audiostreams.c

clean:
	rm -f audiostreams audiostreamc
