#include "ft_ping.h"

void display_help() {
    printf("Usage: ft_ping [OPTIONS] <destination>\n\n");
    printf("Ping utility to send ICMP Echo Requests to a network host.\n");
    printf("Arguments:\n");
    printf("  <destination>      The IP address or hostname to ping.\n\n");
    printf("Options:\n");
    printf("  -c <count>         Number of packets to send (default: infinite).\n");
    printf("  -t <ttl>           Set the Time-To-Live for packets (default: 64).\n");
    printf("  -i <interval>      Interval between packets in seconds (default: 1s).\n");
    printf("  -w <timeout>       Time to wait for a response, in seconds.\n");
    printf("  -W <deadline>      Timeout for the entire operation, in seconds.\n");
    printf("  -v                 Verbose output; display more detailed information.\n");
    printf("  --help             Display this help message and exit.\n\n");
    printf("Examples:\n");
    printf("  ft_ping google.com            Ping Google indefinitely.\n");
    printf("  ft_ping -c 5 -t 32 8.8.8.8    Send 5 packets to 8.8.8.8 with TTL of 32.\n");
    printf("  ft_ping -w 10 192.168.1.1     Wait up to 10 seconds for a response.\n");
    printf("  ft_ping -W 20 -c 3 1.1.1.1    Send 3 packets with a 20s deadline.\n");
    printf("  ft_ping -v example.com        Ping with detailed output.\n");
    printf("\n");
    printf("Note: This utility requires root privileges to send ICMP packets.\n");
    exit(EXIT_SUCCESS);
}

int error_handling(int argc, t_args *args, const char *s)
{
    if (argc < 2) {
        printf("Usage: %s <destination_ip>\n", s);
        return 1;
    }
    if (getuid() != 0) {
        printf("This program requires root privileges. Please run with sudo.\n");
        return 1;
    }
    if (args->invalid_arg){
        printf("ft_ping: unknown host\n");
        if (!args->ip){
            return 1;
        }
    }
    return 0;
}

void    destruct_resources(t_args *args)
{
    free(args->ip);
    free(args->hostname);
    free(args->invalid_arg);
    free(args);
}

int main(int argc, char *argv[]) {
    struct sockaddr_in  dest_addr;
    t_args              *args;
    int                 sockfd;

    args = get_new_args();
    parse_flags(argc, argv, args);
    if (args->option == FLAG_HELP){
        destruct_resources(args);
        display_help();
    }
    else if (args->option == FLAG_USAGE){
        printf("Usage: %s [-v] [-w DEADLINE] [-W TIMEOUT] [--ttl=N] [-i INTERVAL] [--usage] HOST\n", argv[0]);
        destruct_resources(args);
        return 0;
    }
    if (error_handling(argc, args, argv[0])){
        destruct_resources(args);
        return 1;
    }
    args->identifier = getpid() & 0xFFFF;
    sockfd = socket_setup(args, &dest_addr);
    signal(SIGINT, interrupt_handler);
    printf("FT_PING %s (%s)\n", args->hostname, args->ip);
    if (args->option == FLAG_VERBOSE){
        printf("ping: ock4.fd: %d (socktype: SOCK_RAW), hints.ai_family: AF_INET\n\n", sockfd);
    }
    ft_ping(args, sockfd, &dest_addr);
    destruct_resources(args);
    close(sockfd);
    return 0;
}
