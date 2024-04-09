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
        exit(1);
    }

     int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* Initialize socket structs */

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = atoi(argv[6]);
    inet_aton(argv[5], (struct in_addr *) &addr.sin_addr.s_addr);


    bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));


}

