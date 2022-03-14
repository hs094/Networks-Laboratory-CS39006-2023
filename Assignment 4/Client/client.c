#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#define PORT 8080
#define SERVER_ADDRESS "127.0.0.1"

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    int client_socket, read_size;
    struct sockaddr_in server;
    char *image_file = "image.jpg";
    char buffer[1024];

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        error("Could not create socket");
    }
    puts("Socket created");

    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);

    // Connect to server
    if (connect(client_socket, (struct sockaddr *) &server, sizeof(server)) < 0) {
        error("Connection failed. Error");
    }
    puts("Connected\n");

    // Open the image file
    int image = open(image_file, O_RDONLY);
    if (image < 0) {
        error("Failed to open image file");
    }
    puts("Image file opened\n");

    // Read the image file into the buffer
    int bytes_read = read(image, buffer, sizeof(buffer));
    if (bytes_read < 0) {
        error("Failed to read from image file");
    }
    puts("Image file read into buffer\n");

    // Send the buffer to the server
    int bytes_sent = send(client_socket, buffer, bytes_read, 0);
    if (bytes_sent < 0) {
        error("Failed to send buffer to server");
    }
    printf("Sent %d bytes to server\n", bytes_sent);

    // Close the image file and socket
    close(image);
    close(client_socket);

    return 0;
}

