#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <dirent.h>
int main(){
    int sockfd, newsockfd, clilen;
    char buffer[50];
    struct sockaddr_in serv_addr, cli_addr;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Error opening socket");
        exit(1);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(20000);

    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        perror("Error on binding");
        exit(1);
    }
    listen(sockfd, 5);
    printf("Server Running....\n");
    while(1){
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if(newsockfd < 0){
            perror("Error on accept");
            exit(1);
        }
        if(fork() == 0){
            close(sockfd);
            bzero(buffer, 50);
            strcpy(buffer, "LOGIN:");
            send(newsockfd, buffer, strlen(buffer)+1, 0);
            bzero(buffer, 50);
            recv(newsockfd, buffer, 50, 0);
            printf("Username Received: %s", buffer);
            FILE* fp = fopen("users.txt", "r");
            char line[50];
            int found = 0;
            while(fgets(line, 50, fp) != NULL){
                if(strcmp(line, buffer) == 0){
                    found = 1;
                    break;
                }
            }
            fclose(fp);
            bzero(buffer, 50);
            if(found){
                strcpy(buffer, "FOUND");
                send(newsockfd, buffer, strlen(buffer)+1, 0);
                while(1){
                    char* msg_to_send = malloc(1000);
                    memset(msg_to_send, 0, 1000);
                    bzero(buffer, 50);
                    recv(newsockfd, buffer, 50, 0);
                    printf("Command Received: %s\n", buffer);
                    if(strcmp(buffer, "exit") == 0){
                        break;
                    }else if(strcmp(buffer, "pwd")==0){
                        // getcwd(buffer, 50);
                        // send(newsockfd, buffer, strlen(buffer)+1, 0);
                        getcwd(msg_to_send, 1000);
                    }
                    else if((strlen(buffer) == 3 && strcmp(buffer, "dir") == 0) || (buffer[0] == 'd' && buffer[1] == 'i' && buffer[2] == 'r' && buffer[3] == ' ')){
                        char* dir = malloc(100);
                        memset(dir, 0, 100);
                        if(strlen(buffer) == 3){
                            dir = ".";
                        }else{
                            int i = 4;
                            while(buffer[i] != '\0'){
                                dir[i-4] = buffer[i];
                                i++;
                            }
                        }
                        printf("dir: %s\n", dir);
                        DIR* directory = opendir(dir);
                        if(directory){
                            struct dirent* ent;
                            // char* files = malloc(100);
                            // memset(files, 0, 100);
                            // while((ent = readdir(dir)) != NULL){
                            //     strcat(files, ent->d_name);
                            //     strcat(files, " ");
                            // }
                            while((ent = readdir(directory)) != NULL){
                                strcat(msg_to_send, ent->d_name);
                                strcat(msg_to_send, " ");
                            }
                            // send(newsockfd, files, strlen(files)+1, 0);
                        }else{
                            // strcpy(buffer, "####");
                            // send(newsockfd, buffer, strlen(buffer)+1, 0);
                            strcpy(msg_to_send, "####");
                        }
                        closedir(directory);
                    }
                    else if((strlen(buffer) == 2 && strcmp(buffer, "cd") == 0) || (buffer[0] == 'c' && buffer[1] == 'd' && buffer[2] == ' '))
                    {
                        char* dir = malloc(100);
                        memset(dir, 0, 100);
                        if(strlen(buffer) == 2){
                            chdir(getenv("HOME"));
                            // getcwd(buffer, 50);
                            getcwd(msg_to_send, 1000);
                        }else{
                            int i = 3;
                            while(buffer[i] != '\0'){
                                dir[i-3] = buffer[i];
                                i++;
                            }
                            printf("dir: %s\n", dir);
                            DIR* d = opendir(dir);
                            if(d){
                                chdir(dir);
                                // getcwd(buffer, 50);
                                getcwd(msg_to_send, 1000);
                            }else{
                                // strcpy(buffer, "####");
                                strcpy(msg_to_send, "####");
                            }
                            closedir(d);
                        }
                        // send(newsockfd, buffer, strlen(buffer)+1, 0);
                    }else{
                        // strcpy(buffer, "$$$$");
                        // send(newsockfd, buffer, strlen(buffer)+1, 0);
                        strcpy(msg_to_send, "$$$$");
                    }
                    msg_to_send[strlen(msg_to_send)] = '\n';
                    // send(newsockfd, msg_to_send, strlen(msg_to_send)+1, 0);
                    // send msg_to_send through buffer
                    int i = 0;
                    // printf("len: %d\n", strlen(msg_to_send));
                    while(i < strlen(msg_to_send)){
                        // printf("i: %d\n", i);
                        // bzero(buffer, 50);
                        memset(buffer, 0, 50);
                        int j = 0;
                        while(j < 50 && i < strlen(msg_to_send)){
                            buffer[j] = msg_to_send[i];
                            j++;
                            i++;
                        }
                        // printf("sending: %s\n", buffer);
                        send(newsockfd, buffer, strlen(buffer)+1, 0);
                    }
                    memset(buffer, 0, 50);
                    memset(msg_to_send, 0, 1000);
                    // printf("End of msg_to_send\n");
                }
            }else{
                strcpy(buffer, "NOT-FOUND");
                send(newsockfd, buffer, strlen(buffer)+1, 0);
            }
            close(newsockfd);
            exit(0);
        }
        close(newsockfd);
    }
    return 0;
}