#include "audiostream.h"
#include "fifo.h"

#include <signal.h>

// ./audiostreamc kj.au 4096 81920 81920 128.10.112.142  26260 logfileC

static snd_pcm_t *mulawdev;
static snd_pcm_uframes_t mulawfrms;

void mulawopen(size_t *bufsiz) {
	snd_pcm_hw_params_t *p;
	unsigned int rate = 8000;

	snd_pcm_open(&mulawdev, "default", SND_PCM_STREAM_PLAYBACK, 0);
	snd_pcm_hw_params_alloca(&p);
	snd_pcm_hw_params_any(mulawdev, p);
	snd_pcm_hw_params_set_access(mulawdev, p, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(mulawdev, p, SND_PCM_FORMAT_MU_LAW);
	snd_pcm_hw_params_set_channels(mulawdev, p, 1);
	snd_pcm_hw_params_set_rate_near(mulawdev, p, &rate, 0);
	snd_pcm_hw_params(mulawdev, p);
	snd_pcm_hw_params_get_period_size(p, &mulawfrms, 0);
	*bufsiz = (size_t)mulawfrms;
	return;
}

// Write to audio device.
#define mulawwrite(x) snd_pcm_writei(mulawdev, x, mulawfrms)

// Close audio device.
void mulawclose(void) {
	snd_pcm_drain(mulawdev);
	snd_pcm_close(mulawdev);
}

// Handler which will print an error message and quit if server doesn't respond after 2
void sig_handler(int signum){
    printf("No response from the server after 2s, closing the client.\n");
    exit(-1);
}


int main(int argc, char * argv[]) {
    if (argc != 8) {
        fprintf(stdout, "Please input 8 Command Line Arguments\n");
        // exit(1);
    }

	char * aud_file_name = argv[1];
	unsigned short block_size = atoi(argv[2]);
	unsigned short buffer_size = atoi(argv[3]);
	char * target_buf = argv[4];
	char * server_ip = argv[5];
	char * server_port = argv[6];
	char * log_file_name = argv[7];

    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);

    // Create struct for the server address
    struct sockaddr_in server_address = {.sin_family = AF_INET, .sin_port = atoi(server_port)};
    inet_pton(AF_INET, server_ip, &(server_address.sin_addr));

	// Create and send iniital packet to the server
	first_packet_t first_packet = {{0}, block_size};
	memset(first_packet.file_name, ' ', 22);
	strncpy(first_packet.file_name, aud_file_name, 22);
    for (int i = strlen(aud_file_name); i < 22; i++) {
        first_packet.file_name[i] = ' ';
    }
    
	int sent = sendto(udp_sock, &first_packet, sizeof(first_packet_t), 0, (struct sockaddr*) &server_address, sizeof(server_address)); 

	printf("Sent inital packet to server %d\n", sent);

	// Create buffer to store audio data received
	char buf[buffer_size];
	memset(buf, 0, buffer_size);
	fifo_t client_buffer = {0};
	fifo_init(&client_buffer, buf, buffer_size);

	// Start an alarm to quit if no response from server for 2s
    signal(SIGALRM, sig_handler);
    ualarm(2000 * 1000, 0);

	// Receive audio data from the server and push it into the buffer
	uint32_t server_address_len = sizeof(server_address);
	char new_packet[block_size];
	while (1) {
		size_t received_size = recvfrom(udp_sock, &new_packet, block_size, 0, (struct sockaddr*) &server_address, &server_address_len);
		fifo_write(&client_buffer, new_packet, received_size);

		// Reset the alarm to 2 seconds upon receiving packet from server
		ualarm(2000 * 1000, 0);
	}

	

	return 0;
}

