/**
 * @file mysocket.c
 * Hardik Soni 20CS30023
 * Archit Mangrulkar
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

int connectionStatus;
int mySocketfd;

// Wrapper around pthread_mutex_lock for error checking
void LOCK(pthread_mutex_t *mutex)
{
    int ret = pthread_mutex_lock(mutex);
    if (ret != 0)
    {
        ERROR("pthread_mutex_lock failed: %s", strerror(ret));
        exit(1);
    }
}

// Wrapper around pthread_mutex_unlock for error checking
void UNLOCK(pthread_mutex_t *mutex)
{
    int ret = pthread_mutex_unlock(mutex);
    if (ret != 0)
    {
        ERROR("pthread_mutex_unlock failed: %s", strerror(ret));
        exit(1);
    }
}

typedef struct _Received_Message
{
    char *messages[MAX_QUEUE_SIZE];
    int in;
    int out;
    int size;
} Received_Message;

/**
 * @brief Create a Received_Message object
 *
 * @return Received_Message*
 */
Received_Message *create_Received_Message()
{
    Received_Message *Received_Message_Table = (Received_Message *)malloc(1*sizeof(Received_Message));
    Received_Message_Table->in = 0;
    Received_Message_Table->out = 0;
    Received_Message_Table->size = 0;
    return Received_Message_Table;
}

/**
 * @brief
 *
 * @param Received_Message
 */
void destroy_Received_Message(Received_Message *Received_Message)
{
    free(Received_Message);
}

/**
 * @brief
 *
 * @param Received_Message
 * @return int
 */
int is_Empty_Recv(Received_Message *Received_Message)
{
    return (Received_Message->size == 0);
}

/**
 * @brief
 *
 * @param Received_Message
 * @return int
 */
int is_Full_Recv(Received_Message *Received_Message)
{
    return (Received_Message->size == MAX_QUEUE_SIZE);
}

/**
 * @brief
 *
 * @param Received_Message
 * @param item
 */
void add_Received_Message(Received_Message *Received_Message, char *item)
{
    if (is_Full_Recv(Received_Message))
    {
        printf("Error: Received_Message is full\n");
        exit(1);
    }
    Received_Message->messages[Received_Message->out] = (char *)malloc(strlen(item) + 1);
    strcpy(Received_Message->messages[Received_Message->out], item);
    Received_Message->size++;
    Received_Message->out = (Received_Message->out + 1) % 10;
}

/**
 * @brief
 *
 * @param Received_Message
 * @return char*
 */
char *remove_Received_Message(Received_Message *Received_Message)
{
    if (is_Empty_Recv(Received_Message))
    {
        printf("Error: Received_Message is empty\n");
        exit(1);
    }
    char *item = Received_Message->messages[Received_Message->in];
    Received_Message->in = (Received_Message->in + 1) % 10;
    Received_Message->size--;
    return item;
}


typedef struct _Send_Message
{
    char *messages[MAX_QUEUE_SIZE];
    int in;
    int out;
    int size;
} Send_Message;

/**
 * @brief Create a Send_Message object
 *
 * @return Send_Message*
 */
Send_Message *create_Send_Message()
{
    Send_Message *Send_Message_Table = (Send_Message *)malloc(sizeof(Send_Message));
    Send_Message_Table->in = 0;
    Send_Message_Table->out = 0;
    Send_Message_Table->size = 0;
    return Send_Message_Table;
}

/**
 * @brief
 *
 * @param Send_Message
 */
void destroy_Send_Message(Send_Message *Send_Message)
{
    free(Send_Message);
}

/**
 * @brief
 *
 * @param Send_Message
 * @return int
 */
int is_Empty_Send(Send_Message *Send_Message)
{
    return (Send_Message->size == 0);
}

/**
 * @brief
 *
 * @param Send_Message
 * @return int
 */
int is_Full_Send(Send_Message *Send_Message)
{
    return (Send_Message->size == MAX_QUEUE_SIZE);
}

/**
 * @brief
 *
 * @param Send_Message
 * @param item
 */
void add_Send_Message(Send_Message *Send_Message, char *item)
{
    if (is_Full_Send(Send_Message))
    {
        printf("Error: Send_Message is full\n");
        exit(1);
    }
    Send_Message->messages[Send_Message->out] = (char *)malloc(strlen(item) + 1);
    strcpy(Send_Message->messages[Send_Message->out], item);
    Send_Message->size++;
    Send_Message->out = (Send_Message->out + 1) % 10;
}

/**
 * @brief
 *
 * @param Send_Message
 * @return char*
 */
char *remove_Send_Message(Send_Message *Send_Message)
{
    if (is_Empty_Send(Send_Message))
    {
        printf("Error: Send_Message is empty\n");
        exit(1);
    }
    char *item = Send_Message->messages[Send_Message->in];
    Send_Message->in = (Send_Message->in + 1) % 10;
    Send_Message->size--;
    return item;
}

Send_Message *Send_Message_Table;
Received_Message *Received_Message_Table;

pthread_t tid_R, tid_S;
pthread_mutex_t Recv_Lock, Send_Lock;
pthread_cond_t Recv_Cond, Send_Cond;

/**
 * @brief
 *
 * @param arg
 * @return void*
 */
