#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/time.h>

#define PACKET_SIZE 64
#define MAX_TTL 30
#define MAX_TIMEOUT 1000

void print_usage() {
    printf("Usage: ./traceroute <destination_ip>\n");
}

unsigned short calculate_checksum(unsigned short *buffer, int length) {
    unsigned long sum = 0;
    while (length > 1) {
        sum += *buffer++;
        length -= 2;
    }
    if (length == 1) {
        sum += *(unsigned char *)buffer;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

void probe_hop(char *destination_ip, int ttl) {
    struct sockaddr_in dest_addr;
    struct timeval send_time, recv_time;
    struct icmphdr icmp_header;
    char packet_buffer[PACKET_SIZE];
    int sockfd, bytes_received;
    double rtt;
    socklen_t addrlen = sizeof(dest_addr);
    memset(&dest_addr, 0, addrlen);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(destination_ip);

    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("Error creating socket");
        exit(1);
    }
    setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

    memset(packet_buffer, 0, sizeof(packet_buffer));
    memset(&icmp_header, 0, sizeof(icmp_header));
    icmp_header.type = ICMP_ECHO;
    icmp_header.code = 0;
    icmp_header.checksum = 0;
    icmp_header.un.echo.id = htons(getpid() & 0xffff);
    icmp_header.un.echo.sequence = htons(ttl);
    memcpy(packet_buffer, &icmp_header, sizeof(icmp_header));
    icmp_header.checksum = calculate_checksum((unsigned short *)packet_buffer, sizeof(icmp_header));
    memcpy(packet_buffer, &icmp_header, sizeof(icmp_header));

    gettimeofday(&send_time, NULL);
    if (sendto(sockfd, packet_buffer, sizeof(packet_buffer), 0, (struct sockaddr *)&dest_addr, addrlen) < 0) {
        perror("Error sending packet");
        close(sockfd);
        exit(1);
    }

    memset(packet_buffer, 0, sizeof(packet_buffer));
