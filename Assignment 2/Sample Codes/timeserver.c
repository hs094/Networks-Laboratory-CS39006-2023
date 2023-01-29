#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <time.h>

int main(){
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0){
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(8181);

    if(bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    printf("UDP Server Running....\n");
    int n;
    socklen_t len;
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    len = sizeof(cliaddr);

    n = recvfrom(sockfd, (char *)buffer, sizeof(buffer), 0, (struct sockaddr *)&cliaddr, &len);
    buffer[n] = '\0';
    printf("%s\n", buffer);

    memset(buffer, 0, sizeof(buffer));

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    strcpy(buffer, asctime(&tm));   
    //sleep(20); // uncomment this line to test the timeout
    sendto(sockfd, (const char *)buffer, strlen(buffer), 0, (const struct sockaddr *)&cliaddr, sizeof(cliaddr));
    close(sockfd);      
}