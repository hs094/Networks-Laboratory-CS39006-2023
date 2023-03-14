#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include "mysocket.h"

#define PORT_2 50017
#define MAX_MSG_LEN 1000
// Server
int main() {
    int sockfd;
    if ((sockfd = my_socket(AF_INET, SOCK_MyTCP, 0)) < 0) {
        perror("my_socket");
        exit(1);
    }

    struct sockaddr_in u2_addr;
    struct sockaddr_in u1_addr;
    socklen_t u1_addr_len;

    memset(&u2_addr, 0, sizeof(u2_addr));
    u2_addr.sin_family = AF_INET;
    u2_addr.sin_port = htons(PORT_2);
    u2_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (my_bind(sockfd, (struct sockaddr *)&u2_addr, sizeof(u2_addr)) < 0) {
        perror("my_bind");
        exit(1);
    }

    char msg[MAX_MSG_LEN];
    // while (1) {
    //     u1_addr_len = sizeof(u1_addr);
    //     memset(msg, 0, MAX_MSG_LEN);
    //     int msg_len = my_recv(sockfd, msg, MAX_MSG_LEN, 0, (struct sockaddr *)&u1_addr, &u1_addr_len);
    //     if(msg_len == 0) {
    //         break;
    //     }
    //     if (msg_len < 0) {
    //         perror("my_recv");
    //         exit(1);
    //     } else {
    //         printf("%s", msg);
    //         fflush(stdout);
    //     }
    // }
    my_close(sockfd);
    return 0;
}
