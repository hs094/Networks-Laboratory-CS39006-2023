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
#include <poll.h>
#include <stdint.h>
#include <time.h>

void time_server(int sockfd, int newsockfd, char *buf, struct sockaddr_in addr, int ch)
{
    printf("@[S%d] Sending client request to <%s>\n", ch, inet_ntoa(addr.sin_addr));
    // Sending client request to <server IP>
    memset(buf, '\0', 100);
    strcpy(buf, "Send Time");
    send(sockfd, buf, strlen(buf) + 1, 0);
    memset(buf, '\0', 100);
    recv(sockfd, buf, 100, 0);
    close(sockfd);
    send(newsockfd, buf, strlen(buf) + 1, 0);
}

int64_t timespecDiff(struct timespec *timeA_p, struct timespec *timeB_p)
{
  return ((((timeA_p->tv_sec * 1000000000) + timeA_p->tv_nsec) -
           ((timeB_p->tv_sec * 1000000000) + timeB_p->tv_nsec))/(long double)1e6);
}

int main(int argc, char *argv[])
{
    int sockfd, sockfd_for_s1, sockfd_for_s2, sockfd_for_client;
    int clilen, load_1, load_2;
    struct sockaddr_in load_b_addr, client_addr, serv_addr_for_s1, serv_addr_for_s2;
    char *buf1 = malloc(100);
    char *buf2 = malloc(100);
    int timeout_time = 5000;
    struct timespec start, end;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Cannot create socket\n");
        exit(0);
    }

    // Client side communication
    load_b_addr.sin_family = AF_INET;
    load_b_addr.sin_addr.s_addr = INADDR_ANY;
    load_b_addr.sin_port = htons(atoi(argv[1]));

    // server side communication for 1
    serv_addr_for_s1.sin_family = AF_INET;
    inet_aton("127.0.0.1", &serv_addr_for_s1.sin_addr);
    serv_addr_for_s1.sin_port = htons(atoi(argv[2]));

    // server side communication for 2
    serv_addr_for_s2.sin_family = AF_INET;
    inet_aton("127.0.0.1", &serv_addr_for_s2.sin_addr);
    serv_addr_for_s2.sin_port = htons(atoi(argv[3]));

    if (bind(sockfd, (struct sockaddr *)&load_b_addr, sizeof(load_b_addr)) < 0) {
        perror("Unable to bind local address\n");
        exit(0);
    }

    listen(sockfd, 5);

    while (1) {
        if ((sockfd_for_s1 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Cannot create socket\n");
            exit(0);
        }
        if ((sockfd_for_s2 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Cannot create socket\n");
            exit(0);
        }
        if ((connect(sockfd_for_s1, (struct sockaddr *)&serv_addr_for_s1, sizeof(serv_addr_for_s1))) < 0) {
            perror("Unable to connect to server1\n");
            exit(0);
        }
        if ((connect(sockfd_for_s2, (struct sockaddr *)&serv_addr_for_s2, sizeof(serv_addr_for_s2))) < 0) {
            perror("Unable to connect to server2\n");
            exit(0);
        }

        struct pollfd fdset[1];
        fdset[0].fd = sockfd;
        fdset[0].events = POLLIN;
        clock_gettime(CLOCK_MONOTONIC, &start);
        int ret = poll(fdset, 1, timeout_time);

        if (ret == 0) {
            memset(buf1, '\0', 100);
            strcpy(buf1, "Send Load");
            send(sockfd_for_s1, buf1, strlen(buf1) + 1, 0);
            send(sockfd_for_s2, buf1, strlen(buf1) + 1, 0);
            memset(buf1, '\0', 100);
            recv(sockfd_for_s1, buf1, 100, 0);
            printf("- S1::: <%s> <%s>\n", inet_ntoa(serv_addr_for_s1.sin_addr),buf1);
            load_1 = atoi(buf1);
            recv(sockfd_for_s2, buf2, 100, 0);
            printf("- S2::: <%s> <%s>\n", inet_ntoa(serv_addr_for_s2.sin_addr),buf2);
            load_2 = atoi(buf2);
        }
        else if (ret > 0) {
            if (fork() == 0) {
                clilen = sizeof(client_addr);
                sockfd_for_client = accept(sockfd, (struct sockaddr *)&client_addr, &clilen);
                if (sockfd_for_client < 0) {
                    perror("Accept error\n");
                    exit(0);
                }
                else {
                    (load_1 <= load_2) ? time_server(sockfd_for_s1, sockfd_for_client, buf1, serv_addr_for_s1, 1) : time_server(sockfd_for_s2, sockfd_for_client, buf2, serv_addr_for_s2, 2);
                    close(sockfd_for_s1);
                    close(sockfd_for_s2);
                    clock_gettime(CLOCK_MONOTONIC, &end);
                    uint64_t timeElapsed = timespecDiff(&end, &start);
                    printf("Time Elapsed => Poll Timeout Time :- %lu ms\n\n", timeElapsed);
                    timeout_time = timeElapsed;
                    close(sockfd);
                }
                close(sockfd_for_client);
                exit(0);
            }
        }
        else {
            printf("POLL error, exiting...\n");
            exit(0);
        }
        close(sockfd_for_s1);
        close(sockfd_for_s2);
        close(sockfd_for_client);
    }
    return 0;
}