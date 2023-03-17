
1 of 25
Fwd:
Inbox

Vinod Meena <vinodmeena.iitkgp@gmail.com>
Attachments
Thu, Mar 16, 6:00 PM (17 hours ago)
to me


---------- Forwarded message ---------
From: Vinod Meena <vinodmeena.iitkgp@gmail.com>
Date: Thu, Mar 16, 2023, 12:18 AM
Subject:
To: Nirbhay Kumar <nirbhay.jnvj@gmail.com>




4
 Attachments
  •  Scanned by Gmail

Hardik Soni <iamhardikat11@gmail.com>
Attachments
Thu, Mar 16, 7:01 PM (16 hours ago)
to archit.mangrulkar


4
 Attachments
  •  Scanned by Gmail

Hardik Soni <iamhardikat11@gmail.com>
Attachments
Tue, Mar 16, 2021, 8:58 PM
to archit.mangrulkar



Mailtrack	Sender notified by
Mailtrack	16/03/23, 20:55:59	
5
 Attachments
  •  Scanned by Gmail
/**
 * @file mysocket.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-03-15
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "mysocket.h"
#define MAX_QUEUE_SIZE 10


int is_connect ;
int my_sockfd  ;

typedef struct
{
    char *items[MAX_QUEUE_SIZE];
    int front;
    int rear;
    int size  ; 
} Queue;

/**
 * @brief Create a Queue object
 * 
 * @return Queue* 
 */
Queue *createQueue()
{
    Queue *queue = (Queue *)malloc(sizeof(Queue));
    queue->front = 0;
    queue->rear = 0;
    queue->size = 0 ;
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
    return (queue->size == MAX_QUEUE_SIZE );
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
    queue->items[queue->rear] = (char *)malloc(strlen(item) + 1);
    strcpy(queue->items[queue->rear], item);
    queue->size++ ;  
    queue->rear = ( queue->rear + 1 ) % 10  ;
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
    char *item = queue->items[queue->front];
    queue->front = ( queue->front + 1) % 10;
    queue->size-- ;
    return item;
}

Queue *Send_Message, *Received_Message;

pthread_t R, S;

pthread_mutex_t Rq_lock, Sq_lock;
pthread_cond_t Rq_cond, Sq_cond;

/**
 * @brief 
 * 
 * @param arg 
 * @return void* 
 */
