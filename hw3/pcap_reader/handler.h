#ifndef _HANDLER_H
#define _HANDLER_H

#define OFFMASK 0x1fff
#define MAX_IP 100
#define FILE_LEN 20
#define MAC_ADDLEN 18
typedef unsigned char u_char;
typedef unsigned int u_int;

typedef struct{
    int num;
    char srcIP[INET_ADDRSTRLEN];
    char dstIP[INET_ADDRSTRLEN];
}Counter;

//extern int pair_cnt;

extern Counter counter[MAX_IP];

char *mac_ntoa(u_char *d);

char *ip_ttoa(u_int8_t flag);

char *ip_ftoa(u_int16_t flag);

void dump_tcp(u_int32_t length, const u_char *content);

void dump_udp(u_int32_t length, const u_char *content);

void IP_count(char *srcIP, char *dstIP);

void record_counter();

void usage();

#endif