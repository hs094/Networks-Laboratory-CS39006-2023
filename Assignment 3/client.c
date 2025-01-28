/*
	Name:- Hardik Soni
	Roll No:- 20CS30023
	Networks Labortary
	Assignment 3
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

int main(int argc, char ** argv)
{
	int			sockfd ;
	struct sockaddr_in	serv_addr;

	int i;
	char *buf = malloc(100);

	/* Opening a socket is exactly similar to the server process */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Unable to create socket\n");
		exit(0);
	}
	serv_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port	= htons(atoi(argv[1]));
	if ((connect(sockfd, (struct sockaddr *) &serv_addr,
						sizeof(serv_addr))) < 0) {
		perror("Unable to connect to server\n");
		exit(0);
	}
	memset(buf,'\0',100);
	recv(sockfd, buf, 100, 0);   // Receiving Message from Server 
	close(sockfd);
	printf("Current Time is:- %s\n", buf); // Printing Time
	return 0;
}

