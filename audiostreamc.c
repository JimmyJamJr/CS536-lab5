#include "audiostream.h"


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


int main(int argc, char * argv[]) {
    if (argc != 8) {
        fprintf(stdout, "Please input 8 Command Line Arguments\n");
        // exit(1);
    }

	char * aud_file_name = argv[1];
	char * block_size = argv[2];
	char * buffer_size = argv[3];
	char * target_buf = argv[4];
	char * server_ip = argv[5];
	char * server_port = argv[6];
	char * log_file_name = argv[7];

    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);

    // Create struct for the server address
    struct sockaddr_in server_address = {.sin_family = AF_INET, .sin_port = atoi(server_port)};
    inet_pton(AF_INET, server_ip, &(server_address.sin_addr));

	// Create and send iniital packet to the server
	first_packet_t first_packet = {{0}, atoi(block_size)};
	memset(first_packet.file_name, ' ', 22);
	strncpy(first_packet.file_name, aud_file_name, 22);
	int sent = sendto(udp_sock, &first_packet, sizeof(first_packet_t), 0, (struct sockaddr*) &server_address, sizeof(server_address)); 

	printf("Sent inital packet to server %d\n", sent);

	return 0;
}

