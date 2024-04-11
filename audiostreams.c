#include "audiostream.h"

// ./audiostreams 1 1 1 logfileS 128.10.112.142  26260


#define CONTROLLAW 0


int main(int argc, char * argv[]) {
    if (argc != 7) {
        fprintf(stdout, "Please input 7 Command Line Arguments\n");
        exit(1);
    }

    int lambda = atoi(argv[1]);
    int epsilon = atoi(argv[2]);
    int gamma = atoi(argv[3]);
    const int q_star = 10;
    const int beta = 1;

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* Initialize socket structs */

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = atoi(argv[6]);
    inet_aton(argv[5], (struct in_addr *) &addr.sin_addr.s_addr);


    int bind_ret = bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));

    if (bind_ret != 0) {
        fprintf(stdout, "Error binding to port number. Exiting... ");
        exit(1);
    }

    int client_count = 0;

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        first_packet_t first_packet = {0};

        
        recvfrom(sockfd, &first_packet, sizeof(first_packet_t), 0, (struct sockaddr *) &client_addr, &client_addr_len);

        if (first_packet.file_name[20] != ' ') {
            fprintf(stdout, "Error: Filename too long\n");
            continue;
        }

        for (int i = 0; i < 22; i++) {
            if (first_packet.file_name[i] == ' ') {
                first_packet.file_name[i] = '\0';
                break;
            }
        }

        unsigned short block_size = first_packet.block_size; 
        char * ip_addr = inet_ntoa(client_addr.sin_addr);
        int client_num = client_count++;

        int k = fork();
        if (k == 0) { // child process

            int child_socket = socket(AF_INET, SOCK_DGRAM, 0);

            printf("client ip addr: %s\n", ip_addr);

            struct sockaddr_in child_addr;
            memset(&child_addr, 0, sizeof(child_addr));
            child_addr.sin_family = AF_INET;
            child_addr.sin_port = 0;
            inet_aton(argv[5], (struct in_addr *) &child_addr.sin_addr.s_addr);

            int child_bind = bind(child_socket, (struct sockaddr *) &child_addr, sizeof(child_addr));
            

            if (child_bind != 0) {
                fprintf(stdout, "Error binding to port number. Exiting Child... ");
                exit(1);
            }

            FILE * audio_file = fopen(first_packet.file_name, "r");
            if (audio_file == NULL) {
                fprintf(stdout, "Error: Could not open audio file for cleint %d\n", client_num);
                exit(1);
            }

            fseek(audio_file, 0, SEEK_END);
            int file_size = ftell(audio_file);

            fseek(audio_file, 0, SEEK_SET);

            int packet_nums = file_size / block_size;
            if (file_size % block_size != 0) {
                packet_nums++;
            }
            printf("packet_nums: %d\n", packet_nums);

            char ** packets = (char **) malloc(sizeof(char *) * packet_nums);
            assert(packets != NULL);

            float * lambda_vals = (int *) malloc(sizeof(int) * (packet_nums));
            assert(lambda_vals != NULL);

            for (int i = 0; i < packet_nums; i++) {
                packets[i] = (char *) malloc(sizeof(char) * (block_size));
                assert(packets[i] != NULL);
                fread(packets[i], sizeof(char), block_size, audio_file);
            }


            long packetinterval = 1.0 / lambda * 1000000;

            struct timeval start_time;
            gettimeofday(&start_time, NULL);
            

            for (int i = 0; i < packet_nums; i++) {
                // nano sleep at start for fun!
                struct timespec tim1, tim2;
                tim1.tv_sec = 0;
                tim1.tv_nsec = packetinterval;
                nanosleep(&tim1, &tim2);
                int sent = sendto(child_socket, packets[i], block_size, 0, (struct sockaddr*) &client_addr, sizeof(client_addr));
                unsigned short bufferstate;
                int recieved = recvfrom(child_socket, &bufferstate, 3, 0, (struct sockaddr*) &client_addr, &client_addr_len);
                if (recieved != 2) {
                    fprintf(stdout, "Error: Bufferstate not correct size for client number: %d. Exiting...\n", client_num);
                }

                printf("client buffer has %d filled!\n", bufferstate);
                struct timeval time;
                gettimeofday(&time, NULL);
                float elapsed = 1.0 * ((time.tv_sec-start_time.tv_sec)*1000000 + (time.tv_usec-start_time.tv_usec)) / 1000;
                lambda_vals[i] = elapsed; 

                if (CONTROLLAW == 0) { // method d
                    lambda = lambda + epsilon * (q_star - bufferstate) + beta * (gamma - lambda);
                } else { // method c
                    lambda = lambda + epsilon * (q_star - bufferstate);
                }
                

                packetinterval = 1.0 / lambda * 1000000;

            }

            for (int i = 0; i < 5; i++) {
                printf("Sending empty packet %d\n", i);
                int sent = sendto(child_socket, NULL, 0, 0, (struct sockaddr*) &client_addr, sizeof(client_addr));
            }

            // Create file name here
            char log_file_name[50] = "server_log_files/";
            char cnt[10];
            sprintf(cnt, "%d", client_num);
            strcat(log_file_name, argv[4]);
            strcat(log_file_name, "-");
            strcat(log_file_name, cnt);

            FILE * log_file = fopen(log_file_name, "w");
            if (log_file == NULL) {
                fprintf(stdout, "Unable to open log file!\n");
            } else {
                for (int i = 0; i < packet_nums; i++) {
                    fprintf(log_file, "%0.3f, ", lambda_vals[i]);
                }
            }

            free(lambda_vals);
            for (int i = 0; i < packet_nums; i++) {
                free(packets[i]);
            }
            free(packets);
            fprintf(stdout, "Exiting child process at end of file transmission\n");
            exit(1);
        }

    }

}