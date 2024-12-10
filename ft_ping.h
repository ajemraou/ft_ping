#ifndef FT_PING_H
#define FT_PING_H

#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>


#define PING_PKT_SIZE 64
#define DATA_SIZE 56

// int packets_sent = 0;
enum options {
    HELP = 1,
    VERBOSE = 2,
    DOMAIN = 3,
  };

typedef struct arguments {
    char *hostname;
    char *ip;
    enum options option;
    char *invalid_arg;
} t_args;

typedef struct  icmp_header {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t identifier;
    uint16_t sequence;
}t_icmp;

typedef struct  parsed_packet {
    struct ip *ip_header;
    struct icmp_header *icmp;
    ssize_t bytes_received;
    int ip_header_length;
    struct timeval tv_start, tv_end;
}t_parsed_packet;

void  check_args(char **argv, t_args *args);
t_args *get_new_args();
// ------- send ping --------------------//
int send_ping(int sockfd, struct sockaddr_in *dest_addr, int seq_no);
// ------- recieve ping ----------------//
float receive_ping(int sockfd, char *addr_str);

#endif