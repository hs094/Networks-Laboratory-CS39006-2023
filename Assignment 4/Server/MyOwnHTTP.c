#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFSIZE 1024

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

    /* Open the PDF file in binary mode */
    char filename[100];
    memset(filename, '\0', 100);
    recv(new_socket, filename, 100, 0);
    pdf_file = fopen(filename, "wb");
    if (pdf_file == NULL) 
    {
        perror("[-]Error in opening file");
        exit(1);
    }
    
    /* Receive the PDF file and write it to disk */
    while ((ret = recv(new_socket, buffer, BUFSIZE, 0)) > 0) {
        fwrite(buffer, 1, ret, pdf_file);
    }

    /* Close the socket and the file */
    close(new_socket);
    fclose(pdf_file);

    return 0;
}
