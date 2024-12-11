#ifndef FT_PING_BONUS_H
#define FT_PING_BONUS_H

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

#define DATA_SIZE 56
#define PING_PKT_SIZE 64
#define PING_SLEEP_RATE 1000000
#define RECV_TIMEOUT 1

enum options {
    HELP = 1,
    VERBOSE = 2,
    DOMAIN = 3,
};

typedef struct arguments {
    enum options    option;
    char            *hostname;
    char            *ip;
    char            *invalid_arg;
    int             packets_sent;
}t_args;

typedef struct  icmp_header {
    uint8_t     type;
    uint8_t     code;
    uint16_t    checksum;
    uint16_t    identifier;
    uint16_t    sequence;
}t_icmp;

typedef struct  parsed_packet {
    struct icmp_header  *icmp;
    struct timeval      tv_start, tv_end;
    struct ip           *ip_header;
    ssize_t             bytes_received;
    int                 ip_header_length;
}t_parsed_packet;

typedef struct statistics{
    int     packets_received;
    float   min_rtt;
    float   max_rtt;
    float   total_rtt;
}t_statis;

/***  Parsing */
void    check_args(char **argv, t_args *args);
t_args *get_new_args();
/***------- send ping --------------------*/
int     send_ping(int sockfd, struct sockaddr_in *dest_addr, int seq_no);
/***------- recieve ping -----------------*/
float   receive_ping(int sockfd, t_args *args);
/***------- ping -------------------------*/
int     socket_setup( const char *ip_address, struct sockaddr_in *dest_addr );
void    interrupt_handler(int signal);
int     ft_ping( t_args *args, int sockfd, struct sockaddr_in *dest_addr );

#endif