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
    struct sockaddr_in serv_addr;
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8181);
    inet_aton("127.0.0.1", &serv_addr.sin_addr);

    int n;
    socklen_t len;
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    char* server_req = "Time please";
    sendto(sockfd, (const char *)server_req, strlen(server_req), 0, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
    printf("Time request sent to server\n");
    struct pollfd timeout;
    timeout.fd = sockfd;
    timeout.events = POLLIN;
    int cnt = 5;
    int recvd = 0;
    while(cnt--){
        if(poll(&timeout, 1, 3000) > 0){
            recvfrom(sockfd, (char *)buffer, sizeof(buffer), 0, (struct sockaddr *)&serv_addr, &len);
            recvd = 1;
            break;
        }
    }
    if(!recvd){
        printf("Timeout Error\n");
        exit(0);
    }
    printf("Time received from server is %s \n", buffer);
    close(sockfd);
}