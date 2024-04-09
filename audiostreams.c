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


    bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));

    

}