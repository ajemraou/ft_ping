#include "ft_ping.h"

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
    buffer = malloc(PING_PKT_SIZE + sizeof(struct ip));
    if (!packet->icmp || !packet->ip_header || !buffer){
        return NULL;
    }
    gettimeofday(&packet->tv_start, NULL);
     packet->bytes_received = recvfrom(sockfd, buffer, PING_PKT_SIZE + sizeof(struct ip), 0,
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
            printf("Request timeout for icmp_seq \n"); // packets_sent variable here ???
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
