#include "mysocket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT_1 50016
#define MAX_MSG_LEN 5000
// Client
int main()
{
    int sockfd;
    struct sockaddr_in serv_addr;
    if ((sockfd = my_socket(AF_INET, SOCK_MyTCP, 0)) < 0)
    {
        perror("my_socket");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT_1);

    char msg[MAX_MSG_LEN];
    while (1)
    {
        my_connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        memset(msg, 0, MAX_MSG_LEN);
        printf("Enter a message: ");
        scanf("%[^\n]s", msg);   
        int msg_len = strlen(msg);
        // for (int i = 0; i < msg_len; i++)
        // {
        //     int ret = my_send(sockfd, &msg[i], 1, 0, (struct sockaddr *)&u2_addr, sizeof(u2_addr));
        //     if (ret < 0)
        //     {
        //         perror("my_sendto");
        //         exit(1);
        //     }
        // }
    }
    my_close(sockfd);
    return 0;
}
