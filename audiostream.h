#include <stdio.h>
#include <alsa/asoundlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>
#include <assert.h>


typedef struct first_packet {
    char file_name[22];
    unsigned short block_size;
} first_packet_t;

