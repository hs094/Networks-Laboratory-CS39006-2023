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

#define ACK_TYPE 'A'
#define DATA_TYPE 'D'

#define TYPE_SIZE sizeof(char)
#define MSG_ID_SIZE sizeof(short)
#define MAX_MSG_SIZE 5000
#define MAX_FRAME_SIZE (TYPE_SIZE + MSG_ID_SIZE + MAX_MSG_SIZE)

#define MAX_TBL_SIZE 10

/*
Frame Format:Z
1 byte - char - type - D for data, A for ack
2 bytes - short - msg_id
arbitary no. of bytes (max. 100) - msg
*/
struct sockaddr *dest_addr;
socklen_t dest_addr_len;

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

short msg_cntr = 0;     // Message counter to keep track of next message id
int tot_transm = 0;     // Total number of transmissions
pthread_t tid_R, tid_S; // Thread identifiers for the threads R and S

typedef struct _Received_Message_entry
{
    short msg_id;
    char *msg;
    size_t msg_len;
    struct sockaddr src_addr;
    socklen_t addrlen;
} Received_Message_entry;

typedef struct _Received_Message
{
    Received_Message_entry *messages[MAX_TBL_SIZE];
    int in;
    int out;
    int count;
    pthread_mutex_t mutex;
} Received_Message;

Received_Message *recvd_msg_tbl; // Received message table

// Initializes the received message table
void init_Received_Message()
{
    recvd_msg_tbl = (Received_Message *)malloc(sizeof(Received_Message));
    for (int i = 0; i < MAX_TBL_SIZE; i++)
    {
        recvd_msg_tbl->messages[i] = NULL;
    }
    recvd_msg_tbl->in = 0;
    recvd_msg_tbl->out = 0;
    recvd_msg_tbl->count = 0;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK_NP);
    pthread_mutex_init(&recvd_msg_tbl->mutex, &attr);
}

// Adds a message to the back of received message table
void enqueue_Received_Message(char *buf, size_t len, struct sockaddr *src_addr,
                              socklen_t addrlen)
{
    recvd_msg_tbl->messages[recvd_msg_tbl->in] = (Received_Message_entry *)malloc(sizeof(Received_Message_entry));
    Received_Message_entry *in_entry = recvd_msg_tbl->messages[recvd_msg_tbl->in];
    in_entry->msg_len = len;
    in_entry->src_addr = *src_addr;
    in_entry->addrlen = addrlen;
    if (len > 0)
    {
        in_entry->msg_id = ntohs(*(short *)(buf + TYPE_SIZE));
        in_entry->msg = (char *)malloc(len * sizeof(char));
        memcpy(in_entry->msg, buf + TYPE_SIZE + MSG_ID_SIZE, len);
    }

    recvd_msg_tbl->in = (recvd_msg_tbl->in + 1) % MAX_TBL_SIZE;
    recvd_msg_tbl->count++;
}

// Removes a message from the front of received message table
size_t dequeue_Received_Message(void *buf, size_t len, struct sockaddr *src_addr,
                                socklen_t *addrlen)
{
    Received_Message_entry *out_entry = recvd_msg_tbl->messages[recvd_msg_tbl->out];
    size_t copy_len = len < out_entry->msg_len ? len : out_entry->msg_len;
    if (copy_len > 0)
        memcpy(buf, out_entry->msg, copy_len);
    *src_addr = out_entry->src_addr;
    *addrlen = out_entry->addrlen;
    if (copy_len > 0)
        free(out_entry->msg);
    free(out_entry);
    recvd_msg_tbl->messages[recvd_msg_tbl->out] = NULL;
    recvd_msg_tbl->out = (recvd_msg_tbl->out + 1) % MAX_TBL_SIZE;
    recvd_msg_tbl->count--;
    return copy_len;
}

// Frees the memory allocated to the received message table
void free_Received_Message()
{
    pthread_mutex_destroy(&recvd_msg_tbl->mutex);
    for (int i = 0; i < MAX_TBL_SIZE; i++)
    {
        if (recvd_msg_tbl->messages[i] != NULL)
        {
            free(recvd_msg_tbl->messages[i]->msg);
            free(recvd_msg_tbl->messages[i]);
        }
    }
    free(recvd_msg_tbl);
}

typedef struct _Send_Message_entry
{
    int msg_id;
    char *msg;
    size_t msg_len;
    int flags;
    struct sockaddr dest_addr;
    socklen_t addrlen;
    struct timeval sent_time;
} Send_Message_entry;

typedef struct _Send_Message
{
    Send_Message_entry *messages[MAX_TBL_SIZE];
    int count;
    pthread_mutex_t mutex;
} Send_Message;

Send_Message *Send_Message_table; // Unacknowledged message table

// Initializes the unacknowledged message table
void init_Send_Message()
{
    Send_Message_table = (Send_Message *)malloc(sizeof(Send_Message));
    for (int i = 0; i < MAX_TBL_SIZE; i++)
    {
        Send_Message_table->messages[i] = NULL;
    }
    Send_Message_table->count = 0;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK_NP);
    pthread_mutex_init(&Send_Message_table->mutex, &attr);
}

