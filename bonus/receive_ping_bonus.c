#include "ft_ping_bonus.h"

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

// IP Hdr Dump:
//  4500 0054 08dd 4000 0101 b4f4 0a00 020f d83a d78e
// Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src      Dst     Data
//  4  5  00 0054 08dd   2 0000  01  01 b4f4 10.0.2.15  216.58.215.142
// ICMP: type 8, code 0, size 64, id 0x1946, seq 0x0008
// 64 bytes from _gateway (10.13.254.254): Time to live exceeded
/**
 * Function to display verbose information for IP and ICMP headers.
 *
 * @param ip_hdr: Pointer to the parsed IP header structure.
 * @param icmp_hdr: Pointer to the parsed ICMP header structure.
 */
void display_verbose_info(struct ip *ip_hdr, struct icmp_header *icmp_hdr) {
    // Display the IP header dump (raw hex)
    printf("IP Hdr Dump:\n");
    unsigned char *ip_header_raw = (unsigned char *)ip_hdr;
    for (unsigned int i = 0; i < sizeof(struct ip); i++) {
        printf("%02x ", ip_header_raw[i]);
        if ((i + 1) % 4 == 0) {
            printf(" "); // Add spacing every 4 bytes
        }
    }
    printf("\n");

    // Display parsed IP header information
    printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src       Dst        Data\n");
    printf(" %1x  %1x  %02x %04x %04x   %1x %04x   %02x   %02x %04x   ",
           ip_hdr->ip_v,               // Version
           ip_hdr->ip_hl,              // Header length
           ip_hdr->ip_tos,             // Type of service
           ntohs(ip_hdr->ip_len),      // Total length
           ntohs(ip_hdr->ip_id),       // Identification
           (ntohs(ip_hdr->ip_off) >> 13) & 0x7, // Flags
           ntohs(ip_hdr->ip_off) & 0x1FFF, // Fragment offset
           ip_hdr->ip_ttl,             // Time-to-live
           ip_hdr->ip_p,               // Protocol (1 = ICMP)
           ntohs(ip_hdr->ip_sum));     // Header checksum

    char src_ip[INET_ADDRSTRLEN], dst_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ip_hdr->ip_src, src_ip, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &ip_hdr->ip_dst, dst_ip, INET_ADDRSTRLEN);

    printf("%s   %s\n", src_ip, dst_ip);

    // Display ICMP header information
    printf("ICMP: type %d, code %d, size %ld, id 0x%x, seq 0x%x\n",
           icmp_hdr->type,             // ICMP type
           icmp_hdr->code,             // ICMP code
           ntohs(ip_hdr->ip_len) - sizeof(struct ip), // ICMP size
           ntohs(icmp_hdr->identifier),     // ICMP ID
           ntohs(icmp_hdr->sequence)); // ICMP sequence number
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
    packet = parse_packet(sockfd);
    if (packet->icmp->type) {
        handle_packet(packet);
    }
    if (args->option == FLAG_VERBOSE){
        display_verbose_info(packet->ip_header, packet->icmp);
    }
    if (packet->bytes_received < 0) {
        if ((errno == EAGAIN || errno == EWOULDBLOCK) && args->option == FLAG_VERBOSE) {
            printf("Request timeout for icmp_seq %d\n", args->packets_sent);
        }
        return -1;
    }
    if (packet->icmp->type == ICMP_ECHOREPLY) {
        rtt = (packet->tv_end.tv_sec - packet->tv_start.tv_sec) * 1000.0 +
                   (packet->tv_end.tv_usec - packet->tv_start.tv_usec) / 1000.0;

        printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.1f ms\n",
               packet->bytes_received - packet->ip_header_length,
               args->ip,
               ntohs(packet->icmp->sequence),
               packet->ip_header->ip_ttl,
               rtt);
        return rtt;
    }

    return -1;
}
