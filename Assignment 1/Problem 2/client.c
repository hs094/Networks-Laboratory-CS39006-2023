/*
	Name:- Hardik Soni
	Roll No:- 20CS30023
*/
/*    THE CLIENT PROCESS */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
int main()
{
	int sockfd;
	struct sockaddr_in serv_addr;

	int i;
	char *buf, *str;
	buf = (char *)malloc(100 * sizeof(char));
	str = (char *)malloc(100 * sizeof(char));

	/* Opening a socket is exactly similar to the server process */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Unable to create socket\n");
		exit(0);
	}

	serv_addr.sin_family = AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port = htons(20000);

	/* With the information specified in serv_addr, the connect()
	   system call establishes a connection with the server process.
	*/

	if ((connect(sockfd, (struct sockaddr *)&serv_addr,
				 sizeof(serv_addr))) < 0)
	{
		perror("Unable to connect to server\n");
		exit(0);
	}

	/* After connection, the client can send or receive messages.
	   However, please note that recv() will block when the
	   server is not sending and vice versa. Similarly send() will
	   block when the server is not receiving and vice versa. For
	   non-blocking modes, refer to the online man pages.
	*/
	int end = 0;
	int f = 0;
	while (1)
	{
		printf("Enter Expression:- ");
		int done = 0;
		int cnt = 0;
		int finish = 0;
		while (!done)
		{
			cnt++;
			for (int i = 0; i < 100; i++) str[i] = '\0';
			char c;
			for (int i = 0; i < 5; i++)
			{
				scanf("%c", &c);
				if(cnt==1 && i==0 && c=='-') {
					finish = 1;
					break;
				}
				if (c == '\n' || c == '\0')
				{
					done = 1;
					break;
				}
				str[i] = c;
			}
			if(finish) break;
			if(done) break;
			send(sockfd, str, 5, 0);
		}
		if(finish) break;
		int i = 0;
		for (; i < 5; i++)
		{
			if (str[i] == '\0')
				break;
		}
		if (i < 5)
		{
			str[i] = '\n';
			send(sockfd, str, 5, 0);
		}
		else
		{
			send(sockfd, "\n", 1, 0);
		}
		char ans[100] = {'\0'};
		recv(sockfd, ans, 100, 0);
		printf("Solution:- %s\n", ans);
		printf("\n");
	}
	close(sockfd);
	return 0;
}
