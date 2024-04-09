CC=gcc
CFLAGS=-Wall -std=gnu99

make: audiostreamc audiostreams

audiostreamc: audiostreamc.c
	$(CC) $(CFLAGS) -o audiostreamc audiostreamc.c

audiostreams: audiostreams.c
	$(CC) $(CFLAGS) -o audiostreams audiostreams.c

clean:
	rm -f audiostreams audiostreamc
