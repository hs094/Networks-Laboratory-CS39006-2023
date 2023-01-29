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

/* THE SERVER PROCESS */

int main(int argc, char **argv)
{
	int sockfd, newsockfd; /* Socket descriptors */
	int clilen;
	struct sockaddr_in cli_addr, serv_addr;

	int i;
	char buf[100]; /* We will use this buffer for communication */
	srand((unsigned long int)time(NULL));
	/* The following system call opens a socket. The first parameter
	   indicates the family of the protocol to be followed.
	*/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Cannot create socket\n");
		exit(0);
	}
	for (int i = 0; i < strlen(argv[1]); i++)
	{
		if (argv[1][i] >= '0' && argv[1][i] <= '9')
			continue;
		else
		{
			printf("INCORRECT PORT NUMBER\n");
			exit(0);
		}
	}
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(atoi(argv[1]));

	/* With the information provided in serv_addr, we associate the server
	   with its port using the bind() system call.
	*/
	if (bind(sockfd, (struct sockaddr *)&serv_addr,
			 sizeof(serv_addr)) < 0)
	{
		perror("Unable to bind local address\n");
		exit(0);
	}
	printf("Server Started on Port Number:- %s\n", argv[1]);
	listen(sockfd, 5); /* Specifies that up to 5 concurrent client requests will be queued up while the system is executing the "accept" system call below.*/
	while (1)
	{
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr,
						   &clilen);
		if (newsockfd < 0)
		{
			perror("Accept error\n");
			exit(0);
		}
		/* We initialize the buffer, copy the message to it,
			and send the message to the client.
		*/
		memset(buf, '\0', 100);
		recv(newsockfd, buf, 100, 0);
		if (strcmp(buf, "Send Load") == 0)
		{
			unsigned int load = (rand() % 100 + 1);
			printf("Load sent: <%d>\n", load);
			memset(buf, '\0', 100);
			sprintf(buf, "%d", load);
			send(newsockfd, buf, strlen(buf) + 1, 0);
		}
		else if (strcmp(buf, "Send Time") == 0)
		{
			printf("%s\n", buf);
			memset(buf, '\0', 100);
			time_t t;
			time(&t);
			strcpy(buf, ctime(&t));
			send(newsockfd, buf, strlen(buf) + 1, 0); // Sending Message to Client
		}
		else
		{
			printf("SERVER INACTIVE!!\n");
			continue;
		}
		close(newsockfd); // Closing the Socket
	}
	return 0;
}