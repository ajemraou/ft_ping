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

void    fill_data( char *buffer, int size ){
    int i;

    i = 0;
    while (i < size)
    {
        buffer[i] = i;
        i++;
    }
}

int send_ping(int sockfd, struct sockaddr_in *dest_addr, int seq_no) {
    t_icmp *icmp;
    char    *packet;
    int     result;

    packet = malloc(PING_PKT_SIZE);
    icmp = (struct icmp_header *)packet;
    memset(packet, 0, PING_PKT_SIZE);
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->identifier = htons(getpid() & 0xFFFF);
    icmp->sequence = htons(seq_no);
    fill_data(packet + sizeof(struct icmp_header), DATA_SIZE);
    icmp->checksum = calculate_checksum((uint16_t *)icmp, PING_PKT_SIZE);
    result = sendto(sockfd, packet, PING_PKT_SIZE, 0,
                 (struct sockaddr *)dest_addr, sizeof(*dest_addr));
    free(packet);
    return result;
}