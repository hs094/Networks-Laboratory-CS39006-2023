#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <poll.h>

int main(){
    int sockfd;
    struct sockaddr_in server_address;
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8181);
    inet_aton("127.0.0.1", &server_address.sin_addr);
    socklen_t len;
    char server_req[512] = "[*] REQUEST::: SEND TIME--";
    sendto(sockfd, server_req, 512, 0, (const struct sockaddr *)&server_address, sizeof(server_address));
    struct pollfd time_info;
    time_info.fd = sockfd;
    time_info.events = POLLIN;
    char buf[512];
    memset(buf, 0, sizeof(buf));
    for(int i=1;i<=5;i++) {
        if(poll(&time_info, 1, 3000) > 0){
            recvfrom(sockfd, (char *)buf, sizeof(buf), 0, (struct sockaddr *)&server_address, &len);
            printf("[*] THE CURRENT TIME IS:- %s \n", buf);
            exit(0);
        }
        printf("TIME REQUEST:- {%d} ^FAILED^\n", i);
    }
    printf("[*] The Time from the Server was not Received after trying for 5 times [*]\n");
    exit(-1);
    close(sockfd);
    return 0;
}