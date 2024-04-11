#include "audiostream.h"
#include "fifo.h"

#include <signal.h>
#include <sys/time.h>
#include <math.h>

// ./audiostreamc kj.au 4096 81920 40960 128.10.112.142 26260 logfileC

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

typedef struct log_entry {
	int Q;
	float time;
	struct log_entry * next;
} log_entry_t;

unsigned short block_size;
unsigned short buffer_size;
unsigned short target_buf;

struct sockaddr_in server_address;
int udp_sock;

struct timeval start_time = {0};
int transmission_started = 0;
fifo_t client_buffer = {0};
log_entry_t * log_list = NULL;

void log_occupency(int Q) {
	log_entry_t * new_log = malloc(sizeof(log_entry_t));
	new_log->Q = client_buffer.filled;
	printf("%d\n", new_log->Q);
	struct timeval curr_time = {0};
	gettimeofday(&curr_time, NULL);
	float elpased = (curr_time.tv_sec - start_time.tv_sec) * 1000 + (curr_time.tv_usec - start_time.tv_usec) / 1000.0;
	new_log->time = elpased;
	new_log->next = NULL;

	log_entry_t * prev = NULL;
	log_entry_t * curr = log_list;
	while (curr) {
		prev = curr;
		curr = curr->next;
	}

	if (prev) {
		new_log->next = curr;
		prev->next = new_log;
	}
	else {
		log_list = new_log;
	}
}

int get_latest_Q() {
	log_entry_t * curr = log_list;
	while (curr) {
		if (curr->next == NULL) {
			return curr->Q;
		}
		curr = curr->next;
	}
	return -1;
}

void send_buffer_occupancy() {
	int current_Q = get_latest_Q();
	printf("current Q: %d\n", current_Q);
	unsigned short num_to_send = 0;
	if (current_Q > target_buf) {
		float q = (current_Q - target_buf) * 1.0 / (buffer_size - target_buf);
		num_to_send = round(q * 10 + 10);
	}
	else if (current_Q < target_buf) {
		float q = (target_buf - current_Q) * 1.0 / target_buf;
		num_to_send = round(10 - (q * 10));
	}
	else {
		num_to_send = 10;
	}

	printf("sending q to server: %d\n", num_to_send);
	int sent = sendto(udp_sock, &num_to_send, sizeof(num_to_send), 0, (struct sockaddr*) &server_address, sizeof(server_address)); 
}


// Handler which will print an error message and quit if server doesn't respond after 2
// Or if the transmission has started, read from buffer when the alarm goes off
void sig_handler(int signum) {
	if (transmission_started == 1) {
		char read_buf[4096];
		fifo_read(&client_buffer, read_buf, 4096);
		printf("Reading from buffer\n");
		log_occupency(client_buffer.filled);
		send_buffer_occupancy();
		ualarm(313 * 1000, 0);
	}
	else {
		printf("No response from the server after 2s, closing the client.\n");
    	exit(-1);
	}
}


int main(int argc, char * argv[]) {
    if (argc != 8) {
        fprintf(stdout, "Please input 8 Command Line Arguments\n");
        // exit(1);
    }

	gettimeofday(&start_time, NULL);

	char * aud_file_name = argv[1];
	block_size = atoi(argv[2]);
	buffer_size = atoi(argv[3]);
	target_buf = atoi(argv[4]);
	char * server_ip = argv[5];
	char * server_port = argv[6];
	char * log_file_name = argv[7];

    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);

    // Create struct for the server address
    server_address.sin_family = AF_INET;
	server_address.sin_port = atoi(server_port);
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
	fifo_init(&client_buffer, buf, buffer_size);

	// Start an alarm to quit if no response from server for 2s
    signal(SIGALRM, sig_handler);
    ualarm(2000 * 1000, 0);

	// Receive audio data from the server and push it into the buffer
	uint32_t server_address_len = sizeof(server_address);
	char new_packet[block_size + 1];
	int empty_packets = 0;
	while (1) {
		// block size + 1 to include null terminal
		size_t received_size = recvfrom(udp_sock, &new_packet, block_size, 0, (struct sockaddr*) &server_address, &server_address_len);
		printf("Received %ld bytes from the server\n", received_size);
		if (received_size == 0) {
			empty_packets++;
		} else {
			empty_packets = 0;
		}

		if (empty_packets == 4) {
			// Server is done
			break;
		}
		
		if (transmission_started == 0) {
			transmission_started = 1;
			// Set the alarm to go off every 313 ms (for reading from buffer)
			alarm(0);
			ualarm(313 * 1000, 0);
		}
		
		if (received_size > 0) {
			fifo_write(&client_buffer, new_packet, received_size); // don't write the null character at the end
			log_occupency(client_buffer.filled);
			// write back how full the client_buffer is
			send_buffer_occupancy();

		}
		
	}

	

	return 0;
}

