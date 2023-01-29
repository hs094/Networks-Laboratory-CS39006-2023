#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <time.h>
void clearBuf(char *buf)
{
    memset(buf, 0, sizeof(buf));
}
void local_Time(char *buf)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    strcpy(buf, asctime(&tm));
}
int main(){
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0) {
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

    printf("[*] SERVER RUNNING ....\n");
    char buf[512];
    memset(buf, 0, 512);
    socklen_t len = sizeof(cliaddr);
    size_t n = recvfrom(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&cliaddr, &len);
    buf[n] = '\0';
    printf("%s [+] %ld No. of Bytes Received\n", buf, n);
    clearBuf(buf);
    local_Time(buf);
    // sleep(20); // to test the wait period
    printf("[*] REQUEST PROCESSED TIME SENT ....\n");
    sendto(sockfd, (const char *)buf, strlen(buf), 0, (const struct sockaddr *)&cliaddr, sizeof(cliaddr));
    close(sockfd);   
    return 0;   
}