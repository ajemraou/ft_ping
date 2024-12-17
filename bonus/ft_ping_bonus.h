#ifndef FT_PING_BONUS_H
#define FT_PING_BONUS_H

#include <netinet/in.h>
#include <stdint.h>
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

#define DATA_SIZE 56            // Defines the size of the data payload in the ICMP Echo Request.
#define PAYLOAD_SIZE 64         // Defines the total size of the ICMP packet, including both the header and payload.
#define PING_SLEEP_RATE 1000000 // Defines the sleep rate between sending ICMP packets, measured in microseconds.
#define RECV_TIMEOUT 1          // Defines the timeout for receiving ICMP Echo Replies, measured in seconds.
#define PING_PRECISION 1000     // Millisecond precision
#define MAX_INTERVAL 3600       // Maximum interval for -i (1 hour)
#define MIN_REPLY_TIMEOUT 0.2   // Minimum reply timeout for -W (200ms)
#define MAX_TTL 255             // Maximum TTL for --ttl (1 to 255)


/***
 * Represents flags used in the ping program.
 *
 */
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



/***
 * Holds all user-defined options and runtime statistics for the ping program.
 *
 */
typedef struct arguments {
    PingFlags   option;         // Enum representing the parsed flags.
    char        *hostname;      // The hostname specified by the user.
    char        *ip;            // The resolved IP address of the hostname.
    char        *invalid_arg;   // Stores invalid arguments for error reporting.
    int         interval;       // Interval between sending packets (-i)
    int         ttl;            // Time-To-Live (--ttl)
    int         timeout;        // Total timeout for the program (-w)
    float       reply_timeout;  // Reply timeout for each packet (-W)
    int         count;          // Number of packets to send (-c)
    int         packets_sent;   // The total number of packets sent during execution.
    uint16_t    identifier;     // Process ID as unique identifier
}t_args;


/***
 * Holds information about a parsed ICMP packet, including its headers, timestamps, and additional metadata.
 *
 */
typedef struct  parsed_packet {
    struct icmphdr      *icmp;              // Pointer to the parsed ICMP header structure.
    struct timeval      tv_start, tv_end;   // Timestamp when the ICMP Echo Request was sent/received.
    struct ip           *ip_header;         // Pointer to the parsed IP header (struct ip) containing details about the source, destination, and routing.
    ssize_t             bytes_received;     // Number of bytes received in the packet.
    int                 ip_header_length;   // Length of the IP header in bytes.
}t_parsed_packet;


/***
 * Stores runtime statistics for the ping program, including packet counts and round-trip times (RTTs).
 *
 */
typedef struct statistics{
    int     packets_received; // Total number of ICMP Echo Replies successfully received.
    float   min_rtt;          // Minimum round-trip time (RTT) observed.
    float   max_rtt;          // Maximum round-trip time (RTT) observed.
    float   total_rtt;        // Accumulated RTTs, used for calculating the average RTT.
}t_statis;


/**
 * Initializes and returns a new t_args structure with default values for all attributes.
 *
 * @return
 * - Pointer to a newly allocated t_args structure with default values:
 * @behavior
 * - Allocates memory for a new t_args structure.
 * - Ensures all attributes are set to appropriate defaults.
 */
t_args *get_new_args();


/**
 * Parses the command-line arguments (argv) and stores the information in the t_args structure.
 *
 * @param argc: The number of arguments passed to the program.
 * @param argv: The array of argument strings.
 * @param options: Pointer to a t_args structure where parsed values will be stored.
 *
 * @behavior
 * - Parses options such as -i, --ttl, -w, -W, and -c.
 * - Validates the inputs and stores them in the t_args structure.
 */
void parse_flags(int argc, char *argv[], t_args *options);


/**
 * Sends an ICMP Echo Request to the specified destination.
 *
 * @param sockfd: The socket file descriptor.
 * @param dest_addr: Pointer to the destination address structure (struct sockaddr_in).
 * @param args: Pointer to a t_args structure containing user-defined options (e.g., TTL, timeout).
 * @return
 * - 0 on success.
 * - Non-zero value if the packet could not be sent.
 * @behavior
 * - Constructs an ICMP Echo Request packet.
 * - Uses sendto() to send the packet to the destination.
 */
int     send_ping(int sockfd, struct sockaddr_in *dest_addr, t_args *args);



/**
 * Receives and processes incoming ICMP packets.
 *
 * @param sockfd: The socket file descriptor.
 * @param args: Pointer to a t_args structure where received packet information will be stored.
 *
 * @return
 * - Round-trip time (RTT) of the packet as a float in milliseconds.
 * - Negative value if the packet is invalid or if no packet is received within the timeout.
 *
 * @behavior
 * - Uses recvfrom() to receive incoming packets.
 * - Parses the IP and ICMP headers.
 * - Validates the ICMP Echo Reply and calculates the RTT.
 */
float   receive_ping(int sockfd, t_args *args);


/**
 * Creates and configures a socket for sending ICMP packets.
 *
 * @param args: Pointer to a t_args structure containing user-defined options (e.g., TTL, timeout).
 * @param dest_addr: Pointer to a struct sockaddr_in structure to store the destination address.
 *
 * @return
 * - The socket file descriptor on success.
 * - -1 if the socket could not be created or configured.
 *
 * @behavior
 * - Creates a raw socket using socket(AF_INET, SOCK_RAW, IPPROTO_ICMP).
 * - Configures the socket options, such as TTL and timeouts, based on values in t_args.
 */
int     socket_setup( t_args *args, struct sockaddr_in *dest_addr );


/**
 * Handles the SIGINT signal (Ctrl+C).
 *
 * @param signal: The signal number (typically SIGINT).
 *
 * @behavior
 * - Stops the program gracefully when the user interrupts it (Ctrl+C).
 */
void    interrupt_handler(int signal);

/**
 * Handles the main logic for sending ICMP Echo Requests and receiving ICMP Echo Replies.
 *
 * @param args: Pointer to a t_args structure that contains user-defined options and runtime statistics (e.g., interval, TTL, timeout).
 * @param sockfd: The raw socket file descriptor used for communication.
 * @param dest_addr: Pointer to a struct sockaddr_in structure containing the destination address.
 *
 * @return
 * - 0 on successful execution (all packets sent and received as expected).
 * - Non-zero value if an error occurs during execution.
 *
 * @behavior
 * - Creates a raw socket using socket(AF_INET, SOCK_RAW, IPPROTO_ICMP).
 * - Configures the socket options, such as TTL and timeouts, based on values in t_args.
 */
int     ft_ping( t_args *args, int sockfd, struct sockaddr_in *dest_addr );

#endif