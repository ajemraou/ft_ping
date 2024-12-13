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
#include <limits.h>
#include <float.h>
#include <time.h>

#define DATA_SIZE 56
#define PING_PKT_SIZE 64
#define PING_SLEEP_RATE 1000000
#define RECV_TIMEOUT 1
#define PING_PRECISION 1000     /* Millisecond precision */
#define MAX_INTERVAL 3600      // Maximum interval for -i (1 hour)
#define MIN_REPLY_TIMEOUT 0.2  // Minimum reply timeout for -W (200ms)
#define MAX_TTL 255            // Maximum TTL for --ttl (1 to 255)

typedef enum {
    FLAG_USAGE,           // For --usage: Display usage information
    FLAG_HELP,            // For --help: Display help information
    FLAG_VERBOSE,         // For -v: Verbose mode
    FLAG_DOMAIN,          // For Domain name
    FLAG_INTERVAL,        // For -i: Interval between packets
    FLAG_TTL,             // For --ttl: Time-To-Live
    FLAG_TIMEOUT,         // For -w: Total timeout for the program
    FLAG_REPLY_TIMEOUT,   // For -W: Timeout for each packet reply
    FLAG_COUNT,           // For -c: Number of packets to send
} PingFlags;

typedef struct arguments {
    PingFlags   option;
    char        *hostname;
    char        *ip;
    char        *invalid_arg;
    int         interval;       // Interval between sending packets (-i)
    int         ttl;            // Time-To-Live (--ttl)
    int         timeout;        // Total timeout for the program (-w)
    float       reply_timeout; // Reply timeout for each packet (-W)
    int         count;          // Number of packets to send (-c)
    int         packets_sent;
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
void parse_flags(int argc, char *argv[], t_args *options);
t_args *get_new_args();
/***------- send ping --------------------*/
int     send_ping(int sockfd, struct sockaddr_in *dest_addr, int seq_no);
/***------- recieve ping -----------------*/
float   receive_ping(int sockfd, t_args *args);
/***------- ping -------------------------*/
int     socket_setup( t_args *args, struct sockaddr_in *dest_addr );
void    interrupt_handler(int signal);
int     ft_ping( t_args *args, int sockfd, struct sockaddr_in *dest_addr );

#endif