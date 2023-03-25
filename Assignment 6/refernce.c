/*
    Execution Instructions:
    gcc mytraceroute_19CS30008.c -o mytraceroute
    sudo ./mytraceroute www.example.com
*/

#include <arpa/inet.h>
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
#define INFO(msg, ...) printf("\033[1;34m[INFO] " msg " \033[0m\n", ##__VA_ARGS__);
#define DEBUG(msg, ...) printf("\033[1;32m[DEBUG] " msg "\033[0m", ##__VA_ARGS__);

#define PAYLOAD_SIZE 52
#define DEST_PORT 32164
#define LOCAL_PORT 20000
#define MAX_SIZE 100
#define MAX_RUN_TIME 1000000  // microseconds

bool isNumber(char number[], int *p)
{
    int i = 0;
    *p = 0;
    for (; i < strlen(number) ; i++)
    {
        if (number[i] > '9' || number[i] < '0')
            return false;
        *p = *p*10 + (number[i]-'0'); 
    }
    return true;
}
int main(int argc, char *argv[])
{
    int *p = (int *)malloc(sizeof(int *)), max_hops, time_diff; 
    if (argc < 4) {
        ERROR("sudo ./pingnetinfo.c [-IP] [-n] [-T]\n");
        exit(EXIT_FAILURE);
    }

    struct hostent *h = gethostbyname(argv[1]);
    // if host not found
    if (h == NULL) {
        ERROR("Could not resolve host name");
        exit(EXIT_FAILURE);
    }

    if (!isNumber(argv[2], p)) {
        ERROR("Could not resolve number of times a probe will be sent per link (n)");
        exit(EXIT_FAILURE);
    }

    max_hops = *p;

    if (!isNumber(argv[3], p)) {
        ERROR("Could not resolve the time difference between any two probes (T)");
        exit(EXIT_FAILURE);
    }

    time_diff = *p;

    // get IP address of the host
    struct in_addr dest_ip = *(struct in_addr *)h->h_addr_list[0];

    // print destination ip
    printf("Destination IP for %s [ %s ]\n\n", argv[1], inet_ntoa(dest_ip));
    printf("over a maximum of %d hops and time difference between any two probes (T) : %d :-\n\n", max_hops, time_diff);
    int sockfd_udp, sockfd_icmp;
    struct sockaddr_in local_addr, dest_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    memset(&dest_addr, 0, sizeof(dest_addr));

    // set local address
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(LOCAL_PORT);
    local_addr.sin_addr.s_addr = INADDR_ANY;

    // create socket for udp
    if ((sockfd_udp = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0) {
        perror("Could not create udp socket");
        exit(EXIT_FAILURE);
    }
    // set IP_HDRINCL to true to tell the kernel that headers are included in the packet
    int opt = 1;
    if (setsockopt(sockfd_udp, IPPROTO_IP, IP_HDRINCL, &opt, sizeof(opt)) < 0) {
        perror("Could not set socket option for udp socket");
        exit(1);
    }
    // bind the udp socket to local address
    if (bind(sockfd_udp, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        perror("Could not bind udp socket");
        exit(1);
    }

    // create socket for icmp
    if ((sockfd_icmp = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        perror("Could not create icmp socket");
        exit(1);
    }
    // bind the icmp socket to local address
    if (bind(sockfd_icmp, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        perror("Could not bind icmp socket");
        exit(1);
    }

    // set destination address
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(DEST_PORT);
    dest_addr.sin_addr = dest_ip;

    int ttl = 1;
    // iterate on Incremental Max Hop Values to check the Route of the Packet
    
    while(ttl <= max_hops)
    {
        
        ttl++;
    }
    
    close(sockfd_udp);   // close the udp socket
    close(sockfd_icmp);  // close the icmp socket
    exit(EXIT_SUCCESS);
}

