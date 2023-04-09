/*
      Author:-
    Hardik Soni 20CS30023
    Archit Mangrulkar 20CS10086
*/

/* How to RUN:
$] gcc pingnetinfo.c -o t
$] sudo ./t www.iitkgp.ac.in 1 2
*/

/*
    The Output for the Header Information will be printed in the file with name "output_file"
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
#include <poll.h>

#define ERROR(msg, ...) printf("\033[1;31m[ERROR] " msg " \033[0m\n", ##__VA_ARGS__);
#define SUCCESS(msg, ...) printf("\033[1;36m[PING] " msg " \033[0m\n", ##__VA_ARGS__);
#define INFO(msg, ...) printf("\033[1;34m" msg " \033[0m\n", ##__VA_ARGS__);
#define DEBUG(msg, ...) printf("\033[1;32m[DEBUG] " msg "\033[0m", ##__VA_ARGS__);

#define N 60
#define MSG_SIZE 2048
#define MAX_CHAR 100
#define PCKT_LEN 8192
#define MAX_HOP 64
#define DEST_PORT 32164
#define S_PORT 8080
#define WAIT_D 0.01
#define TIMEOUT 1

// int TIMEOUT = 1;
int rawfd1, rawfd2, probe, time_probe;
struct sockaddr_in saddr_raw, cli_addr;
socklen_t saddr_raw_len;
char buf[100], payload[52], ipaddr[MAX_CHAR];
u_int16_t src_port, dst_port;
u_int32_t dst_addr;
struct iphdr *ip;
struct udphdr *udp;
char *output_file = "output.txt";
FILE *fp;

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

int min(int a, int b)
{
    return a > b ? b : a;
}

/* Function to generate random string */
void gen(char *dst, int len)
{
    for (int i = 0; i < min(N, len); i++)
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
void print_ICMP(struct iphdr ip, struct icmphdr hdricmp)
{
    // IP Header
    fprintf(fp, "------------------------------------------------------------------\n");
    fprintf(fp, "IP Header\n");
    fprintf(fp, "---------\n");
    fprintf(fp, "Version:        %d\n", ip.version);
    fprintf(fp, "Header Length:  %d bytes\n", ip.ihl * 4);
    fprintf(fp, "Type of Service:%d\n", ip.tos);
    fprintf(fp, "Total Length:   %d bytes\n", ntohs(ip.tot_len));
    fprintf(fp, "Identification: %d\n", ntohs(ip.id));
    fprintf(fp, "Time To Live:   %d\n", ip.ttl);
    if (ip.protocol == IPPROTO_TCP)
    {
        // Handle TCP packet
        fprintf(fp, "Protocol:       TCP\n");
    }
    else if (ip.protocol == IPPROTO_UDP)
    {
        // Handle UDP packet
        fprintf(fp, "Protocol:       UDP\n");
    }
    else if (ip.protocol == IPPROTO_ICMP)
    {
        // Handle ICMP packet
        fprintf(fp, "Protocol:       ICMP\n");
    }
    else
    {
        // Handle other protocol types, or ignore the packet
        fprintf(fp, "Protocol:       Unknown Protocol\n");
    }
    fprintf(fp, "Source Address: %s\n", inet_ntoa(*(struct in_addr *)&ip.saddr));
    fprintf(fp, "Destination Address: %s\n", inet_ntoa(*(struct in_addr *)&ip.daddr));
    fprintf(fp, "Checksum:       %d\n", ntohs(ip.check));
    // ICMP Header
    fprintf(fp, "\nICMP Header\n");
    fprintf(fp, "-----------\n");
    fprintf(fp, "Type:           %d\n", hdricmp.type);
    fprintf(fp, "Code:           %d\n", hdricmp.code);
    fprintf(fp, "Checksum:       %d\n", ntohs(hdricmp.checksum));
    fprintf(fp, "Identifier:     %d\n", ntohs(hdricmp.un.echo.id));
    fprintf(fp, "Sequence Number:%d\n", ntohs(hdricmp.un.echo.sequence));
    fprintf(fp, "------------------------------------------------------------------\n");
}
clock_t sendICMP(int ttl, char buffer[], char payload[], int sz)
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
    // calculate the checksum for integrity
    ip->check = csum((unsigned short *)buffer, sizeof(struct iphdr) + sizeof(struct udphdr));
    // fabricate the UDP header
    udp->source = htons(src_port);
    // destination port number
    udp->dest = htons(dst_port);
    udp->len = htons(sizeof(struct udphdr) + N);
    /* 6. Send the packet */
    strcpy(buffer + sizeof(struct iphdr) + sizeof(struct udphdr), payload);
    if (sendto(rawfd1, buffer, ip->tot_len, 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr)) < 0)
    {
        perror("sendto()");
        close(rawfd1);
        close(rawfd2);
        fclose(fp);
        exit(EXIT_FAILURE);
    }
    return clock();
}
void networkICMP(char *argv[])
{
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
    if (hostname_to_ip(argv[1], ipaddr) != 0)
    {
        close(rawfd1);
        close(rawfd2);
        fclose(fp);
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
        fclose(fp);
        exit(EXIT_FAILURE);
    }

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
        fclose(fp);
        exit(EXIT_FAILURE);
    }
}
double val(double a, double b)
{
    a = a - b;
    if (a < 0)
        a = -a;
    return a;
}
double computeBandWidth(int ttl, char buffer[], char payload[], double last_bd)
{
    double time = 0.0;
    for (int i = 0; i < probe; i++)
    {
        int len1 = rand() % N + 1;
        memset(payload, 0, N);
        gen(payload, len1);
        memset(buffer, 0, PCKT_LEN);
        sendICMP(ttl, buffer, payload, N);
        clock_t start_time1 = clock();
        char msg[MAX_CHAR];
        int msglen;
        socklen_t raddr_len = sizeof(saddr_raw);
        while ((msglen = recvfrom(rawfd2, msg, MSG_SIZE, 0, (struct sockaddr *)&saddr_raw, &raddr_len)) <= 0)
            ;
        struct iphdr hdrip = *((struct iphdr *)msg);
        int iphdrlen = sizeof(hdrip);
        struct icmphdr hdricmp = *((struct icmphdr *)(msg + iphdrlen));
        print_ICMP(hdrip, hdricmp);
        // print_ICMP();
        clock_t end_time1 = clock();
        int len2 = rand() % N + 1;
        memset(payload, 0, N);
        gen(payload, len2);
        memset(buffer, 0, PCKT_LEN);
        sendICMP(ttl, buffer, payload, N);
        clock_t start_time2 = clock();
        raddr_len = sizeof(saddr_raw);
        while ((msglen = recvfrom(rawfd2, msg, MSG_SIZE, 0, (struct sockaddr *)&saddr_raw, &raddr_len)) <= 0)
            ;
        // print_ICMP();
        struct iphdr hdrip1 = *((struct iphdr *)msg);
        int iphdrlen1 = sizeof(hdrip);
        struct icmphdr hdricmp1 = *((struct icmphdr *)(msg + iphdrlen));
        print_ICMP(hdrip1, hdricmp1);
        clock_t end_time2 = clock();
        double val1 = 1000.0 * ((double)(end_time1 - start_time1) / (double)CLOCKS_PER_SEC);
        double val2 = 1000.0 * ((double)(end_time2 - start_time2) / (double)CLOCKS_PER_SEC);
        double val3 = 2.0 * abs(len2 + len1);
        time += val3 / (val1 + val2);
        sleep(time_probe);
    }
    time /= probe;
    return time;
}
double computeLatency(int ttl, char buffer[], char payload[], double last_laten)
{
    // Calculating Latency
    // Sending for Latency a Message of Size Zero
    double time = 0;
    char msg[MAX_CHAR];
    int raddr_len, msglen;
    for (int i = 0; i < probe; i++)
    {
        memset(payload, 0, N);
        memset(buffer, 0, PCKT_LEN);
        gen(payload, 0);
        sendICMP(ttl, buffer, payload, 0);
        clock_t start_time_laten = clock();
        raddr_len = sizeof(saddr_raw);
        while ((msglen = recvfrom(rawfd2, msg, MSG_SIZE, 0, (struct sockaddr *)&saddr_raw, &raddr_len)) <= 0)
            ;
        struct iphdr hdrip = *((struct iphdr *)msg);
        int iphdrlen = sizeof(hdrip);
        struct icmphdr hdricmp = *((struct icmphdr *)(msg + iphdrlen));
        print_ICMP(hdrip, hdricmp);
        // print_ICMP();
        clock_t end_time_laten = clock();
        time += 1000 * (((double)(end_time_laten - start_time_laten)) / CLOCKS_PER_SEC);
        sleep(time_probe);
    }
    time /= probe;
    return time;
}

