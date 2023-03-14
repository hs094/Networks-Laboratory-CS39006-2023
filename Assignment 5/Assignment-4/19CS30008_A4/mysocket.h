#ifndef __MSOCKET_H
#define __MSOCKET_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SOCK_MyTCP 15
#define T 2
#define P 0.05
#define TIMEOUT (2 * T)

extern int tot_transm;

int my_socket(int domain, int type, int protocol);
int my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int my_listen(int sockfd, int k);
int my_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int my_accept(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
ssize_t my_send(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t my_recv(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
int my_close(int fd);
int dropMessage(float p);

#endif
