#include "ft_ping_bonus.h"

t_parsed_packet *parse_packet(int sockfd, int identifier)
{
    t_parsed_packet     *packet;
    struct sockaddr_in  recv_addr;
    struct timeval      timeout;
    struct icmphdr      *orig_icmp_header;
    char                *buffer;
    socklen_t           addr_len;
    struct ip           *orig_ip_header;
    int                 orig_ip_header_length;

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    addr_len = sizeof(recv_addr);
    packet = malloc(sizeof(struct parsed_packet));
    if (!packet){
        return NULL;
    }
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    while (true)
    {
        buffer = malloc(PAYLOAD_SIZE + sizeof(struct ip));
        if (!buffer){
            return NULL;
        }
        gettimeofday(&packet->tv_start, NULL);
        packet->bytes_received = recvfrom(sockfd, buffer, PAYLOAD_SIZE + sizeof(struct ip), 0,
                                    (struct sockaddr*)&recv_addr, &addr_len);
        packet->ip_header = (struct ip*)buffer;
        packet->ip_header_length = packet->ip_header->ip_hl * 4;
        packet->icmp = (struct icmphdr*)(buffer + packet->ip_header_length);
        if (packet->icmp->type == ICMP_ECHOREPLY && htons(packet->icmp->un.echo.id) == identifier) {
           break;
        }
        else if (packet->icmp->type != ICMP_ECHO)
        {
            orig_ip_header = (struct ip*)(buffer + packet->ip_header_length + sizeof(struct icmphdr));
            orig_ip_header_length = orig_ip_header->ip_hl * 4;
            orig_icmp_header = (struct icmphdr*)((char*)orig_ip_header + orig_ip_header_length);
            if (htons(orig_icmp_header->un.echo.id) == identifier){
                break;
            }
        }
        free(buffer);
    }
    gettimeofday(&packet->tv_end, NULL);
    return packet;
}

void display_verbose_info(struct ip *ip_hdr, struct icmphdr *icmp_hdr) {
    unsigned int    i;
    size_t          size;
    char            *src_ip;
    char            *dst_ip;

    i = 0;
    src_ip = malloc(INET_ADDRSTRLEN);
    dst_ip = malloc(INET_ADDRSTRLEN);
    size = sizeof(struct ip);
    printf("IP Hdr Dump:\n");
    unsigned char *ip_header_raw = (unsigned char *)ip_hdr;
    while (i < size)
    {
        printf("%02x ", ip_header_raw[i]);
        if ((i + 1) % 4 == 0) {
            printf(" ");
        }
        i++;
    }
    printf("\n");
    printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src       Dst        Data\n");
    printf(" %1x  %1x  %02x %04x %04x   %1x %04x   %02x   %02x %04x   ",
           ip_hdr->ip_v,
           ip_hdr->ip_hl,
           ip_hdr->ip_tos,
           ntohs(ip_hdr->ip_len),
           ntohs(ip_hdr->ip_id),
           (ntohs(ip_hdr->ip_off) >> 13) & 0x7,
           ntohs(ip_hdr->ip_off) & 0x1FFF,
           ip_hdr->ip_ttl,
           ip_hdr->ip_p,
           ntohs(ip_hdr->ip_sum));
    inet_ntop(AF_INET, &ip_hdr->ip_src, src_ip, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &ip_hdr->ip_dst, dst_ip, INET_ADDRSTRLEN);
    printf("%s   %s\n", src_ip, dst_ip);
    printf("ICMP: type %d, code %d, size %ld, id 0x%x, seq 0x%x\n",
           icmp_hdr->type,
           icmp_hdr->code,
           ntohs(ip_hdr->ip_len) - sizeof(struct ip),
           ntohs(icmp_hdr->un.echo.id),
           ntohs(icmp_hdr->un.echo.sequence));
    free(src_ip);
    free(dst_ip);
}


void handle_packet(t_parsed_packet *packet) {
    switch (packet->icmp->type) {
        case ICMP_DEST_UNREACH:
            printf("Destination Unreachable from %s, Code: %d\n",
                inet_ntoa(*(struct in_addr *)&packet->ip_header->ip_dst),
                packet->icmp->code);
            break;
        case ICMP_TIME_EXCEEDED:
            printf("%ld bytes from %s : Time to live exceeded\n",
                packet->bytes_received, inet_ntoa(*(struct in_addr *)&packet->ip_header->ip_src));
            break;
        case ICMP_REDIRECT:
            printf("Redirect Message %s\n",
                inet_ntoa(*(struct in_addr *)&packet->ip_header->ip_dst));
            break;
        case ICMP_PARAMPROB:
            printf("Parameter Problem: Bad IP header %s\n",
                inet_ntoa(*(struct in_addr *)&packet->ip_header->ip_dst));
            break;
        default:
            printf("Unexpected ICMP Packet: Type=%d, Code=%d\n",
                packet->icmp->type, packet->icmp->code);
    }
}

float receive_ping(int sockfd, t_args *args) {
    t_parsed_packet *packet;
    float            rtt;

    packet = parse_packet(sockfd, args->identifier);
    if (packet->icmp->type) {
        handle_packet(packet);
    }
    if (args->option == FLAG_VERBOSE && packet->icmp->type){
        display_verbose_info(packet->ip_header, packet->icmp);
    }
    if (packet->bytes_received < 0) {
        if ((errno == EAGAIN || errno == EWOULDBLOCK) && args->option == FLAG_VERBOSE) {
            printf("Request timeout for icmp_seq %d\n", args->packets_sent);
        }
        free(packet->ip_header);
        free(packet);
        return -1;
    }
    if (packet->icmp->type == ICMP_ECHOREPLY) {
        rtt = (packet->tv_end.tv_sec - packet->tv_start.tv_sec) * 1000.0 +
                   (packet->tv_end.tv_usec - packet->tv_start.tv_usec) / 1000.0;
        printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
               packet->bytes_received - packet->ip_header_length,
               args->ip,
               ntohs(packet->icmp->un.echo.sequence),
               packet->ip_header->ip_ttl,
               rtt);
        free(packet->ip_header);
        free(packet);
        return rtt;
    }
    free(packet->ip_header);
    free(packet);
    return -1;
}