int main(int argc, char *argv[])
{
    srand(100);
    // open the output file in write mode
    fp = fopen(output_file, "w");

    if (fp == NULL)
    {
        printf("Error opening file!\n");
        return 1;
    }
    int *p = (int *)malloc(sizeof(int *));
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

    probe = *p;

    if (!isNumber(argv[3], p))
    {
        ERROR("Could not resolve the time difference between any two probes (T)");
        exit(EXIT_FAILURE);
    }

    time_probe = *p;
    networkICMP(argv);

    char trim[100];
    memset(trim, 0, 100);
    char star[57];
    memset(star, '*', 56);

    int ttl = 1, timeout = TIMEOUT, is_send = 1, times = 0;
    double bd = 0.0, ln = 0.0;
    fd_set readSockSet;
    clock_t start_time;

    SUCCESS("PingNetInfo to %s (%s), %d hops max, %d byte packets", argv[1], ipaddr, MAX_HOP, N);
    INFO("TTL\tIPv4 Address\tResponse_Time\tLatency\t\tBandwidth");

    while (1)
    {
        if (ttl >= MAX_HOP)
            break;
        char buffer[PCKT_LEN];
        ip = (struct iphdr *)buffer;
        udp = (struct udphdr *)(buffer + sizeof(struct iphdr));
        if (is_send)
        {
            /* 4. generate Payload */
            times++;
            gen(payload, N);
            memset(buffer, 0, PCKT_LEN);
            sendICMP(ttl, buffer, payload, N);
            start_time = clock();
        }
        /* 7. Wait on select call */
        FD_ZERO(&readSockSet);
        FD_SET(rawfd2, &readSockSet);
        // Declare and initialize the pollfd structure
        struct pollfd pollfd_arr[2];
        pollfd_arr[0].fd = rawfd2;
        pollfd_arr[0].events = POLLIN;

        // Set the timeout value in milliseconds
        int timeout_ms = timeout * 1000;
        // Wait for I/O activity using poll()
        int ret = poll(pollfd_arr, 1, timeout_ms);
        if (ret == -1)
        {
            perror("poll()\n");
            close(rawfd1);
            close(rawfd2);
            fclose(fp);
            exit(EXIT_FAILURE);
        }
        else if (ret)
        {
            // ICMP
            if (FD_ISSET(rawfd2, &readSockSet))
            {
                /* 8. Read the ICMP Message */
                char msg[MAX_CHAR];
                int msglen;
                socklen_t raddr_len = sizeof(saddr_raw);
                msglen = recvfrom(rawfd2, msg, MSG_SIZE, 0, (struct sockaddr *)&saddr_raw, &raddr_len);
                struct iphdr hdrip = *((struct iphdr *)msg);
                int iphdrlen = sizeof(hdrip);
                struct icmphdr hdricmp = *((struct icmphdr *)(msg + iphdrlen));
                print_ICMP(hdrip, hdricmp);
                clock_t end_time = clock();
                if (msglen <= 0)
                {
                    timeout = TIMEOUT;
                    is_send = 1;
                    continue;
                }
                struct iphdr hdrip1 = *((struct iphdr *)msg);
                int iphdrlen1 = sizeof(hdrip);
                struct icmphdr hdricmp1 = *((struct icmphdr *)(msg + iphdrlen));
                print_ICMP(hdrip1, hdricmp1);
                /* 9. Handle Different Case */
                // read the destination IP
                struct in_addr saddr_ip;
                saddr_ip.s_addr = hdrip.saddr;
                if (hdrip.protocol == 1) // ICMP
                {
                    if (hdricmp.type == 3)
                    {
                        // verify
                        if (FD_ISSET(rawfd2, &readSockSet))
                        {
                            ln = computeLatency(ttl, buffer, payload, ln);
                            bd = computeBandWidth(ttl, buffer, payload, bd);
                            if (hdrip.saddr == ip->daddr)
                            {
                                printf("%d\t%s\t%.3f ms\t%.3f ms\t%.3f mbps\n", ttl, inet_ntoa(saddr_ip), (float)(end_time - start_time) / CLOCKS_PER_SEC * 1000, ln, bd);
                            }
                            close(rawfd1);
                            close(rawfd2);
                            fclose(fp);
                            exit(EXIT_SUCCESS);
                        }
                    }
                    else if (hdricmp.type == 11)
                    {
                        // time exceed
                        ln = computeLatency(ttl, buffer, payload, ln);
                        bd = computeBandWidth(ttl, buffer, payload, bd);
                        printf("%d\t%s\t%.3f ms\t%.3f ms\t%.3f mbps\n", ttl, inet_ntoa(saddr_ip), (float)(end_time - start_time) / CLOCKS_PER_SEC * 1000, ln, bd);
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
                    if (timeout >= WAIT_D)
                        continue;
                    else
                    {
                        if (times > 3)
                        {
                            memset(trim, 0, 100);
                            sprintf(trim, "%d\t%s", ttl, star);
                            printf("%s\n", trim);
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
            // timeout
            if (times > 3)
            {
                memset(trim, 0, 100);
                sprintf(trim, "%d\t%s", ttl, star);
                printf("%s\n", trim);
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
    fclose(fp);
    exit(EXIT_SUCCESS);
}
