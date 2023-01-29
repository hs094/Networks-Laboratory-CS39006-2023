#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(){
    int sockfd;
    struct sockaddr_in serv_addr;
    char buffer[50];

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Error opening socket");
        exit(1);
    }

    serv_addr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &serv_addr.sin_addr);
    serv_addr.sin_port = htons(20000);

    if((connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0){
        perror("Error on connect");
        exit(1);
    }
    printf("Connected to server\n");
    recv(sockfd, buffer, 50, 0);
    printf("%s\n", buffer);
    printf("Enter username: ");
    bzero(buffer, 50);
    fgets(buffer, 50, stdin);
    printf("\n");
    // scanf("%[^\n]%*c", buffer);
    send(sockfd, buffer, 50, 0);
    bzero(buffer, 50);
    recv(sockfd, buffer, 50, 0);
    if(strcmp(buffer, "FOUND") == 0){
        while(1){
            bzero(buffer, 50);
            printf("Enter a shell command: ");
            scanf("%[^\n]%*c", buffer);
            send(sockfd, buffer, strlen(buffer)+1, 0);
            printf("RESULT:\n");
            if(strcmp(buffer, "exit") == 0){
                break;
            }
            bzero(buffer, 50);
            // recv(sockfd, buffer, 50, 0);
            char* output = malloc(1000);
            memset(output, 0, 1000);
            int end = 0;
            while(!end){
                // printf("Waiting for output\n");
                memset(buffer, 0, 50);
                recv(sockfd, buffer, 50, 0);
                // printf("recv: %s\n", buffer);
                for(int i = 0; i < strlen(buffer); i++){
                    if(buffer[i] == '\n'){
                        buffer[i] = '\0';
                        end = 1;
                        break;
                    }
                }
                // printf("%s", buffer);
                // bzero(buffer, 50);
                // printf("end: %d\n", end);
                strcat(output, buffer);
            }
            // receive data in chunks of 50 bytes
            // int n;
            // while((n = recv(sockfd, buffer, 50, 0)) > 0){
            //     // printf("recv: %s\n", buffer);
            //     // if(strcmp(buffer, "$$$$") == 0){
            //     //     break;
            //     // }else if(strcmp(buffer, "####") == 0){
            //     //     break;
            //     // }
            //     // printf("recvd: %s\n", buffer);
            //     int end = 0;
            //     for(int i=0; i<strlen(buffer); i++){
            //         if(buffer[i] == '\n'){
            //             buffer[i] = '\0';
            //             end = 1;
            //             break;
            //         }
            //     }
            //     strcat(output, buffer);
            //     bzero(buffer, 50);
            //     if(end){
            //         break;
            //     }
            // }
            if(strcmp(output, "$$$$") == 0){
                printf("Invalid command\n");
            }else if(strcmp(output, "####") == 0){
                printf("Error in running command\n");
            }else{
                printf("%s\n", output);
            }
        }
    }else{
        printf("Invalid Username\n");
    }
    close(sockfd);
    return 0;
}