void *recvThread(void *arg)
{
    while (1)
    {
        if (connectionStatus == 0)
            continue;
        int sockfd = mySocketfd;
        char message[MAX_MSG];
        int messageBytes = 0;
        char buffer[4];
        while (4 - messageBytes > 0)
        {
            int rec_bytes = recv(sockfd, buffer + messageBytes, 4 - messageBytes, 0);
            messageBytes += rec_bytes;
            if (rec_bytes == 0)
            {
                connectionStatus = 0;
                break;
            }
        }
        int messageSize = 0;
        for (int i = 3; i >= 0; i--)
            messageSize = messageSize * 10 + buffer[i] - '0';
        int totalRecvBytes = 0;
        while (messageSize - totalRecvBytes > 0)
        {
            int rec_bytes = recv(sockfd, message + totalRecvBytes, messageSize - totalRecvBytes, 0);
            if (rec_bytes == 0)
            {
                connectionStatus = 0;
                break;
            }
            totalRecvBytes += rec_bytes;
        }
        // Critical Section Starts
        LOCK(&Recv_Lock);
        while (is_Full_Recv(Received_Message_Table))
            pthread_cond_wait(&Recv_Cond, &Recv_Lock);
        add_Received_Message(Received_Message_Table, message);
        pthread_cond_signal(&Recv_Cond);
        UNLOCK(&Recv_Lock);
        // Critical Section Ends
    }
    pthread_exit(NULL);
}

/**
 * @brief
 *
 * @param arg
 * @return void*
 */
void *sendThread(void *arg)
{
    while (1)
    {
        sleep(T);
        if (connectionStatus == 0)
            continue;
        int sockfd = mySocketfd;
        char message[MAX_MSG];
        // Critical Section Starts
        LOCK(&Send_Lock);
        while (is_Empty_Send(Send_Message_Table))
            pthread_cond_wait(&Send_Cond, &Send_Lock);
        strcpy(message, remove_Send_Message(Send_Message_Table));
        pthread_cond_signal(&Send_Cond);
        UNLOCK(&Send_Lock);
        // Critical Section Ends
        int messageSize = strlen(message) > MAX_MSG ? MAX_MSG : strlen(message);
        int n = messageSize;
        char sizeBuffer[4];
        for (int i = 0; i < 4; i++)
        {
            sizeBuffer[i] = '0' + n % 10;
            n = n / 10;
        }
        send(sockfd, sizeBuffer, 4, 0);
        int sentLength = 0;
        while (sentLength < messageSize)
        {
            int sendSize = (messageSize - sentLength > MAX_ONE) ? MAX_ONE : messageSize - sentLength;
            sentLength += send(sockfd, message + sentLength, sendSize, 0);
        }
        
    }
    pthread_exit(NULL);
}

/**
 * @brief
 *
 * @param family
 * @param type
 * @param protocol
 * @return int
 */
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
    int *sockfd_arg = (int *)malloc(sizeof(int));
    *sockfd_arg = sockfd;

    mySocketfd = sockfd;
    char sockfd_arr[10];

    sprintf(sockfd_arr, "%d", sockfd);
    // cond inits
    pthread_cond_init(&Recv_Cond, NULL);
    pthread_cond_init(&Send_Cond, NULL);

    // mutex inits
    pthread_mutex_init(&Recv_Lock, NULL);
    pthread_mutex_init(&Send_Lock, NULL);

    Send_Message_Table = create_Send_Message();
    Received_Message_Table = create_Received_Message();
    pthread_create(&tid_R, NULL, recvThread, (void *)sockfd_arr);
    pthread_create(&tid_S, NULL, sendThread, (void *)sockfd_arr);
    return sockfd;
}

/**
 * @brief
 *
 * @param sockfd
 * @param addr
 * @param addrlen
 * @return int
 */

// Binds the socket with an address and a port
int my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    int ret = bind(sockfd, addr, addrlen);
    return ret;
}

/**
 * @brief
 *
 * @param Sockfd
 * @param clients
 * @return int
 */
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

/**
 * @brief
 *
 * @param Sockfd
 * @param addr
 * @param addrlen
 * @return int
 */
// Wrapper Around Accept Call
int my_accept(int sockfd, struct sockaddr *cli_addr, socklen_t *cli_addrlen)
{
    mySocketfd = accept(sockfd, cli_addr, cli_addrlen);
    connectionStatus = 1;
    return mySocketfd;
}
/**
 * @brief
 *
 * @param Sockfd
 * @param addr
 * @param addrlen
 * @return int
 */
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

/**
 * @brief
 * @param Sockfd
 * @param buf
 * @param len
 * @param flags
 * @return ssize_t
 */
// Send the Message
ssize_t my_send(int Sockfd, char *buf, size_t len, int flags)
{
    // Critical Section Starts
    LOCK(&Send_Lock);
    while (is_Full_Send(Send_Message_Table))
        pthread_cond_wait(&Send_Cond, &Send_Lock);
    add_Send_Message(Send_Message_Table, buf);
    pthread_cond_signal(&Send_Cond);
    UNLOCK(&Send_Lock);
    // Critical Section Ends
    return strlen(buf);
}

/**
 * @brief
 *
 * @param Sockfd
 * @param buf
 * @param len
 * @param flags
 * @return ssize_t
 */

// Checks the received-message table, if there is a message then returns that message and deletes it from the table
// If there is no message, sleeps for some time and checks again
ssize_t my_recv(int Sockfd, char *buf, size_t len, int flags)
{
    // Critical Section Starts
    LOCK(&Recv_Lock);
    while (is_Empty_Recv(Received_Message_Table))
        pthread_cond_wait(&Recv_Cond, &Recv_Lock);
    strcpy(buf, remove_Received_Message(Received_Message_Table));
    pthread_cond_signal(&Recv_Cond);
    UNLOCK(&Recv_Lock);
    // Critical Section Ends
    return strlen(buf);
}

/**
 * @brief
 *
 * @param Sockfd
 * @return int
 */
// Kills threads R and S, frees the tables, closes the socket
int my_close(int Sockfd)
{
    pthread_cancel(tid_R);
    pthread_cancel(tid_S);
    destroy_Send_Message(Send_Message_Table);
    destroy_Received_Message(Received_Message_Table);
    close(Sockfd);
}
