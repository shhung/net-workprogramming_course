#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <net/ethernet.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <time.h>
#include <pcap.h>
#include "handler.h"

int pair_cnt = 0;

char *mac_ntoa(u_char *d) {
    static char str[MAC_ADDLEN];

    snprintf(str, sizeof(str), "%02x:%02x:%02x:%02x:%02x:%02x", d[0], d[1], d[2], d[3], d[4], d[5]);

    return str;
}

char *ip_ttoa(u_int8_t flag) {
    static int f[] = {'1', '1', '1', 'D', 'T', 'R', 'C', 'X'};
#define TOS_MAX (sizeof(f)/sizeof(f[0]))
    static char str[TOS_MAX + 1];
    u_int8_t mask = 1 << 7;
    int i;

    for(i = 0 ; i < TOS_MAX ; i++) {
        if(mask & flag)
            str[i] = f[i];
        else
            str[i] = '-';
        mask >>= 1;
    }
    str[i] = 0;

    return str;
}

char *ip_ftoa(u_int16_t flag) {
    static int f[] = {'R', 'D', 'M'};
#define IP_FLG_MAX (sizeof(f)/sizeof(f[0]))
    static char str[IP_FLG_MAX + 1];
    u_int16_t mask = 1 << 15;
    int i;

    for(i = 0 ; i < IP_FLG_MAX ; i++) {
        if(mask & flag)
            str[i] = f[i];
        else
            str[i] = '-';
        mask >>= 1;
    }
    str[i] = 0;

    return str;
}

void dump_tcp(u_int32_t length, const u_char *content) {
	struct ip *ip = (struct ip *)(content + ETHER_HDR_LEN);
    struct tcphdr *tcp = (struct tcphdr *)(content + ETHER_HDR_LEN + (ip->ip_hl << 2));
    // determine endianness
	u_int16_t source_port = ntohs(tcp->th_sport);
    u_int16_t destination_port = ntohs(tcp->th_dport);
	printf("Protocol: TCP\n");
    printf("+-------------------------+-------------------------+\n");
    printf("| Source Port:       %5u| Destination Port:  %5u|\n", source_port, destination_port);
    printf("+-------------------------+-------------------------+\n");
}

void dump_udp(u_int32_t length, const u_char *content) {
    struct ip *ip = (struct ip *)(content + ETHER_HDR_LEN);
    struct udphdr *udp = (struct udphdr *)(content + ETHER_HDR_LEN + (ip->ip_hl << 2));
    // determine endianness
    u_int16_t source_port = ntohs(udp->uh_sport);
    u_int16_t destination_port = ntohs(udp->uh_dport);
    u_int16_t len = ntohs(udp->uh_ulen);
    u_int16_t checksum = ntohs(udp->uh_sum);

    printf("Protocol: UDP\n");
    printf("+-------------------------+-------------------------+\n");
    printf("| Source Port:       %5u| Destination Port:  %5u|\n", source_port, destination_port);
    printf("+-------------------------+-------------------------+\n");
    printf("| Length:            %5u| Checksum:          %5u|\n", len, checksum);
    printf("+-------------------------+-------------------------+\n");
}

// record source IP and destination IP
void IP_count(char *srcIP, char *dstIP){
    int i;
    for(i=0; i < MAX_IP; i++){
        if(strcmp(srcIP, counter[i].srcIP) == 0 && strcmp(dstIP, counter[i].dstIP) == 0){
            counter[i].num++;
            break;
        }
        else if(strlen(counter[i].srcIP) == 0){
            strcpy(counter[i].srcIP, srcIP);
            strcpy(counter[i].dstIP, dstIP);
            counter[i].num++;
            pair_cnt++;
            break;
        }
    }
}

int cmp(const void *a, const void *b){
    if( strcmp((*(Counter *)a).srcIP, (*(Counter *)b).srcIP) == 0)
        return strcmp((*(Counter *)a).dstIP, (*(Counter *)b).dstIP);
    else
        return strcmp((*(Counter *)a).srcIP, (*(Counter *)b).srcIP);
}

void record_counter(){
    int i, cnt=0;

    qsort(counter, pair_cnt, sizeof(counter[0]), cmp);
    printf("---------------IP record---------------\n");
    for(i = 0; i < pair_cnt; i++){
        printf("%s -> %s : %d\n",counter[i].srcIP, counter[i].dstIP, counter[i].num);
        cnt += counter[i].num;
    }
    printf("The total of record: %d\n", cnt);
    
}

void usage(){
    printf("argument wrong\n");
    printf("usage:./read_cap {pcap file}\n");
    exit(0);
}
