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

Counter counter[MAX_IP];


int main(int argc, char **argv){
    char *file_name = NULL;
    if(argc != 2){
        usage();
    }else if(argc == 2){
        file_name = argv[1];
    }

    // file open 
    char errbuff[PCAP_ERRBUF_SIZE];
    pcap_t *handler = pcap_open_offline(file_name, errbuff);
    char *dev;

    // header structure
    struct pcap_pkthdr *header;
    struct ether_header *eth_header;
    struct ip *ip_header;
    struct tcphdr* tcp_header;
    struct udphdr* udp_header;

    char src_ip[INET_ADDRSTRLEN];   // source IP
    char dst_ip[INET_ADDRSTRLEN];   // destination IP

    for(int i = 0; i < MAX_IP; i++){
        counter[i].num = 0;
        memset(counter[i].srcIP, '\0', INET_ADDRSTRLEN);
        memset(counter[i].dstIP, '\0', INET_ADDRSTRLEN);
    }

    const u_char *packet;
    int packet_cnt = 0;
    u_int size_ip;
    u_int size_tcp;
    time_t tmp;
    struct tm ts;
	char dateBuf[80];
	int ret;

    u_int version, header_len;
    u_char tos, ttl, protocol;
    u_int16_t total_len, id, offset, checksum;

    while((ret = pcap_next_ex(handler, &header, &packet)) >= 0){
        if(ret == 0)continue;

        char dst_mac_addr[MAC_ADDLEN] = {};
    	char src_mac_addr[MAC_ADDLEN] = {};
		u_int16_t type;
        printf("Packet #%d:\n",++packet_cnt);

        // time stamp
        tmp = header->ts.tv_sec;
        ts = *localtime(&tmp);
        strftime(dateBuf, sizeof(dateBuf), "%m/%d/%Y %a %H:%M:%S", &ts);

        printf("%s\n", dateBuf);
		printf("Length: %d bytes\t", header->len);
    	printf("Capture length: %d bytes\n", header->caplen);

        eth_header = (struct ether_header *) packet;

        //  mac address
		strncpy(dst_mac_addr, mac_ntoa(eth_header->ether_dhost), sizeof(dst_mac_addr));
		strncpy(src_mac_addr, mac_ntoa(eth_header->ether_shost), sizeof(src_mac_addr));
		type = ntohs(eth_header->ether_type);
		printf("+-------------------------+-------------------------+\n");
		printf("| Destination MAC Address:     %17s    |\n", dst_mac_addr);
		printf("+-------------------------+-------------------------+\n");
		printf("| Source MAC Address:          %17s    |\n", src_mac_addr);
		printf("+-------------------------+-------------------------+\n");
        printf("\n");

        // Protocol is IP
		if (ntohs(eth_header->ether_type) == ETHERTYPE_IP) {
			ip_header = (struct ip*)(packet + sizeof(struct ether_header));
			inet_ntop(AF_INET, &(ip_header->ip_src), src_ip, INET_ADDRSTRLEN);
			inet_ntop(AF_INET, &(ip_header->ip_dst), dst_ip, INET_ADDRSTRLEN);
			version = ip_header->ip_v;
			header_len = ip_header->ip_hl << 2;
			tos = ip_header->ip_tos;
			total_len = ntohs(ip_header->ip_len);
			id = ntohs(ip_header->ip_id);
			offset = ntohs(ip_header->ip_off);
			ttl = ip_header->ip_ttl;
			protocol = ip_header->ip_p;
			checksum = ntohs(ip_header->ip_sum);
        	printf("Protocol: IP\n");
			printf("+-----+------+------------+-------------------------+\n");
			printf("| IV:%1u| HL:%2u| T: %8s| Total Length: %10u|\n", version, header_len, ip_ttoa(tos), total_len);
			printf("+-----+------+------------+-------+-----------------+\n");
			printf("| Identifier:        %5u| FF:%3s| FO:        %5u|\n", id, ip_ftoa(offset), offset & OFFMASK);
			printf("+------------+------------+-------+-----------------+\n");
			printf("| TTL:    %3u| Pro:    %3u| Header Checksum:   %5u|\n",ttl, protocol, checksum);
			printf("+------------+------------+-------------------------+\n");
			printf("| Source IP Address:                 %15s|\n", src_ip);
			printf("+---------------------------------------------------+\n");
			printf("| Destination IP Address:            %15s|\n", dst_ip);
			printf("+---------------------------------------------------+\n");

			// record source IP and destination IP
            IP_count(src_ip, dst_ip);

			// handle UDP and TCP
			switch (protocol) {
				case IPPROTO_UDP:
					dump_udp(header->caplen, packet);
					break;

				case IPPROTO_TCP:
					dump_tcp(header->caplen, packet);
					break;

				case IPPROTO_ICMP:
					printf("Protocol: ICMP\n");
					break;

				    default:
        			printf("Protocol: %d\n", protocol);
        			break;
			}
		}
		else if(ntohs(eth_header->ether_type) == ETHERTYPE_ARP){
			printf("ARP\n");
		}
		else if(ntohs(eth_header->ether_type) == ETHERTYPE_REVARP){
			printf("Reverse ARP\n");
		}
		else{
			printf("not support\n");
		}
        printf("\n");
        printf("\n");
    }

    record_counter();

    return 0;
}