#include "ft_ping_bonus.h"

int main(int argc, char *argv[]) {
    t_args *args;
    int sockfd;
    struct sockaddr_in dest_addr;

    args = get_new_args();
    parse_flags(argc, argv, args);
    printf("hostname        : %s\n", args->hostname);
    printf("ip              : %s\n", args->ip);
    printf("invalid_arg     : %s\n", args->invalid_arg);
    printf("option          : %d\n", args->option);
    printf("packet_sent     : %d\n", args->packets_sent);
    printf("Parsed Options  :\n");
    printf("Interval        : %d seconds\n", args->interval);
    printf("TTL             : %d\n", args->ttl);
    printf("Timeout         : %d seconds\n", args->timeout);
    printf("Reply Timeout   : %.1f seconds\n", args->reply_timeout);
    printf("Count           : %d\n", args->count == -1 ? 0 : args->count);
    return (0);
    if (argc < 2) {
        printf("Usage: %s <destination_ip>\n", argv[0]);
        return 1;
    }
    if (getuid() != 0) {
        printf("This program requires root privileges. Please run with sudo.\n");
        return 1;
    }
    if (args->invalid_arg){
        printf("ft_ping: unknown host: %s\n", args->invalid_arg);
        return 1;
    }
    sockfd = socket_setup(args, &dest_addr);
    signal(SIGINT, interrupt_handler);
    printf("FT_PING %s (%s)\n", args->hostname, args->ip);
    if (args->option == FLAG_VERBOSE){
        printf("ping: sock4.fd: %d (socktype: SOCK_RAW), hints.ai_family: AF_INET\n\n", sockfd);
    }
    ft_ping(args, sockfd, &dest_addr);
    close(sockfd);
    return 0;
}
