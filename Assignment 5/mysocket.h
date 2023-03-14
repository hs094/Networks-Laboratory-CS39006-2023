#ifndef COMMON_HEADER
#define COMMON_HEADER

#include <stdio.h>
#include <arpa/inet.h>//for htonl and sockaddr_in
#include <unistd.h>//for getopt,read,write,close
#include <stdlib.h>//for atoi
#include <string.h>//for memset and memcpy
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>//for errno
#include <signal.h>//for signal func
#include <pthread.h>
#include <netinet/in.h>
#include <time.h> 
#include "threadpool.h"
#include <stdbool.h>

#define TCP_PORT  7878
#define BUF_SIZE 100
#define POOL_SIZE 2
#define QUEUE_SIZE 10
#define MAX_MSG_SIZE 5000
#define T 10

bool stop = false;
char Send_Message[MAX_MSG_SIZE];
char Received_Message[MAX_MSG_SIZE];
// queue<Info> ReceiveTable;
// queue<Info> SendTable;

void ctrl_c_signal_handler(int sig_num);
void my_socket();  // This  function  opens  a  standard  TCP  socket  with  the  socket  call.  It  also  creates two threads R and S (to be described later), allocates and initializes space for two  tables  Send_Messageand Received_Message (to  be  described  later),  and  any  additional  space  that  may  be  needed.  The  parameters  to  these  are  the  same  as  the  normal socket( ) call, except that it will take only  SOCK_MyTCP as the socket type.  
void my_bind();    // binds the socket with some address-port using the bind call 
void my_listen();  // makes  a listen call 
void my_accept();  // accepts a connection on the MyTCP socket by making the accept call on the TCP socket (only on server side)
void my_connect(); // opens a connection through the MyTCP socket by making the connectcall on the TCP socket
void my_send();    // sends a message (see description later). One message is defined as what is sent in one my_send call.
void my_recv();    // receives  a  message.  This  is  a  blocking  call  and  will  return  only  if  a  message, sent using the my_send call, is received. If the buffer length specified is less than the message length, the extra bytes in the message are discarded (similar to that in UDP).
void my_close();   // closes the socket and cleans up everything. If there are any messages to be sent/received, they are discarded.

#endif