#include "mysocket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT_2 50017
#define MAX_MSG_LEN 1000
// Server
int main()
{
    int sockfd, newsockfd;
    struct sockaddr_in cli_addr, serv_addr;
    socklen_t clilen;
    char msg[MAX_MSG_LEN];
    if ((sockfd = my_socket(AF_INET, SOCK_MyTCP, 0)) < 0)
    {
        perror("my_socket");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_2);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (my_bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Unable to bind local address\n");
        exit(0);
    }
    my_listen(sockfd, 5);

    while (1)
    {
        clilen = sizeof(cli_addr);
        newsockfd = my_accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        clilen = sizeof(cli_addr);
		if (newsockfd < 0) {
			perror("Accept error\n");
			exit(0);
		}
		/* We initialize the msgfer, copy the message to it,
			and send the message to the client. 
		*/
		strcpy(msg,"Message from server");
		// my_send(newsockfd, msg, strlen(msg) + 1, 0);
        // my_recv(newsockfd, msg, 100, 0);
		printf("%s\n", msg);
		my_close(newsockfd);
    }
    my_close(sockfd);
    return 0;
}