// Adds a message to the unacknowledged message table
void insert_Send_Message(char *buf, size_t len, int flags, const struct sockaddr *dest_addr,
                         socklen_t addrlen, struct timeval sent_time, short msg_id)
{
    for (int i = 0; i < MAX_TBL_SIZE; i++)
    {
        if (Send_Message_table->messages[i] == NULL)
        {
            Send_Message_table->messages[i] = (Send_Message_entry *)malloc(sizeof(Send_Message_entry));
            Send_Message_entry *curr_entry = Send_Message_table->messages[i];
            curr_entry->msg = (char *)malloc(len * sizeof(char));
            curr_entry->msg_id = msg_id;
            curr_entry->msg_len = len;
            curr_entry->flags = flags;
            curr_entry->dest_addr = *(struct sockaddr *)dest_addr;
            curr_entry->addrlen = addrlen;
            curr_entry->sent_time = sent_time;
            memcpy(curr_entry->msg, buf, len);
            Send_Message_table->count++;
            return;
        }
    }
}

// Removes a message from the unacknowledged message table
void delete_Send_Message(short msg_id)
{
    for (int i = 0; i < MAX_TBL_SIZE; i++)
    {
        if (Send_Message_table->messages[i] != NULL && Send_Message_table->messages[i]->msg_id == msg_id)
        {
            free(Send_Message_table->messages[i]->msg);
            free(Send_Message_table->messages[i]);
            Send_Message_table->messages[i] = NULL;
            Send_Message_table->count--;
            return;
        }
    }
}

// Frees the memory allocated to the received message table
void free_Send_Message()
{
    pthread_mutex_destroy(&Send_Message_table->mutex);
    for (int i = 0; i < MAX_TBL_SIZE; i++)
    {
        if (Send_Message_table->messages[i] != NULL)
        {
            free(Send_Message_table->messages[i]->msg);
            free(Send_Message_table->messages[i]);
        }
    }
    free(Send_Message_table);
}

/*
    Function for thread R

    Waits for message by calling recvfrom (this will get a frame)
    If it is a data message, calld dropMessage, accordingly adds to the received-message table and sends back an ACK
    If it is an ACK message, delete the appropriate message from the unacknowledged-message table
*/
void *recv_thread(void *arg)
{
    int sockfd = *(int *)arg;
    while (1)
    {
        struct sockaddr src_addr;
        socklen_t addrlen = sizeof(src_addr);
        char buf[MAX_FRAME_SIZE];
        pthread_testcancel(); // cancellation point
        ssize_t len = recvfrom(sockfd, buf, MAX_FRAME_SIZE, 0, &src_addr, &addrlen);
        if (len >= 0)
        {
            int drop = ((len > 0) && dropMessage(P));
            if (drop)
            {
                continue;
            }
            if (len == 0 || (len > 0 && buf[0] == DATA_TYPE))
            {
                LOCK(&recvd_msg_tbl->mutex);
                while (recvd_msg_tbl->count == MAX_TBL_SIZE)
                {
                    UNLOCK(&recvd_msg_tbl->mutex);
                    usleep(100);
                    LOCK(&recvd_msg_tbl->mutex);
                }
                ssize_t entry_len = ((len > 0) ? len - TYPE_SIZE - MSG_ID_SIZE : 0);
                enqueue_Received_Message(buf, entry_len, &src_addr, addrlen);
                UNLOCK(&recvd_msg_tbl->mutex);

                // need to send acknowledgement if len > 0
                if (len > 0)
                {
                    char ack_frame[MAX_FRAME_SIZE];
                    ack_frame[0] = ACK_TYPE;
                    short msg_id = ntohs(*(short *)(buf + TYPE_SIZE));
                    short t = htons(msg_id);
                    memcpy(ack_frame + TYPE_SIZE, &t, MSG_ID_SIZE);
                    sendto(sockfd, ack_frame, TYPE_SIZE + MSG_ID_SIZE, 0, &src_addr, addrlen);
                }
            }
            else
            {
            //     // assert(len > 0 && buf[0] == ACK_TYPE);
                short msg_id = ntohs(*(short *)(buf + TYPE_SIZE));
                LOCK(&Send_Message_table->mutex);
                delete_Send_Message(msg_id);
                UNLOCK(&Send_Message_table->mutex);
            }
        }
        pthread_testcancel(); // cancellation point
    }
}

