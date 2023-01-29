/*
	Name:- Hardik Soni
	Roll No:- 20CS30023
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

/* THE SERVER PROCESS */

int main()
{
	int			sockfd, newsockfd ; /* Socket descriptors */
	int			clilen;
	struct sockaddr_in	cli_addr, serv_addr;

	int i;
	char buf[100];		/* We will use this buffer for communication */

	/* The following system call opens a socket. The first parameter
	   indicates the family of the protocol to be followed.
	*/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Cannot create socket\n");
		exit(0);
	}

	/* This has three main fields. The
 	   field "sin_family" specifies the family and is therefore AF_INET
	   for the internet family.
	*/
	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(20000);

	/* With the information provided in serv_addr, we associate the server
	   with its port using the bind() system call. 
	*/
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		perror("Unable to bind local address\n");
		exit(0);
	}
	listen(sockfd, 5); /* Specifies that up to 5 concurrent client requests will be queued up while the system is executing the "accept" system call below.*/
	while (1) {
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,
					&clilen) ;
		if (newsockfd < 0) {
			perror("Accept error\n");
			exit(0);
		}
		/* We initialize the buffer, copy the message to it,
			and send the message to the client. 
		*/
		time_t t;
		time(&t);
		strcpy(buf,ctime(&t));
		send(newsockfd, buf, strlen(buf) + 1, 0);  // Sending Message to Client
		recv(newsockfd, buf, 100, 0);  
		close(newsockfd);  // Closing the Socket
	}
	return 0;
}
