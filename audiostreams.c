#include "audiostream.h"


int main(int argc, char * argv[]) {
    if (argc != 7) {
        fprintf(stdout, "Please input 7 Command Line Arguments\n");
        exit(1);
    }

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

        first_packet_t first_packet;

        
        recvfrom(sockfd, &first_packet, sizeof(first_packet_t), 0, (struct sockaddr *) &client_addr, &client_addr_len);

        if (first_packet.file_name[20] != ' ') {
            continue;
        }

        unsigned short block_size = first_packet.block_size; 

        int k = fork();
        if (k == 0) { // child process
            printf("Child!\n");
            

            exit(1);
        }

    }

}