/*
    Function for thread S

    Sleeps for time T
    On waking up, checks the unacknowledged-message table
    If any message has timed out, resends it and updates its timestamp
    Does this for all messages in the unacknowledged-message table
*/
void *send_thread(void *arg)
{
    int sockfd = *(int *)arg;
    while (1)
    {
        sleep(T);
        LOCK(&Send_Message_table->mutex);
        for (int i = 0; i < MAX_TBL_SIZE; i++)
        {
            Send_Message_entry *curr_entry = Send_Message_table->messages[i];
            if (curr_entry != NULL)
            {
                struct timeval curr_time, diff;
                gettimeofday(&curr_time, NULL);
                timersub(&curr_time, &curr_entry->sent_time, &diff);
                if (diff.tv_sec > TIMEOUT)
                {
                    // check return value to see if other side has closed connection
                    curr_entry->sent_time = curr_time;
                    Send_Message_entry send_entry = *curr_entry;
                    UNLOCK(&Send_Message_table->mutex);
                    int ret = sendto(sockfd, send_entry.msg, send_entry.msg_len, send_entry.flags, &send_entry.dest_addr, send_entry.addrlen);
                    if (ret >= 0)
                    {
                        tot_transm++;
                    }
                    LOCK(&Send_Message_table->mutex);
                }
            }
        }
        UNLOCK(&Send_Message_table->mutex);
        pthread_testcancel(); // cancellation point
    }
}

int dropMessage(float p)
{
    float rnd = (float)rand() / (float)RAND_MAX;
    return (rnd < p);
}

// Opens a UDP socket, creates threads R and S, allocates space for the 2 tables
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
    int *sockfd_arg = (int *)malloc(sizeof(int));
    *sockfd_arg = sockfd;
    if (sockfd >= 0)
    {
        init_Received_Message();
        init_Send_Message();
        // if (pthread_create(&tid_R, NULL, recv_thread, sockfd_arg) != 0)
        // {
        //     perror("pthread_create R");
        //     return -1;
        // }
        // if (pthread_create(&tid_S, NULL, send_thread, sockfd_arg) != 0)
        // {
        //     perror("pthread_create S");
        //     return -1;
        // }
    }
    return sockfd;
}

// Binds the socket with an address and a port
int my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    int ret = bind(sockfd, addr, addrlen);
    if(ret == 0)
    {
        dest_addr = addr;
        dest_addr_len = addrlen;
    }
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
int my_accept(int sockfd, struct sockaddr *cli_addr, socklen_t cli_len)
{
    int newsockfd = 0;
    if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &cli_len)) < 0)
    {
        printf("Accept Failed\n");
        exit(1);
    }
    return newsockfd;
}
// Wrapper Around Connect Call
int my_connect(int sockfd, struct sockaddr *serv_addr, socklen_t serv_len)
{
    int ret = connect(sockfd, serv_addr, serv_len);
    return ret;
}

// Sends the message (by adding extra bytes to make the frame), and adds the message to the unacknowledged-message table
ssize_t my_send(int sockfd, const char *buf, size_t len, int flags)
{
    char data_frame[MAX_FRAME_SIZE];
    data_frame[0] = DATA_TYPE;
    short msg_id = msg_cntr++;
    short msg_id_buf = htons(msg_id);
    memcpy(data_frame + TYPE_SIZE, &msg_id_buf, MSG_ID_SIZE);
    memcpy(data_frame + TYPE_SIZE + MSG_ID_SIZE, buf, len);
    struct timeval sent_time;
    gettimeofday(&sent_time, NULL);
    ssize_t sent_len = send(sockfd, data_frame, TYPE_SIZE + MSG_ID_SIZE + len, flags);
    if (sent_len >= 0)
    {
        tot_transm++;
        LOCK(&Send_Message_table->mutex);
        insert_Send_Message(data_frame, len + TYPE_SIZE + MSG_ID_SIZE, flags, dest_addr, dest_addr_len, sent_time, msg_id);
        UNLOCK(&Send_Message_table->mutex);
    }
    return sent_len;
}

// Checks the received-message table, if there is a message then returns that message and deletes it from the table
// If there is no message, sleeps for some time and checks again
ssize_t my_recv(int sockfd, char *buf, size_t len, int flags)
{
    printf("Hello\n");
    char* temp =  (char *)malloc(len);
    size_t size = 0;
    while((size = recv(sockfd, temp, 1000, 0)) > 0 && len <= 5000)
    {
        strcat(buf, temp);
        bzero(temp, 1000);
        len += size;
    }
    // while (recv())
    // {
    //     LOCK(&recvd_msg_tbl->mutex);
    //     if (recvd_msg_tbl->count > 0)
    //     {
    //         size_t recv_len = enqueue_Received_Message(buf, len, dest_addr, dest_addr_len);
    //         UNLOCK(&recvd_msg_tbl->mutex);
    //         return recv_len;
    //     }
    //     else
    //     {
    //         UNLOCK(&recvd_msg_tbl->mutex);
    //         sleep(1);
    //     }
    // }
}

// Kills threads R and S, frees the tables, closes the socket
int my_close(int fd)
{
    // Check if everything has been acked, only then go ahead
    LOCK(&Send_Message_table->mutex);
    while (Send_Message_table->count > 0)
    {
        UNLOCK(&Send_Message_table->mutex);
        usleep(100);
        LOCK(&Send_Message_table->mutex);
    }

    pthread_cancel(tid_R);
    pthread_cancel(tid_S);
    pthread_join(tid_R, NULL);
    pthread_join(tid_S, NULL);

    free_Received_Message();
    free_Send_Message();

    int ret = close(fd);
    return ret;
}