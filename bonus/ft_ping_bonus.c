#include "ft_ping_bonus.h"

volatile sig_atomic_t keep_running = 1;

void    interrupt_handler(int signal) {
    if (signal == SIGINT){
        keep_running = 0;
    }
}

t_statis    *init_statistics_list()
{
    t_statis *list;

    list = malloc(sizeof(t_statis));
    list->packets_received = 0;
    list->min_rtt = 999999;
    list->max_rtt = 0;
    list->total_rtt = 0;
    return list;
}

int socket_setup( t_args *args, struct sockaddr_in *dest_addr )
{
    struct timeval  tv_timeout;
    int             sockfd;

   sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        return -1;
    }
    tv_timeout.tv_sec = args->reply_timeout;
    tv_timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
               &tv_timeout, sizeof(tv_timeout));
    memset(dest_addr, 0, sizeof(*dest_addr));
    dest_addr->sin_family = AF_INET;
    if (inet_pton(AF_INET, args->ip, &dest_addr->sin_addr) <= 0) {
        return -1;
    }
    return sockfd;
}

void    print_statistics(t_args *args, t_statis *list)
{
    printf("\n--- %s ping statistics ---\n", args->hostname);
    printf("%d packets transmitted, %d received, %.1f%% packet loss\n",
        args->packets_sent,
        list->packets_received,
        100.0 * (args->packets_sent - list->packets_received) / args->packets_sent);
    if (list->packets_received > 0) {
        printf("rtt min/avg/max = %.3f/%.3f/%.3f ms\n",
            list->min_rtt,
            list->total_rtt / list->packets_received,
            list->max_rtt);
    }
}

bool check_timeout(time_t start_time, t_args *options) {
    time_t  current_time;
    int     elapsed_time;

    if (options->timeout == 0) {
        return false;
    }
    current_time = time(NULL);
    elapsed_time = (int)(current_time - start_time);
    return elapsed_time >= options->timeout;
}

int ft_ping( t_args *args, int sockfd, struct sockaddr_in *dest_addr )
{
    t_statis    *list;
    time_t      start_time;
    float       rtt;

    start_time = time(NULL);
    list = init_statistics_list();
    while (keep_running && args->count) {
        args->packets_sent++;
        if (send_ping(sockfd, dest_addr, args->packets_sent) < 0) {
            continue;
        }
        rtt = receive_ping(sockfd, args);
        if (rtt >= 0) {
           list->packets_received++;
           list->total_rtt += rtt;
           if (rtt < list->min_rtt){
            list->min_rtt = rtt;
           }
           if (rtt > list->max_rtt){
            list->max_rtt = rtt;
           }
       }
       if (args->count > 0){
        args->count--;
       }
       else if (check_timeout(start_time, args)){
        break ;
       }
       usleep(PING_SLEEP_RATE * args->interval);
    }
    print_statistics(args, list);
    free(list);
    return 0;
}