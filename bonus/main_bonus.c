#include "ft_ping_bonus.h"

int main(int argc, char *argv[]) {
    struct sockaddr_in  dest_addr;
    t_args              *args;
    int                 sockfd;

    args = get_new_args();
    parse_flags(argc, argv, args);
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
        printf("ping: ock4.fd: %d (socktype: SOCK_RAW), hints.ai_family: AF_INET\n\n", sockfd);
    }
    ft_ping(args, sockfd, &dest_addr);
    free(args->ip);
    free(args->hostname);
    free(args->invalid_arg);
    free(args);
    close(sockfd);
    return 0;
}
