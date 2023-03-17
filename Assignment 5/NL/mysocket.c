/**
 * @file mysocket.c
 * Hardik Soni 20CS30023
 * Archit Mangrulkar 20CS10086
 */
#include "mysocket.h"

#include <arpa/inet.h>
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define ERROR(msg, ...) printf("\033[1;31m[ERROR] " msg " \033[0m\n", ##__VA_ARGS__);
#define SUCCESS(msg, ...) printf("\033[1;36m[SUCCESS] " msg " \033[0m\n", ##__VA_ARGS__);
#define INFO(msg, ...) printf("\033[1;32m[INFO] " msg " \033[0m\n", ##__VA_ARGS__);
#define PROMPT(msg, ...) printf("\033[1;32m" msg "\033[0m", ##__VA_ARGS__);
#define min(a, b) a > b ? b : a

int connectionStatus;
long int mySocketfd;

pthread_t tid_R, tid_S;
pthread_mutex_t Recv_Lock, Send_Lock;
pthread_cond_t Recv_Cond, Send_Cond;

// Wrapper around pthread_mutex_lock for error checking
void LOCK(pthread_mutex_t *mutex)
{
    long int ret = pthread_mutex_lock(mutex);
    if (ret != 0)
    {
        ERROR("pthread_mutex_lock failed: %s", strerror(ret));
        exit(1);
    }
}

// Wrapper around pthread_mutex_unlock for error checking
void UNLOCK(pthread_mutex_t *mutex)
{
    long int ret = pthread_mutex_unlock(mutex);
    if (ret != 0)
    {
        ERROR("pthread_mutex_unlock failed: %s", strerror(ret));
        exit(1);
    }
}
typedef struct _Received_Message
{
    char *messages[MAX_QUEUE_SIZE];
    long int in;
    long int out;
    long int size;
} Received_Message;

Received_Message *create_Received_Message()
{
    Received_Message *Received_Message_Table = (Received_Message *)malloc(1 * sizeof(Received_Message));
    Received_Message_Table->in = 0;
    Received_Message_Table->out = 0;
    Received_Message_Table->size = 0;
    return Received_Message_Table;
}
void destroy_Received_Message(Received_Message *Received_Message)
{
    free(Received_Message);
}
int is_Empty_Recv(Received_Message *Received_Message)
{
    if (Received_Message->size == 0)
        return 1;
    else
        return 0;
}
int is_Full_Recv(Received_Message *Received_Message)
{
    if (Received_Message->size == MAX_QUEUE_SIZE)
        return 1;
    else
        return 0;
}
void add_Received_Message(Received_Message *Received_Message, char *item)
{
    if (is_Full_Recv(Received_Message))
    {
        printf("Error: Received_Message is full\n");
        exit(1);
    }
    Received_Message->size++;
    Received_Message->messages[Received_Message->out] = (char *)malloc(strlen(item) + 1);
    strcpy(Received_Message->messages[Received_Message->out], item);
    Received_Message->out = (Received_Message->out + 1) % 10;
}

char *remove_Received_Message(Received_Message *Received_Message)
{
    if (is_Empty_Recv(Received_Message))
    {
        printf("Error: Received_Message is empty\n");
        exit(1);
    }
    Received_Message->size--;
    char *item = Received_Message->messages[Received_Message->in];
    Received_Message->in = (Received_Message->in + 1) % 10;
    return item;
}

typedef struct _Send_Message
{
    char *messages[MAX_QUEUE_SIZE];
    long int in;
    long int out;
    long int size;
} Send_Message;

Send_Message *create_Send_Message()
{
    Send_Message *Send_Message_Table = (Send_Message *)malloc(sizeof(Send_Message));
    Send_Message_Table->in = 0;
    Send_Message_Table->out = 0;
    Send_Message_Table->size = 0;
    return Send_Message_Table;
}

void destroy_Send_Message(Send_Message *Send_Message)
{
    free(Send_Message);
}
int is_Empty_Send(Send_Message *Send_Message)
{
    if (Send_Message->size == 0)
        return 1;
    else
        return 0;
}

int is_Full_Send(Send_Message *Send_Message)
{
    if ((Send_Message->size == MAX_QUEUE_SIZE))
        return 1;
    else
        return 0;
}

void add_Send_Message(Send_Message *Send_Message, char *item)
{
    if (is_Full_Send(Send_Message))
    {
        printf("Error: Send_Message is full\n");
        exit(1);
    }
    Send_Message->size++;
    Send_Message->messages[Send_Message->out] = (char *)malloc(strlen(item) + 1);
    strcpy(Send_Message->messages[Send_Message->out], item);
    Send_Message->out = (Send_Message->out + 1) % 10;
}

char *remove_Send_Message(Send_Message *Send_Message)
{
    if (is_Empty_Send(Send_Message))
    {
        printf("Error: Send_Message is empty\n");
        exit(1);
    }
    Send_Message->size--;
    char *item = Send_Message->messages[Send_Message->in];
    Send_Message->in = (Send_Message->in + 1) % 10;
    return item;
}

Send_Message *Send_Message_Table;
Received_Message *Received_Message_Table;


void *recvThread(void *arg)
{
    for (; 1;)
    {
        if (connectionStatus == 0)
            continue;
        long int messages = 0;
        long int Size = 1;
        int sockfd = mySocketfd;
        char message[MAX_MSG];
        long messageBytes = 0;
        long int messageSize = 0;
        long totalRecvBytes = 0;
        char buffer[4];
        for (; (4 - messageBytes)*Size+messages > 0;)
        {
            ssize_t rec_bytes = recv(sockfd, buffer + messageBytes, 4 - messageBytes, 0);
            messageBytes += rec_bytes;
            if (rec_bytes == 0)
            {
                connectionStatus = 0;
                break;
            }
        }
        for (int i = 3; i >= 0; i--)
            messageSize = messageSize * Size * 10 + (buffer[i] - '0') * Size + messages;
        for (; messageSize - totalRecvBytes > 0;)
        {
            ssize_t rec_bytes = recv(sockfd, message + totalRecvBytes * Size, messageSize - totalRecvBytes * Size + messages, 0);
            if (rec_bytes == 0)
            {
                connectionStatus = 0;
                break;
            }
            totalRecvBytes += rec_bytes * Size;
        }
        // Critical Section Starts
        LOCK(&Recv_Lock);
        for (; is_Full_Recv(Received_Message_Table);)
            pthread_cond_wait(&Recv_Cond, &Recv_Lock);
        add_Received_Message(Received_Message_Table, message);
        pthread_cond_signal(&Recv_Cond);
        UNLOCK(&Recv_Lock);
        // Critical Section Ends
    }
    pthread_exit(NULL);
}

void *sendThread(void *arg)
{
    for (; 1;)
    {
        sleep(1);
        if (connectionStatus == 0)
            continue;
        char message[MAX_MSG];
        // Critical Section Starts
        LOCK(&Send_Lock);
        for (; is_Empty_Send(Send_Message_Table);)
            pthread_cond_wait(&Send_Cond, &Send_Lock);
        strcpy(message, remove_Send_Message(Send_Message_Table));
        pthread_cond_signal(&Send_Cond);
        UNLOCK(&Send_Lock);
        // Critical Section Ends
        long int messages = 0;
        long int Size = 1;
        long int sockfd = mySocketfd;
        int messageSize = min(strlen(message), MAX_MSG);
        int n = messageSize;
        char sizeBuffer[4];
        for (long int i = 0; i < 4; i++)
        {
            sizeBuffer[i] = '0' + (n % 10) * Size + messages;
            n = n / (10 * Size);
        }
        send(sockfd, sizeBuffer, 4, 0);
        ssize_t sentLength = 0;
        for (; sentLength < messageSize;)
        {
            int sendSize = min(MAX_ONE + messages, (messageSize - sentLength) * Size);
            sentLength += send(sockfd, message + sentLength * Size + messages, sendSize, 0);
        }
    }
    pthread_exit(NULL);
}

// // Opens a TCP (SOCK_STREAM) socket, creates threads R and S, allocates space for the 2 tables
int my_socket(int domain, int type, int protocol)
{
    struct timeval seed;
    gettimeofday(&seed, NULL);
    srand(seed.tv_usec);

    if (type != SOCK_MyTCP)
    {
        ERROR("r_socket: type must be SOCK_MyTCP");
        return -1;
    }
    int sockfd = socket(domain, SOCK_STREAM, protocol);
    if (sockfd < 0)
    {
        perror("Cannot create socket\n");
        exit(0);
    }

    // cond inits
    pthread_cond_init(&Recv_Cond, NULL);
    pthread_cond_init(&Send_Cond, NULL);

    // mutex inits
    pthread_mutex_init(&Recv_Lock, NULL);
    pthread_mutex_init(&Send_Lock, NULL);

    long int *sockfd_arg = (long int *)malloc(sizeof(long int));
    *sockfd_arg = sockfd;

    mySocketfd = sockfd;
    char sockfd_arr[10];

    sprintf(sockfd_arr, "%d", sockfd);
    
    Send_Message_Table = create_Send_Message();
    Received_Message_Table = create_Received_Message();
    
    pthread_create(&tid_R, NULL, recvThread, (void *)sockfd_arr);
    pthread_create(&tid_S, NULL, sendThread, (void *)sockfd_arr);
    
    return sockfd;
}

// Binds the socket with an address and a port
int my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    int ret = bind(sockfd, addr, addrlen);
    return ret;
}

