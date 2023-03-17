#ifndef __MSOCKET_H
#define __MSOCKET_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SOCK_MyTCP 15
#define T 1
#define P 0.05
#define TIMEOUT (2 * T)
#define MAX_QUEUE_SIZE 10
#define MAX_ONE 1000
#define MAX_MSG 5000


extern int tot_transm;

int my_socket(int domain, int type, int protocol);
int my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);;
int my_listen(int sockfd, int k);
int my_connect(int sockfd, struct sockaddr *serv_addr, socklen_t serv_len);
int my_accept(int sockfd, struct sockaddr *cli_addr, socklen_t *cli_addrlen);
ssize_t my_send(int Sockfd,  char *buf, size_t len, int flags);
ssize_t my_recv(int sockfd, char *buf, size_t len, int flags);
int my_close(int fd);
int dropMessage(float p);

#endif