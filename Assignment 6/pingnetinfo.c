
/*
      Author:-
    Hardik Soni
    20CS30023
*/

/* How to RUN:
$] gcc pingnetinfo.c -o tr
$] sudo ./tr www.iitkgp.ac.in

*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#define ERROR(msg, ...) printf("\033[1;31m[ERROR] " msg " \033[0m\n", ##__VA_ARGS__);
#define SUCCESS(msg, ...) printf("\033[1;36m[PING] " msg " \033[0m\n", ##__VA_ARGS__);
#define INFO(msg, ...) printf("\033[1;34m" msg " \033[0m\n", ##__VA_ARGS__);
#define DEBUG(msg, ...) printf("\033[1;32m[DEBUG] " msg "\033[0m", ##__VA_ARGS__);

#define N 52
#define MSG_SIZE 2048
#define MAX_CHAR 100
#define PCKT_LEN 8192
#define MAX_HOP 64
#define DEST_PORT 32164
#define S_PORT 8080

int TIMEOUT = 1;
int rawfd1, rawfd2;
struct sockaddr_in saddr_raw, cli_addr;
socklen_t saddr_raw_len;
char buf[100];
u_int16_t src_port, dst_port;
u_int32_t dst_addr;
struct iphdr *ip;
struct udphdr *udp;

void print_Usage_Error(char *name)
{
    ERROR("Error: Invalid parameters!\n");
    ERROR("Usage: sudo %s [-IP] [-n] [-T]\n", name);
    ERROR("Where:- \n\t-> IP:- IP Address in IPv4\n\t-> n:- the number of times a probe will be sent per link\n\t-> T:- the time difference between any two probes.\n");
}

bool isNumber(char number[], int *p)
{
    int i = 0;
    *p = 0;
    for (; i < strlen(number); i++)
    {
        if (number[i] > '9' || number[i] < '0')
            return false;
        *p = *p * 10 + (number[i] - '0');
    }
    return true;
}
/* Function to generate random string */
void gen(char *dst)
{
    for (int i = 0; i < N; i++)
    {
        dst[i] = rand() % 26 + 'A';
    }
    dst[N - 1] = '\0';
}

/*The IPv4 layer generates an IP header when sending a packet unless the IP_HDRINCL socket option is enabled on the socket.
When it is enabled, the packet must contain an IP header.*/

