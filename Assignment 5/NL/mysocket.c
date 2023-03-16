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
#define MAX_QUEUE_SIZE 10

int is_connect;
int my_sockfd;

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

typedef struct 
{
    char *messages[MAX_QUEUE_SIZE];
    int in;
    int out;
    int size;
} Queue;

/**
 * @brief Create a Queue object
 *
 * @return Queue*
 */
Queue *createQueue()
{
    Queue *queue = (Queue *)malloc(sizeof(Queue));
    queue->in = 0;
    queue->out = 0;
    queue->size = 0;
    return queue;
}

/**
 * @brief
 *
 * @param queue
 */
void destroyQueue(Queue *queue)
{
    free(queue);
}

/**
 * @brief
 *
 * @param queue
 * @return int
 */
int isEmpty(Queue *queue)
{
    return (queue->size == 0);
}

/**
 * @brief
 *
 * @param queue
 * @return int
 */
int isFull(Queue *queue)
{
    return (queue->size == MAX_QUEUE_SIZE);
}

/**
 * @brief
 *
 * @param queue
 * @param item
 */
void enqueue(Queue *queue, char *item)
{
    if (isFull(queue))
    {
        printf("Error: Queue is full\n");
        exit(1);
    }
    queue->messages[queue->out] = (char *)malloc(strlen(item) + 1);
    strcpy(queue->messages[queue->out], item);
    queue->size++;
    queue->out = (queue->out + 1) % 10;
}

/**
 * @brief
 *
 * @param queue
 * @return char*
 */
char *dequeue(Queue *queue)
{
    if (isEmpty(queue))
    {
        printf("Error: Queue is empty\n");
        exit(1);
    }
    char *item = queue->messages[queue->in];
    queue->in = (queue->in + 1) % 10;
    queue->size--;
    return item;
}

Queue *Send_Message, *Received_Message;

pthread_t tid_R, tid_S;
pthread_mutex_t Recv_Lock, Send_Lock;
pthread_cond_t Recv_Cond, Send_Cond;

/**
 * @brief
 *
 * @param arg
 * @return void*
 */
void *recv_thread(void *arg)
{
    while (1)
    {
        if (is_connect == 0)
            continue;
        int sockfd = my_sockfd;
        char buf[1000];
        char message[5000];
        int mess_bytes = 0;
        char buffer[4];
        while (4 - mess_bytes > 0)
        {
            int rec_bytes = recv(sockfd, buffer + mess_bytes, 4 - mess_bytes, 0);
            mess_bytes += rec_bytes;
            if (rec_bytes == 0)
            {
                is_connect = 0;
                break;
            }
        }
        int mess_size = 0;
        for (int i = 3; i >= 0; i--)
            mess_size = mess_size * 10 + buffer[i] - '0';
        printf("Size ecieved  : %d \n", mess_size);
        int total_rec_bytes = 0;
        while (mess_size - total_rec_bytes > 0)
        {
            int rec_bytes = recv(sockfd, message + total_rec_bytes, mess_size - total_rec_bytes, 0);
            if (rec_bytes == 0)
            {
                is_connect = 0;
                break;
            }
            total_rec_bytes += rec_bytes;
        }

        printf("Message Received : %s Queue Size : %d \n", message, Received_Message->out + 1);

        // Critical Section Starts
        LOCK(&Recv_Lock);
        while (isFull(Received_Message))
            pthread_cond_wait(&Recv_Cond, &Recv_Lock);
        enqueue(Received_Message, message);
        printf("queue size : %d\n", Received_Message->size);
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
void *send_thread(void *arg)
{
    while (1)
    {
        if (is_connect == 0)
            continue;
        int sockfd = my_sockfd;
        char buf[1000];
        char message[5000];
        // Critical Section Starts
        LOCK(&Send_Lock);
        while (isEmpty(Send_Message))
            pthread_cond_wait(&Send_Cond, &Send_Lock);
        strcpy(message, dequeue(Send_Message));
        pthread_cond_signal(&Send_Cond);
        UNLOCK(&Send_Lock);
        // Critical Section Ends
        int mess_size = strlen(message);
        if (mess_size > 5000)
            mess_size = 5000;
        int n = mess_size;
        char size_buff[4];
        for (int i = 0; i < 4; i++)
        {
            size_buff[i] = '0' + n % 10;
            n = n / 10;
        }
        send(sockfd, size_buff, 4, 0);
        printf("Size sent : %d \n", mess_size);
        int sent_len = 0;
        while (sent_len < mess_size)
        {
            int send_size = (mess_size - sent_len > 1000) ? 1000 : mess_size - sent_len;
            sent_len += send(sockfd, message + sent_len, send_size, 0);
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

    my_sockfd = sockfd;
    char sockfd_arr[10];

    sprintf(sockfd_arr, "%d", sockfd);
    // cond inits
    pthread_cond_init(&Recv_Cond, NULL);
    pthread_cond_init(&Send_Cond, NULL);

    // mutex inits
    pthread_mutex_init(&Recv_Lock, NULL);
    pthread_mutex_init(&Send_Lock, NULL);

    Send_Message = createQueue();
    Received_Message = createQueue();
    pthread_create(&tid_R, NULL, recv_thread, (void *)sockfd_arr);
    pthread_create(&tid_S, NULL, send_thread, (void *)sockfd_arr);
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
    my_sockfd = accept(sockfd, cli_addr, cli_addrlen);
    is_connect = 1 ;
    return  my_sockfd ;
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
    is_connect = 1;
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
    while (isFull(Send_Message))
        pthread_cond_wait(&Send_Cond, &Send_Lock);
    enqueue(Send_Message, buf);
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
    while (isEmpty(Received_Message))
        pthread_cond_wait(&Recv_Cond, &Recv_Lock);
    strcpy(buf, dequeue(Received_Message));
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
    destroyQueue(Send_Message);
    destroyQueue(Received_Message);
    close(Sockfd);
}
