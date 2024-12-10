#include "ft_ping.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>


#define DATA_SIZE 56
#define PING_PKT_SIZE 64
#define PORT_NO 0
#define PING_SLEEP_RATE 1000000
#define RECV_TIMEOUT 1

uint16_t expected_id;
uint16_t expected_seq;

int ping_count = 0;
int packets_sent = 0;
int packets_received = 0;
float min_rtt = 999999;
float max_rtt = 0;
float total_rtt = 0;
volatile sig_atomic_t keep_running = 1;

struct  icmp_header {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t identifier;
    uint16_t sequence;
};

uint16_t    calculate_checksum(void *buffer, int length) {
    unsigned long   sum;
    uint16_t        *data;
    int             index;

    sum = 0;
    data = (uint16_t *)buffer;
    index = 0;
    while (index < (length / 2))
    {
        sum += data[index];
        index++;
    }
    if (length % 2 == 1) {
        sum += ((uint8_t *)buffer)[length - 1];
    }
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    return ~sum;
}

void    interrupt_handler(int signal) {
    if (signal == 2){
        keep_running = 0;
    }
}

void    fill_data( char *buffer, int size ){
    int i;

    i = 0;
    while (i < size)
    {
        buffer[i] = i;
        i++;
    }
}

int send_ping(int sockfd, struct sockaddr_in *dest_addr, int seq_no) {
    struct  icmp_header *icmp;
    char    *packet;
    int     result;

    packet = malloc(PING_PKT_SIZE);
    icmp = (struct icmp_header *)packet;
    memset(packet, 0, PING_PKT_SIZE);
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->identifier = htons(getpid() & 0xFFFF);
    icmp->sequence = htons(seq_no);
    fill_data(packet + sizeof(struct icmp_header), DATA_SIZE);
    icmp->checksum = calculate_checksum((uint16_t *)icmp, PING_PKT_SIZE);
    result = sendto(sockfd, packet, PING_PKT_SIZE, 0,
                 (struct sockaddr *)dest_addr, sizeof(*dest_addr));
    free(packet);
    return result;
}

typedef struct  parsed_packet {
    struct ip *ip_header;
    struct icmp_header *icmp;
    ssize_t bytes_received;
    int ip_header_length;
    struct timeval tv_start, tv_end;
}t_parsed_packet;


t_parsed_packet *parse_packet(int sockfd)
{
    t_parsed_packet    *packet;
    struct sockaddr_in  recv_addr;
    char                *buffer;
    socklen_t           addr_len;

    addr_len = sizeof(recv_addr);
    packet = malloc(sizeof(struct parsed_packet));
    if (!packet){
        return NULL;
    }
    packet->icmp = malloc(sizeof(struct icmp_header));
    packet->ip_header = malloc(sizeof(struct ip));
    if (!packet->icmp || !packet->ip_header){
        return NULL;
    }
    buffer = malloc(PING_PKT_SIZE + sizeof(struct ip));
    gettimeofday(&packet->tv_start, NULL);
     packet->bytes_received = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                                    (struct sockaddr*)&recv_addr, &addr_len);
    gettimeofday(&packet->tv_end, NULL);
    packet->ip_header = (struct ip*)buffer;
    packet->ip_header_length = packet->ip_header->ip_hl * 4;
    packet->icmp = (struct icmp_header*)(buffer + packet->ip_header_length);
    return packet;
}

float receive_ping(int sockfd, char *addr_str) {
    t_parsed_packet *packet;
    float            rtt;

    packet = parse_packet(sockfd);
    if (packet->bytes_received < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            printf("Request timeout for icmp_seq %d\n", packets_sent);
            return -1;
        }
        perror("recvfrom failed");
        return -1;
    }
    if (packet->icmp->type == ICMP_ECHOREPLY) {
        rtt = (packet->tv_end.tv_sec - packet->tv_start.tv_sec) * 1000.0 +
                   (packet->tv_end.tv_usec - packet->tv_start.tv_usec) / 1000.0;

        printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.1f ms\n",
               packet->bytes_received - packet->ip_header_length,
               addr_str,
               ntohs(packet->icmp->sequence),
               packet->ip_header->ip_ttl,
               rtt);
        return rtt;
    }

    return -1;
}


int main(int argc, char *argv[]) {
    t_args *args;
    int sockfd;
    struct sockaddr_in dest_addr;

    args = get_new_args();
    check_args(argv, args);
    if (argc != 2) {
        printf("Usage: %s <destination_ip>\n", argv[0]);
        return 1;
    }
    if (getuid() != 0) {
        printf("This program requires root privileges. Please run with sudo.\n");
        return 1;
    }
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("socket creation failed");
        return 1;
    }
    struct timeval tv_timeout;
    tv_timeout.tv_sec = RECV_TIMEOUT;
    tv_timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
               &tv_timeout, sizeof(tv_timeout));
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, args->ip, &dest_addr.sin_addr) <= 0) {
        printf("Invalid IP address\n");
        close(sockfd);
        return 1;
    }
    signal(SIGINT, interrupt_handler);
    printf("PING %s (%s)\n", args->hostname, args->ip);
    while (keep_running) {
        packets_sent++;
        if (send_ping(sockfd, &dest_addr, packets_sent) < 0) {
            perror("Failed to send ping");
            continue;
        }
       float rtt = receive_ping(sockfd, args->ip);
       if (rtt >= 0) {
           packets_received++;
           total_rtt += rtt;
           if (rtt < min_rtt) min_rtt = rtt;
           if (rtt > max_rtt) max_rtt = rtt;
       }

        usleep(PING_SLEEP_RATE * 5);
    }
    printf("\n--- %s ping statistics ---\n", args->ip);
    printf("%d packets transmitted, %d received, %.1f%% packet loss\n",
           packets_sent,
           packets_received,
           100.0 * (packets_sent - packets_received) / packets_sent);
    if (packets_received > 0) {
        printf("rtt min/avg/max = %.3f/%.3f/%.3f ms\n",
               min_rtt,
               total_rtt / packets_received,
               max_rtt);
    }
    close(sockfd);
    return 0;
}