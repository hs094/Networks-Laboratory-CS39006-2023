#include "mysocket.h"
#include "threadpool.c"

struct sockaddr_in serv_addr;
int listenfd = 0, connfd = 0;
int tcp_port = 0;
int num_threads = POOL_SIZE;


void ctrl_c_signal_handler(int sig_num);
const int queue_size=15;
threadpool_t *tp;

void ctrl_c_signal_handler(int sig_num){
    printf("Exiting\n");
    stop=true;
}
void my_socket() // This  function  opens  a  standard  TCP  socket  with  the  socket  call.  It  also  creates two threads R and S (to be described later), allocates and initializes space for two  tables  Send_Messageand Received_Message (to  be  described  later),  and  any  additional  space  that  may  be  needed.  The  parameters  to  these  are  the  same  as  the  normal socket( ) call, except that it will take only  SOCK_MyTCP as the socket type.
{
    //AF_INET connection with differnent machine
    //SOCK_STREAM TCP connection
    //SOCK_NONBLOCK mark socket as non blocking
    //when accept is called and there is no pending connections
    //accept blocks but with SOCK_NONBLOCK it returns immidiately with 
    //error EAGAIN or EWOULDBLOCK
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    if((listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0))== -1)
    {
        perror("Error : Could not create socket\n");
        printf("Errno %d\n",errno);
        exit(1);
    }
    tp=threadpool_create(num_threads,queue_size,0);
    printf("Thread pool size %d\n",tp->thread_count);
    signal(SIGINT,ctrl_c_signal_handler);
    return;
}
void my_bind() // binds the socket with some address-port using the bind call.•my_listen – makes  a listen call.
{
    memset(&serv_addr, '0', sizeof(serv_addr));

    //AF_INET tell that the connection is with different machine
    //AF_UNIX connect inside same machine
    serv_addr.sin_family = AF_INET;

    //To accept connctions from all IPs
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    //The htonl(uint32_t hostlong) function converts the unsigned integer 
    //hostlong from host byte order to network byte order.
    //htonl was giving wrong value htons gives correct
    serv_addr.sin_port=htons(tcp_port);

    if(bind(listenfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))==-1){
        printf("Error:Bindint with port # %d failed\n",tcp_port);
        printf("Errno %d\n",errno);
        if(errno == EADDRINUSE)
            printf("Another socket is already listening on the same port\n");
        exit(1);
    }
    
    return;
}
void my_listen()
{
    //The backlog(second arg of listen) argument defines the maximum length to which the queue of
    //pending connections for sockfd may grow. If a connection request 
    //arrives when the queue is full, the client may receive an error
    // with an indication of ECONNREFUSED or, if the
    // underlying protocol supports retransmission, 
    //the request may be ignored so that a later reattempt at connection succeeds.
    if(listen(listenfd, QUEUE_SIZE) == -1){
        printf("Error:Failed to listen\n");
        printf("Errno %d\n",errno);
        if(errno == EADDRINUSE)
            printf("Another socket is already listening on the same port\n");
        exit(1);
    }

    printf("Lintning on TCP port %d\n",tcp_port);
    return;
}
void my_accept() // accepts a connection on the MyTCP socket by making the accept call on the TCP socket (only on server side)
{
    while(!stop){
        connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL); // accept awaiting request
        if(connfd!=-1){
            // threadpool_add(tp,server_run,connfd,0);
        }else{
            //sleep for 0.5 seconds
            usleep(500000);
        }
    }
    return;
}
void my_connect() // opens a connection through the MyTCP socket by making the connectcall on the TCP socket
{
    return;
}
void my_send() // sends a message (see description later). One message is defined as what is sent in one my_send call.
{
    return;
}
void my_recv() // receives  a  message.  This  is  a  blocking  call  and  will  return  only  if  a  message, sent using the my_send call, is received. If the buffer length specified is less than the message length, the extra bytes in the message are discarded (similar to that in UDP).
{
    return;
}
void my_close() // closes the socket and cleans up everything. If there are any messages to be sent/received, they are discarded.
{
    close(listenfd);
    threadpool_destroy(tp,0);//wait until all tasks are finished
    return;
}
int main()
{
    my_socket();
    my_bind();
    my_accept();
    my_connect();
    my_send();
    my_recv();
    my_close();
    return 0;
}