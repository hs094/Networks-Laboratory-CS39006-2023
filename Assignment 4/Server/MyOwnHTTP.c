#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFSIZE 1024

void handlePut(int new_socket, int total_bytes, char* extension)
{
    /* Close the socket and the file */
    char* response = (char *)malloc(10000*sizeof(char));
    char* result = (char *)malloc(10*sizeof(char));
    strcat(response, "HTTP/1.1 200 OK\r\n");
    send(new_socket, "Hello", 6, 0);

    strcat(response, "Content-Length: ");
    sprintf(result, "%d\r\n", total_bytes);
    strcat(response, result);
    strcat(response, "Content-Type: application/");
    strcat(response, extension);
    strcat(response, "\r\n");
    printf("%s", response);
    // send(new_socket, response, strlen(response)+1, 0);
}

int main(int argc, char *argv[]) {
    int sockfd, ret, new_socket;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFSIZE];
    FILE *pdf_file;

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
    if (ret < 0) {
        perror("[-]Error in bind");
        exit(1);
    }
    printf("[+]Bind to the address.\n");

    /* Listen for incoming connections */
    if (listen(sockfd, 10) == 0) 
    {
        printf("[+]Listening....\n");
    } else 
    {
        perror("[-]Error in listen");
        exit(1);
    }

    /* Accept incoming connections */
    int addrlen = sizeof(client_addr);
    new_socket = accept(sockfd, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen);
    if (new_socket < 0) {
        perror("[-]Error in accept");
        exit(1);
    }
    printf("[+]Accepted connection from client.\n");
    char request1[300];
    memset(request1, '\0', 300);
    recv(new_socket, request1, 300, 0);
    printf("%s", request1);
    /* Open the PDF file in binary mode */
    char file[100];
    memset(file, '\0', 100);
    recv(new_socket, file, 100, 0);
    
    char filename[100] = {0};
    strcpy(filename,file);
    char* token = strtok(file,".");
    char* extension = strtok(NULL, ".");
    pdf_file = fopen(filename, "wb");
    if (pdf_file == NULL) 
    {
        perror("[-]Error in opening file");
        exit(1);
    }
    size_t total_bytes = 0;
    /* Receive the PDF file and write it to disk */
    while ((ret = recv(new_socket, buffer, BUFSIZE, 0)) > 0) 
    {
        fwrite(buffer, 1, ret, pdf_file);
        total_bytes += ret;
    }
    handlePut(new_socket, total_bytes, extension);
    close(new_socket);
    fclose(pdf_file);
    return 0;
}
