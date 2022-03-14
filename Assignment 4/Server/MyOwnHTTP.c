#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFSIZE 1024

int main(int argc, char *argv[])
{
    int sockfd, ret, new_socket;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFSIZE];

    FILE *acsess_log = fopen("AcsessLog.txt", "rb");
    char* hello = "Hello";
    fwrite(hello, 5, 1, acsess_log);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("[-]Error in socket creation");
        exit(1);
    }
    printf("[+]Server Socket is created.\n");

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Bind the socket to the address and port */
    ret = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret < 0)
    {
        perror("[-]Error in bind");
        exit(1);
    }
    printf("[+]Bind to the address.\n");

    /* Listen for incoming connections */
    if (listen(sockfd, 10) == 0)
    {
        printf("[+]Listening....\n");
    }
    else
    {
        perror("[-]Error in listen");
        exit(1);
    }
    while (1)
    {
        int addrlen = sizeof(client_addr);
        new_socket = accept(sockfd, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen);
        if (new_socket < 0)
        {
            perror("[-]Error in accept");
            exit(1);
        }
        printf("[+]Accepted connection from client.\n");

        /* Accept incoming connections */
        if (fork() == 0)
        {
            FILE *pdf_file;
            char recv1[300];
            recv(new_socket, recv1, 300, 0);

            char method[100];
            char file[100];
            char http_version[10];
            char host[100];
            char connection[100];
            char accept_lang[100];
            char content_lang[100];

            size_t content_length = 0;
            char content_type[100];

            // the two formats of the request are as follows
            // sprintf(message, "GET %s%s%s%s", path, " HTTP/1.1\r\nHost: ", host, "\r\nConnection: Close\r\n\r\n");
            // sprintf(message, "PUT %s HTTP/%s\r\nHost: %s\r\nConnection: close\r\nAccept-Language: en-us\r\nContent-language: en-us\r\nContent-length: %d\r\nContent-type: %s\r\n", filename, "1.1", host, total_bytes, "plain/text");
            char recv2[300] = {0};
            strcpy(recv2, recv1);
            printf("Request 2%s\n", recv1);
            char *token = strtok(recv1, " ");
            if (token != NULL)
            {
                strcpy(method, token);
                printf("\nMethod: %s\n", token);
                if (strcmp(method, "PUT") == 0 || strcmp(method, "put") == 0)
                {
                    token = strtok(NULL, " ");
                    if (token != NULL)
                    {
                        printf("Path: %s\n", token);
                        strcpy(file, token);
                        token = strtok(NULL, "\r\n");
                        if (token != NULL)
                        {
                            printf("HTTP Version: %s\n", token);
                            strcpy(http_version, token);
                            token = strtok(NULL, "\r\n");
                            if (token != NULL)
                            {
                                printf("%s\n", token);
                                strcpy(host, token);
                                token = strtok(NULL, "\r\n");
                                if (token != NULL)
                                {
                                    strcpy(connection, token);
                                    printf("%s\n", token);
                                    token = strtok(NULL, "\r\n");
                                    if (token != NULL)
                                    {
                                        strcpy(accept_lang, token);
                                        printf("%s\n", token);
                                        token = strtok(NULL, "\r\n");
                                        if (token != NULL)
                                        {
                                            strcpy(content_lang, token);
                                            printf("%s\n", token);
                                            token = strtok(NULL, "\r\n");
                                            if (token != NULL)
                                            {
                                                printf("%s\n", token);
                                                content_length = atoi(token);
                                                token = strtok(NULL, "\r\n");
                                                if (token != NULL)
                                                {
                                                    printf("%s\n", token);
                                                    strcpy(content_type, token);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else if (strcmp(method, "GET") == 0 || strcmp(method, "get") == 0)
                {
                    token = strtok(NULL, " ");
                    if (token != NULL)
                    {
                        printf("Path: %s\n", token);
                        strcpy(file, token);
                        token = strtok(NULL, "\r\n");
                        if (token != NULL)
                        {
                            printf("HTTP Version: %s\n", token);
                            strcpy(http_version, token);
                            token = strtok(NULL, "\r\n");
                            if (token != NULL)
                            {
                                printf("%s\n", token);
                                strcpy(host, token);
                                token = strtok(NULL, "\r\n");
                                if (token != NULL)
                                {
                                    strcpy(connection, token);
                                    printf("%s\n", token);
                                    // token = strtok(NULL, "\r\n");
                                    // if (token != NULL) {
                                    //     strcpy(accept_lang, token);
                                    //     printf("Accept-Language: %s\n", token);
                                    //     token = strtok(NULL, "\r\n");
                                    //     if (token != NULL) {
                                    //         strcpy(content_lang, token);
                                    //         printf("Content-Language: %s\n", token);
                                    //         token = strtok(NULL, "\r\n");
                                    //         if (token != NULL) {
                                    //             printf("Content-Length: %s\n", token);
                                    //             content_length = atoi(token);
                                    //             token = strtok(NULL, "\r\n");
                                    //             if (token != NULL) {
                                    //                 printf("Content-Type: %s\n", token);
                                    //                 strcpy(content_type, token);
                                    //             }
                                    //         }
                                    //     }
                                    // }
                                }
                            }
                        }
                    }
                }
            }
            char filename[100] = {0};
            strcpy(filename, file);
            token = strtok(file, ".");
            char *extension = strtok(NULL, ".");

            if (strcmp(method, "PUT") == 0 || strcmp(method, "put") == 0)
            {
                /* Open the PDF file in binary mode */
                pdf_file = fopen(filename, "wb");
                if (pdf_file == NULL)
                {
                    perror("[-]Error in opening file");
                    exit(1);
                }
                else
                {
                    printf("[+]File opened for writing\n");
                }
                size_t total_bytes = 0;
                char file[100];
                char http_version[10];
                char host[100];
                size_t content_length1 = 0;
                char content_type[100];
                sscanf(recv2, "PUT %s HTTP/%s\r\nHost: %s\r\nConnection: close\r\nAccept-Language: en-us\r\nContent-language: en-us\r\nContent-length: %d\r\nContent-type: %s\r\n", file, http_version, host, &content_length1, content_type);
                while ((total_bytes < content_length1) && (ret = recv(new_socket, buffer, BUFSIZE, 0)) > 0)
                {
                    {
                        printf("buffer is %s", buffer);
                        fwrite(buffer, 1, ret, pdf_file);
                        total_bytes += ret;
                    }
                }
                /* Close the socket and the file */
                char *response = (char *)malloc(1000 * sizeof(char));
                char *result = (char *)malloc(10 * sizeof(char));
                strcat(response, "HTTP/1.1 200 OK\r\n");
                strcat(response, "Content-Length: ");
                sprintf(result, "%d\r\n", total_bytes);
                strcat(response, result);
                strcat(response, "Content-Type: application/");
                strcat(response, extension);
                strcat(response, "\r\n");

                send(new_socket, response, strlen(response) + 1, 0);
            }
            else if (strcmp(method, "GET") == 0 || strcmp(method, "get") == 0)
            {
                printf("GET Request Received:-\n");
                printf("filename is %s\n", filename);
                char *pt = strtok(filename, "/");
                printf("filename is %s\n", pt);

                /* Open the PDF file in binary mode */
                pdf_file = fopen(pt, "rb");
                if (pdf_file == NULL)
                {
                    perror("[-]Error in opening file");
                    exit(1);
                }
                else
                {
                    printf("File opened successfully\n");
                }
                // send 200 ok message
                char *response = (char *)malloc(1000 * sizeof(char));
                char *result = (char *)malloc(10 * sizeof(char));
                strcat(response, "HTTP/1.1 200 OK\r\n");
                strcat(response, "Content-Length: ");
                sprintf(result, "%d\r\n", content_length);
                strcat(response, result);
                strcat(response, "Content-Type: application/");
                strcat(response, extension);
                strcat(response, "\r\n");

                send(new_socket, response, BUFSIZE, 0);
                size_t bytes_read = 0;
                while ((bytes_read = fread(buffer, 1, BUFSIZE, pdf_file)) > 0)
                {
                    // send(new_socket, buffer, BUFSIZE, 0);
                    send(new_socket, buffer, bytes_read, 0);
                    // printf("[%s]", buffer);
                }
                exit(0);
            }
            else
            {
                printf("Invalid Request Received:-\n%s\n", recv1);
            }

            close(new_socket);
            fclose(pdf_file);
            exit(0);
        }
    }
    close(new_socket);
    return 0;
}