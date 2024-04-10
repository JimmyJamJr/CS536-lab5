#include "audiostream.h"

// ./audiostreams 1 1 1 logfileS 128.10.112.142  26260


int main(int argc, char * argv[]) {
    if (argc != 7) {
        fprintf(stdout, "Please input 7 Command Line Arguments\n");
        exit(1);
    }

    float lambda = atoi(argv[1]);
    int epsilon = atoi(argv[2]);
    int gamma = atoi(argv[3]);

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

        
        int received = recvfrom(sockfd, &first_packet, sizeof(first_packet_t), 0, (struct sockaddr *) &client_addr, &client_addr_len);
        printf("receieved %d\n", received);

        for (int i = 0; i < 22; i++) {
            printf("%c\n", first_packet.file_name[i]);
        }

        printf("%c.\n", first_packet.file_name[20]);
        if (first_packet.file_name[20] != ' ') {
            printf("Filename too long\n");
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

        int k = fork();
        if (k == 0) { // child process
            printf("Child! %s\n", ip_addr);

            struct sockaddr_in child_addr;
            memset(&child_addr, 0, sizeof(child_addr));
            child_addr.sin_family = AF_INET;
            child_addr.sin_port = 0;
            inet_aton(ip_addr, (struct in_addr *) &child_addr.sin_addr.s_addr);

            int client_num = client_count++;      

            float packetinterval = 1.0 / lambda;      
            



            // Create file name here
            char log_file[50] = "log_files/";
            char cnt[10];
            sprintf(cnt, "%d", client_num);
            strcat(log_file, argv[4]);
            strcat(log_file, "-");
            strcat(log_file, cnt);




            

            exit(1);
        }

    }

}