// Listen the socket with an address and a port and stores the message in the queue of size k
int my_listen(int sockfd, int k)
{
    int ret = 0;
    if ((ret = listen(sockfd, k)) == -1)
    {
        perror("Listen Failed\n");
        exit(1);
    }
    return ret;
}

// Wrapper Around Accept Call
int my_accept(int sockfd, struct sockaddr *cli_addr, socklen_t *cli_addrlen)
{
    mySocketfd = accept(sockfd, cli_addr, cli_addrlen);
    connectionStatus = 1;
    return mySocketfd;
}
// Wrapper Around Connect Call
int my_connect(int sockfd, struct sockaddr *serv_addr, socklen_t serv_len)
{
    int ret = connect(sockfd, serv_addr, serv_len);
    if (ret < 0)
    {
        perror("Unable to connect to server\n");
        exit(0);
    }
    connectionStatus = 1;
    return ret;
}

// Send the Message
ssize_t my_send(int sockfd, char *buf, size_t len, int flags)
{
    // Critical Section Starts
    LOCK(&Send_Lock);
    for (; is_Full_Send(Send_Message_Table);)
        pthread_cond_wait(&Send_Cond, &Send_Lock);
    add_Send_Message(Send_Message_Table, buf);
    pthread_cond_signal(&Send_Cond);
    UNLOCK(&Send_Lock);
    // Critical Section Ends
    return strlen(buf);
}

// Checks the received-message table, if there is a message then returns that message and deletes it from the table
// If there is no message, sleeps for some time and checks again
ssize_t my_recv(int sockfd, char *buf, size_t len, int flags)
{
    // Critical Section Starts
    LOCK(&Recv_Lock);
    for (; is_Empty_Recv(Received_Message_Table);)
        pthread_cond_wait(&Recv_Cond, &Recv_Lock);
    strcpy(buf, remove_Received_Message(Received_Message_Table));
    pthread_cond_signal(&Recv_Cond);
    UNLOCK(&Recv_Lock);
    // Critical Section Ends
    return strlen(buf);
}

// Kills threads R and S, frees the tables, closes the socket
int my_close(int sockfd)
{
    pthread_cancel(tid_R);
    pthread_cancel(tid_S);
    destroy_Send_Message(Send_Message_Table);
    destroy_Received_Message(Received_Message_Table);
    close(sockfd);
}