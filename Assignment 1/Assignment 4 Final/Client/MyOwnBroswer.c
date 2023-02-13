// gcc -Wall -o cli MyOwnBroswer.c -L/usr/lib -lssl -lcrypto
/*
	Client*
	MyOwnBrowser
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#define PORT 8080
#define BUFSIZE 1024

void handleGETRequest(char message[], char filename[], int socket_desc)
{

	char receive_msg[100010] = {0};

	if (send(socket_desc, message, strlen(message), 0) < 0)
	{
		puts("Send failed");
		exit(1);
	}

	remove(filename);
	FILE *f = fopen(filename, "ab");
	if (f == NULL)
	{
		printf("Error while opening file.\n");
		exit(1);
	}

	int count = 0;
	if ((count = recv(socket_desc, receive_msg, 100000, 0)) > 0)
	{
		if (receive_msg[9] != '2' || receive_msg[10] != '0' || receive_msg[11] != '0')
		{
			printf("Didn't get a 200 OK response. Following is header received. Exiting ...\n\n");
			printf("%s", receive_msg);
			remove(filename);
			exit(1);
		}

		int i = 4;
		while (receive_msg[i - 4] != '\r' || receive_msg[i - 3] != '\n' || receive_msg[i - 2] != '\r' || receive_msg[i - 1] != '\n')
			i++;

		fwrite(receive_msg + i, count - i, 1, f);
		receive_msg[i] = '\0';
		printf("HTTP response header:\n\n%s", receive_msg);
	}

	while ((count = recv(socket_desc, receive_msg, 100000, 0)) > 0)
	{
		fwrite(receive_msg, count, 1, f);
	}

	printf("Reply received.\n");
	char *name, *extension;
	name = strtok(filename, ".");
	extension = strtok(NULL, ".");
	printf("\n\nFile's Information:\n");
	printf("Name: %s\n", name);
	printf("Extension: %s\n", extension);
	// if ()
	fclose(f);
}

void handleGETRequestSecured(char message[], char filename[], int socket_desc)
{
	printf("2\n");
	SSL *ssl;
	SSL_CTX *ctx;

	char receive_msg[100010] = {0};

	SSL_load_error_strings();
	SSL_library_init();
	OpenSSL_add_all_algorithms();

	ctx = SSL_CTX_new(SSLv23_client_method());
	if (ctx == NULL)
	{
		printf("CTX is null.\n");
		exit(1);
	}
	ssl = SSL_new(ctx);
	if (!ssl)
	{
		printf("Error creating SSL.\n");
		exit(1);
	}

	SSL_set_fd(ssl, socket_desc);

	if (SSL_connect(ssl) <= 0)
	{
		printf("Error while creating SSL connection.\n");
		exit(1);
	}
	printf("SSL connected.\n");

	if (SSL_write(ssl, message, strlen(message)) <= 0)
	{
		puts("Send failed");
		exit(1);
	}

	remove(filename);
	FILE *f = fopen(filename, "ab");
	if (f == NULL)
	{
		printf("Error while opening file.\n");
		exit(1);
	}

	int count = 0;
	if ((count = SSL_read(ssl, receive_msg, 100000)) > 0)
	{
		if (receive_msg[9] != '2' || receive_msg[10] != '0' || receive_msg[11] != '0')
		{
			printf("Didn't get a 200 OK response. Following is header received. Exiting ...\n\n");
			printf("%s", receive_msg);
			remove(filename);
			exit(1);
		}
		int i = 4;
		while (receive_msg[i - 4] != '\r' || receive_msg[i - 3] != '\n' || receive_msg[i - 2] != '\r' || receive_msg[i - 1] != '\n')
			i++;

		fwrite(receive_msg + i, count - i, 1, f);
		receive_msg[i] = '\0';
		printf("HTTP response header:\n\n%s", receive_msg);
	}

	while ((count = SSL_read(ssl, receive_msg, 100000)) > 0)
	{
		fwrite(receive_msg, count, 1, f);
	}

	printf("Reply received.\n");
	fclose(f);
	SSL_CTX_free(ctx);
}

// gcc -Wall -o client client.c -L/usr/lib -lssl -lcrypto

int main(void)
{
	char *prompt = "MyOwnBroswer> ";
	size_t sz = 1000;
	char *ans = (char *)malloc(sz * sizeof(char));
	while (1)
	{
		fprintf(stdout, prompt);
		memset(ans, '\0', sz);
		getline(&ans, &sz, stdin);
		ans[strlen(ans) - 1] = '\0';
		int cnt = 0;
		for (int i = 0; i < strlen(ans); i++)
			if (ans[i] == ' ')
				cnt++;
		// printf("\n[%s]\n", ans);
		char *request = strtok(ans, " ");
		if (cnt == 1 && (strcmp(request, "GET") == 0 || strcmp(request, "get") == 0))
		{
			// fprintf(stdout,"1\n");
			char *urlname = strtok(NULL, " ");
			int count = 0,
				i = (urlname[4] == 's' ? 8 : 7);
			for (; urlname[i + count] != '/'; count++)
				;
			char host[count + 1], path[strlen(urlname) - i - count + 10];
			for (int j = 0; j < count; j++)
				host[j] = urlname[i + j];
			host[count] = '\0';
			i += count, count = 0;
			for (int j = 0; urlname[i + count]; count++, j++)
				path[j] = urlname[i + count];

			path[count] = '\0';

			count = 0;

			i = strlen(urlname) - 1;

			for (; urlname[i - count] != '/'; count++)
				;

			char filename[count + 1];

			for (int j = 1; j <= count; j++)
				filename[j - 1] = urlname[i - count + j];
			filename[count] = '\0';

			char message[4096];

			sprintf(message, "GET %s%s%s%s", path, " HTTP/1.1\r\nHost: ", host, "\r\nConnection: Close\r\n\r\n");

			struct sockaddr_in server = {0};

			int socket_desc, port = (urlname[4] == 's' ? 443 : 80);

			if ((socket_desc = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			{
				printf("Socket can't be created");
				return 1;
			}

			struct hostent *hostent = gethostbyname(host);
			if (hostent == NULL)
			{
				printf("No such url exist. No such host. Exiting...!\n");
				return 1;
			}
			memcpy(&server.sin_addr.s_addr, hostent->h_addr, hostent->h_length);
			server.sin_family = AF_INET;
			server.sin_port = htons(port);
			if (connect(socket_desc, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) < 0)
			{
				printf("Server is unreachable! Exiting...\n");
				return 1;
			}
			printf("Sucessfully conected with server\n");
			if(urlname[4]=='s')
			{
				handleGETRequestSecured(message, filename, socket_desc);
			}
			else
			{
				handleGETRequest(message, filename, socket_desc);
			}
			printf("Closing the connection...\n");
			close(socket_desc);
			shutdown(socket_desc, 0);
			shutdown(socket_desc, 1);
			shutdown(socket_desc, 2);
			continue;
		}
		else if (cnt == 2 && (strcmp(request, "PUT") == 0 || strcmp(request, "put") == 0))
		{
			// fprintf(stdout,"2\n");
			size_t sz1 = 100;
			char *urlname = (char *)malloc(sz1 * sizeof(char));
			urlname = strtok(NULL, " ");
			char *filename = (char *)malloc(sz1 * sizeof(char));
			filename = strtok(NULL, " ");
			int count = 0,
				i = (urlname[4] == 's' ? 8 : 7);
			for (; urlname[i + count] != '/'; count++)
				;
			char host[count + 1], path[strlen(urlname) - i - count + 10];
			for (int j = 0; j < count; j++)
				host[j] = urlname[i + j];
			host[count] = '\0';
			i += count, count = 0;
			for (int j = 0; urlname[i + count]; count++, j++)
				path[j] = urlname[i + count];
			path[count] = '\0';
			int sockfd, ret;
			struct sockaddr_in server_addr;
			char buffer[BUFSIZE];
			FILE *pdf_file;
			sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if (sockfd < 0)
			{
				perror("[-]Error in socket creation");
				exit(1);
			}
			printf("[+]Client Socket is created.\n");

			struct hostent *hostent = gethostbyname(host);
			if (hostent == NULL)
			{
				printf("No such url exist. No such host. Exiting...!\n");
				return 1;
			}
			// printf("[%s]\n", hostent->h_a)
			memcpy(&server_addr.sin_addr.s_addr, hostent->h_addr, hostent->h_length);

			// memset(&server_addr, 0, sizeof(server_addr));

			server_addr.sin_family = AF_INET;
			server_addr.sin_port = htons(PORT);
			// server_addr.sin_addr.s_addr = inet_addr(host);

			ret = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
			if (ret < 0)
			{
				perror("[-]Error in connect");
				exit(1);
			}
			printf("[+]Connected to the server.\n");
			
			FILE* fp = fopen(filename, "rb");
			fseek(fp, 0L, SEEK_END);
			// int total_bytes = 0;
			int total_bytes = ftell(fp);
			close(fp);
			pdf_file = fopen(filename, "rb");
			if (pdf_file == NULL)
			{
				perror("[-]Error in opening file");
				exit(1);
			}
			char* recv1[300];
			sprintf(recv1, "PUT %s HTTP/%s\r\nHost: %s\r\nConnection: close\r\nAccept-Language: en-us\r\nContent-language: en-us\r\nContent-length: %d\r\nContent-type: %s\r\n", filename, "1.1", host, total_bytes, "plain/text");
			// printf("[%s]\n", recv1);
			send(sockfd, recv1, 300, 0);
			
			// send(sockfd, filename, 100, 0);
			while ((ret = fread(buffer, 1, BUFSIZE, pdf_file)) > 0)
				send(sockfd, buffer, ret, 0);
			
			fprintf(stdout, "PUT's Response:\n");
			char buf[1000];
			memset(buf, '\0', 1000);
			recv(sockfd, buf, 1000, 0);
			fprintf(stdout, "%s", buf);
			close(sockfd);
			fclose(pdf_file);
			continue;
		}
		else if (cnt == 0 && (strcmp(request, "QUIT") == 0 || strcmp(request, "quit") == 0))
		{
			break;
		}
		else
		{
			fprintf(stdout, "INVALID CHOICE\n");
			continue;
		}
	}

	return 0;
}