void *func_R(void *arg)
{
    while (1)
    {
        if( is_connect == 0 )continue; 
        int sockfd = my_sockfd;
        char buf[1000];
        char message[5000];
        
        int mess_bytes = 0 ; 
        char buffer[4] ;

        while( 4 - mess_bytes > 0 )
        {
            int rec_bytes = recv( sockfd , buffer + mess_bytes , 4 - mess_bytes  , 0 ) ;
            mess_bytes += rec_bytes  ; 
            if( rec_bytes == 0 )
            {
                is_connect = 0 ;
                break;
            }
            // printf("0\n");
        }
        int mess_size = 0 ;
        for( int i = 3 ; i >= 0 ; i-- )
        {
            mess_size = mess_size * 10 + buffer[i] - '0' ; 
        }
        printf("size recieved  : %d \n" , mess_size ) ;
        int total_rec_bytes = 0 ;
        while( mess_size - total_rec_bytes > 0  )
        {
            int rec_bytes = recv( sockfd , message + total_rec_bytes , mess_size - total_rec_bytes  , 0 ) ;
            if( rec_bytes == 0 )
            {
                is_connect = 0 ;
                break;
            }
            total_rec_bytes += rec_bytes  ;   
            // printf("1\n") ;
        }

        // printf("mess rec : %s queue size : %d \n",message , Received_Message->rear + 1 );

        // Critical Section Starts

        pthread_mutex_lock(&Rq_lock);

        while (isFull(Received_Message))
        {
            pthread_cond_wait(&Rq_cond, &Rq_lock);
            printf("waiting\n");
        }
        enqueue(Received_Message, message);
        printf("queue size : %d\n", Received_Message->size ) ;
        pthread_cond_signal(&Rq_cond);
        pthread_mutex_unlock(&Rq_lock);

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
void *func_S(void *arg)
{
    while (1)
    {
        if( is_connect == 0 )continue;
        int sockfd = my_sockfd;
        char buf[1000];
        char message[5000];


        // Critical Section Starts

        pthread_mutex_lock(&Sq_lock);

        while (isEmpty(Send_Message))
        {
            pthread_cond_wait(&Sq_cond, &Sq_lock);
        }
        strcpy(message, dequeue(Send_Message));

        pthread_cond_signal(&Sq_cond);
        pthread_mutex_unlock(&Sq_lock);
        

        // Critical Section Ends

        int mess_size = strlen(message) ; 
        if( mess_size > 5000 )mess_size = 5000 ;
        int n = mess_size ;
        char size_buff[4] ;
        for( int  i = 0 ; i < 4 ; i++ )
        {
            size_buff[i] = '0' + n % 10 ; 
            n = n / 10 ; 
        }
        send( sockfd , size_buff , 4  , 0 ) ;
        printf("Size sent : %d \n",mess_size);
        
        int sent_len = 0;
        while (sent_len < mess_size)
        {
            int send_size = (mess_size - sent_len > 1000)? 1000 : mess_size - sent_len;
            sent_len += send( sockfd , message + sent_len , send_size , 0 ) ; 
            // printf("3\n") ;
        }

        
        // send_message( message , sockfd ) ;
        // sleep(5);
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

int my_socket(int family, int type, int protocol)
{
    int fd = socket(family, SOCK_STREAM, protocol);
    if (fd < 0) {
		perror("Cannot create socket\n");
		exit(0);
	}
    my_sockfd = fd ;
    char sockfd[10]; 

    sprintf( sockfd ,"%d", fd ) ;
    // cond inits
    pthread_cond_init(&Rq_cond, NULL);
    pthread_cond_init(&Sq_cond, NULL);

    // mutex inits
    pthread_mutex_init(&Rq_lock, NULL);
    pthread_mutex_init(&Sq_lock, NULL);

    Send_Message = createQueue();
    Received_Message = createQueue();
    pthread_create(&R, NULL, func_R, (void *)sockfd);
    pthread_create(&S, NULL, func_S, (void *)sockfd);
    return fd;
}

/**
 * @brief 
 * 
 * @param Sockfd 
 * @param addr 
 * @param addrlen 
 * @return int 
 */

int my_bind(int Sockfd, struct sockaddr *addr, socklen_t addrlen)
{
    int bind_val = bind(Sockfd, addr,
                addrlen);
    if (bind_val < 0) {
		perror("Unable to bind local address\n");
		exit(0);
	}
    return bind_val ;
}

/**
 * @brief 
 * 
 * @param Sockfd 
 * @param clients 
 * @return int 
 */
int my_listen(int Sockfd, int clients)
{
    return listen(Sockfd, clients);
}

/**
 * @brief 
 * 
 * @param Sockfd 
 * @param addr 
 * @param addrlen 
 * @return int 
 */

int my_accept(int Sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    my_sockfd = accept(Sockfd, addr, addrlen);
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

int my_connect(int Sockfd, struct sockaddr *addr, socklen_t addrlen)
{
    int connect_val = connect(Sockfd, addr, addrlen); 
    if (connect_val < 0)
	{
		perror("Unable to connect to server\n");
		exit(0);
	}
    is_connect = 1 ;
    return connect_val ;
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
ssize_t my_send(int Sockfd,  char *buf, size_t len, int flags)
{

    // Critical Section Starts

    pthread_mutex_lock(&Sq_lock);
    while (isFull(Send_Message))
    {
        pthread_cond_wait(&Sq_cond, &Sq_lock);
    }

    enqueue(Send_Message, buf);

    pthread_cond_signal(&Sq_cond);
    pthread_mutex_unlock(&Sq_lock);

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
ssize_t my_recv(int Sockfd,  char *buf, size_t len, int flags)
{
    // Critical Section Starts
    
    pthread_mutex_lock(&Rq_lock);
    while (isEmpty(Received_Message))
    {  
        pthread_cond_wait(&Rq_cond, &Rq_lock); 
    }
    strcpy(buf, dequeue(Received_Message));
    pthread_cond_signal(&Rq_cond);
    pthread_mutex_unlock(&Rq_lock);

    // Critical Section Ends
    return strlen(buf);
}

/**
 * @brief 
 * 
 * @param Sockfd 
 * @return int 
 */
int my_close(int Sockfd)
{
    pthread_cancel(R);
    pthread_cancel(S);
    destroyQueue(Send_Message);
    destroyQueue(Received_Message);
    close(Sockfd);
}