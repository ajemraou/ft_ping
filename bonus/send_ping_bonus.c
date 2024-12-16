#include "ft_ping_bonus.h"

uint16_t    calculate_checksum(void *buffer, int length) {
    unsigned long   sum;
    uint16_t        *data;
    int             index;

    sum = 0;
    data = (uint16_t *)buffer;
    index = 0;
    while (index < (length / 2))
    {
        sum += data[index];
        index++;
    }
    if (length % 2 == 1) {
        sum += ((uint8_t *)buffer)[length - 1];
    }
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    return ~sum;
}

void    fill_payload( char *buffer, int size ){
    int i;

    i = 0;
    while (i < size)
    {
        buffer[i] = i;
        i++;
    }
}

int send_ping(int sockfd, struct sockaddr_in *dest_addr, t_args *args) {
    struct icmphdr *icmp;
    char    *packet;
    int     result;

    packet = malloc(PAYLOAD_SIZE);
    icmp = (struct icmphdr *)packet;
    memset(packet, 0, PAYLOAD_SIZE);
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->un.echo.id = htons(getpid() & 0xFFFF);
    icmp->un.echo.sequence = htons(args->packets_sent);
    fill_payload(packet + sizeof(struct icmphdr), DATA_SIZE);
    icmp->checksum = calculate_checksum((uint16_t *)icmp, sizeof(struct icmphdr) + PAYLOAD_SIZE);
    args->checksum = icmp->checksum;
    result = sendto(sockfd, packet, PAYLOAD_SIZE, 0,
                 (struct sockaddr *)dest_addr, sizeof(*dest_addr));
    free(packet);
    return result;
}