/* Function to find IP  */
int hostname_to_ip(char *hostname, char *ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    if ((he = gethostbyname(hostname)) == NULL)
    {
        // get the host info
        herror("gethostbyname");
        return 1;
    }

    addr_list = (struct in_addr **)he->h_addr_list;
    if (addr_list[0] == NULL)
        return 1;
    else
    {
        strcpy(ip, inet_ntoa(*addr_list[0]));
        return 0;
    }
}
/* Check sum function  */
unsigned short csum(unsigned short *buf, int nwords)
{
    unsigned long sum;
    for (sum = 0; nwords > 0; nwords--)
        sum += *buf++;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

void form_socket(char s[])
{
    if (strcmp(s, "IPPROTO_UDP") == 0)
    {
        if ((rawfd1 = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0)
        {
            perror("Socket error");
            exit(EXIT_FAILURE);
        }
    }
    else if (strcmp(s, "IPPROTO_ICMP"))
    {
        if ((rawfd2 = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
        {
            perror("Socket error");
            exit(EXIT_FAILURE);
        }
    }
}
void sendICMP(int ttl, char buffer[], char payload[])
{
    /* 5. Generate UPD and IP header */
    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0;                                                    // low delay
    ip->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + N; // https://tools.ietf.org/html/rfc791#page-11
    ip->id = htons(54322);
    ip->ttl = ttl;     // hops
    ip->protocol = 17; // UDP
    ip->saddr = 0;     // src_addr;
    ip->daddr = dst_addr;

    // fabricate the UDP header
    udp->source = htons(src_port);
    // destination port number
    udp->dest = htons(dst_port);
    udp->len = htons(sizeof(struct udphdr) + N);

    // calculate the checksum for integrity
    ip->check = csum((unsigned short *)buffer, sizeof(struct iphdr) + sizeof(struct udphdr));

    /* 6. Send the packet */
    strcpy(buffer + sizeof(struct iphdr) + sizeof(struct udphdr), payload);
    if (sendto(rawfd1, buffer, ip->tot_len, 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr)) < 0)
    {
        perror("sendto()");
        close(rawfd1);
        close(rawfd2);
        exit(EXIT_FAILURE);
    }
}
int main(int argc, char *argv[])
{
    int *p = (int *)malloc(sizeof(int *)), max_hops;
    if (argc != 4)
    {
        print_Usage_Error(argv[0]);
        exit(EXIT_FAILURE);
    }

    if (!isNumber(argv[2], p))
    {
        ERROR("Could not resolve number of times a probe will be sent per link (n)");
        exit(EXIT_FAILURE);
    }

    max_hops = *p;

    if (!isNumber(argv[3], p))
    {
        ERROR("Could not resolve the time difference between any two probes (T)");
        exit(EXIT_FAILURE);
    }

    TIMEOUT = *p;
    // bzero(buf,0);
    // strcpy(buf, "IPPROTO_UDP");
    // form_socket(buf);
    // bzero(buf,0);
    // strcpy(buf, "IPPROTO_ICMP");
    // form_socket(buf);
    // /* 1. Create two Raw Socket */
    if ((rawfd1 = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0)
    {
        perror("Socket error");
        exit(EXIT_FAILURE);
    }
    if ((rawfd2 = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
    {
        perror("Socket error");
        exit(EXIT_FAILURE);
    }

    /* 2. get the destination IP */
    char ipaddr[MAX_CHAR];
    if (hostname_to_ip(argv[1], ipaddr) != 0)
    {
        close(rawfd1);
        close(rawfd2);
        exit(EXIT_FAILURE);
    }

    dst_addr = inet_addr(ipaddr);
    src_port = S_PORT;
    dst_port = DEST_PORT;
    saddr_raw.sin_family = AF_INET;
    saddr_raw.sin_port = htons(src_port);
    saddr_raw.sin_addr.s_addr = INADDR_ANY; // inet_addr(LISTEN_IP);
    saddr_raw_len = sizeof(saddr_raw);
    /* 3. Bind the Sockets */
    if (bind(rawfd1, (struct sockaddr *)&saddr_raw, saddr_raw_len) < 0)
    {
        perror("raw bind");
        close(rawfd1);
        close(rawfd2);
        exit(1);
    }

    SUCCESS("PingNetInfo to %s (%s), %d hops max, %d byte packets", argv[1], ipaddr, MAX_HOP, N);
    INFO("TTL \tIPv4 Address \tResponse Time \tLatency \tBandwidth");
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_port = htons(dst_port);
    cli_addr.sin_addr.s_addr = dst_addr;

    int one = 1;
    const int *val = &one;
    if (setsockopt(rawfd1, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0)
    {
        fprintf(stderr, "Error: setsockopt. You need to run this program as root\n");
        close(rawfd1);
        close(rawfd2);
        exit(EXIT_FAILURE);
    }
    int ttl = 1, timeout = TIMEOUT, is_send = 1;
    fd_set readSockSet;
    int times = 0;
    char payload[52];
    clock_t start_time;
    while (1)
    {
        if (ttl >= 64)
            break;
        char buffer[PCKT_LEN];
        ip = (struct iphdr *)buffer;
        udp = (struct udphdr *)(buffer + sizeof(struct iphdr));
        if (is_send)
        {
            /* 4. generate Payload */
            times++;
            gen(payload);
            memset(buffer, 0, PCKT_LEN);
            sendICMP(ttl, buffer, payload);
            start_time = clock();
        }
        /* 7. Wait on select call */
        FD_ZERO(&readSockSet);
        FD_SET(rawfd2, &readSockSet);
        struct timeval tv = {timeout, 0};
        int ret = select(rawfd2 + 1, &readSockSet, 0, 0, &tv);
        if (ret == -1)
        {
            perror("select()\n");
            close(rawfd1);
            close(rawfd2);
            exit(EXIT_FAILURE);
        }
        else if (ret)
        {
            // ICMP
            if (FD_ISSET(rawfd2, &readSockSet))
            {
                /* 8. Read the ICMP Message */
                // printf("ICMP\n");
                char msg[MAX_CHAR];
                int msglen;
                socklen_t raddr_len = sizeof(saddr_raw);
                msglen = recvfrom(rawfd2, msg, MSG_SIZE, 0, (struct sockaddr *)&saddr_raw, &raddr_len);
                clock_t end_time = clock();
                if (msglen <= 0)
                {
                    timeout = TIMEOUT;
                    is_send = 1;
                    continue;
                }
                struct iphdr hdrip = *((struct iphdr *)msg);
                int iphdrlen = sizeof(hdrip);
                struct icmphdr hdricmp = *((struct icmphdr *)(msg + iphdrlen));
                /* 9. Handle Different Case */
                // read the destination IP
                struct in_addr saddr_ip;
                saddr_ip.s_addr = hdrip.saddr;
                if (hdrip.protocol == 1) // ICMP
                {
                    if (hdricmp.type == 3)
                    {
                        // verify

                        if (hdrip.saddr == ip->daddr)
                            printf("%d\t%s\t%.3f ms\n", ttl, inet_ntoa(saddr_ip), (float)(end_time - start_time) / CLOCKS_PER_SEC * 1000);
                        close(rawfd1);
                        close(rawfd2);
                        exit(EXIT_SUCCESS);
                    }
                    else if (hdricmp.type == 11)
                    {
                        // time exceed
                        printf("%d\t%s\t%.3f ms\n", ttl, inet_ntoa(saddr_ip), (float)(end_time - start_time) / CLOCKS_PER_SEC * 1000);
                        ttl++;
                        times = 1;
                        timeout = TIMEOUT;
                        is_send = 1;
                        continue;
                    }
                }
                else
                {
                    // Ignore the message
                    //  printf("ignore\n");
                    is_send = 0;
                    timeout = end_time - start_time;
                    if (timeout >= 0.01)
                        continue;
                    else
                    {
                        if (times > 3)
                        {
                            printf("%d\t*\t*\n", ttl);
                            times = 1;
                            ttl++;
                        }
                        timeout = TIMEOUT;
                        is_send = 1;
                        continue;
                    }
                }
            }
        }
        else
        {
            // timeou
            if (times > 3)
            {
                printf("%d\t*\t*\n", ttl);
                times = 1;
                ttl++;
            }
            timeout = TIMEOUT;
            is_send = 1;
            continue;
        }
    }
    close(rawfd1);
    close(rawfd2);
    exit(EXIT_SUCCESS);
}