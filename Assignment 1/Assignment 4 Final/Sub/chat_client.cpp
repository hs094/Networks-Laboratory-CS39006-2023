// #include <stdio.h> 
// #include <sys/socket.h> 
// #include <arpa/inet.h> 
// #include <unistd.h> 
// #include <string.h> 

// int main(int argc, char const *argv[]) 
// { 
//     int sock = 0, valread; 
//     struct sockaddr_in serv_addr; 
//     char buffer[1024] = {0}; 
//     if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
//     { 
//         printf("\n Socket creation error \n"); 
//         return -1; 
//     } 
   
//     serv_addr.sin_family = AF_INET; 
//     serv_addr.sin_port = htons(80); 
       
//     // Convert IPv4 and IPv6 addresses from text to binary form 
//     if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
//     { 
//         printf("\nInvalid address/ Address not supported \n"); 
//         return -1; 
//     } 
   
//     if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
//     { 
//         printf("\nConnection Failed \n"); 
//         return -1; 
//     } 

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
	int			sockfd , valread;
	struct sockaddr_in	serv_addr;

	int i;
	char buffer[1024];

	/* Opening a socket is exactly similar to the server process */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Unable to create socket\n");
		exit(0);
	}

	/* Recall that we specified INADDR_ANY when we specified the server
	   address in the server. Since the client can run on a different
	   machine, we must specify the IP address of the server. 

	   In this program, we assume that the server is running on the
	   same machine as the client. 127.0.0.1 is a special address
	   for "localhost" (this machine)
	   
	/* IF YOUR SERVER RUNS ON SOME OTHER MACHINE, YOU MUST CHANGE 
           THE IP ADDRESS SPECIFIED BELOW TO THE IP ADDRESS OF THE 
           MACHINE WHERE YOU ARE RUNNING THE SERVER. 
    	*/

	serv_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port	= htons(20000);

	/* With the information specified in serv_addr, the connect()
	   system call establishes a connection with the server process.
	*/
	if ((connect(sockfd, (struct sockaddr *) &serv_addr,
						sizeof(serv_addr))) < 0) {
		perror("Unable to connect to server\n");
		exit(0);
	}

	/* After connection, the client can send or receive messages.
	   However, please note that recv() will block when the
	   server is not sending and vice versa. Similarly send() will
	   block when the server is not receiving and vice versa. For
	   non-blocking modes, refer to the online man pages.
	*/
	for(i=0; i < 1024; i++) buffer[i] = '\0';
	recv(sockfd, buffer, 1024, 0);
	printf("%s\n", buffer);
    char *request = "GET / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: sample_client\r\n\r\n";
    send(sockfd , request , strlen(request) , 0 ); 
    valread = read( sockfd , buffer, 1024); 
    printf("%s\n",buffer ); 
    close(sockfd);
    return 0; 
} 