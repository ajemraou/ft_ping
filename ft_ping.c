#include "ft_ping.h"

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

void    interrupt_handler(int signal) {
    if (signal == 2){
        keep_running = 0;
    }
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

        usleep(PING_SLEEP_RATE);
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