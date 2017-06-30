/**
 * MO639 - Segurança de Redes
 * lab01- Extractor
 * extractor.c
 *
 * Gabriel Lorencetti Prado - 060999
 * Mauro Tardivo Filho      - 063140
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcap.h>
#include <netdb.h>

#define IP_SIZE 16
#define REQUEST_SIZE 100
#define PCAP_INTERFACENAME_SIZE 16
#define FILTER_SIZE 200
#define IANA_RESERVED_PORTS_COUNT 1024

/**
 * Compare function to be used with qsort
 */
int simple_cmp(const void *i1, const void *i2){

  return *((unsigned int*)i1) - *((unsigned int*)i2);

}

/**
 * Program usage
 */
void usage(char *prog_name){

  fprintf(stderr, "Usage:%s --victim-ip <ip> --victim-ethernet <mac> [-proto <protocols>] <pcap_file>\n", prog_name);
  exit(-1);

}

/**
 * Callback function to count packets
 */
void cnt_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet){
  printf("pacote, tamanho %d\n", header->len);
  (*((int *)args))++;

}

/**
 * Runs a specified filter in the specified pcap_file
 */
void run_filter(char *filter, pcap_t *handle){

  char errbuf[PCAP_ERRBUF_SIZE]; /* error buffer */
  struct bpf_program fp;         /* compiled filter */
  unsigned int cnt = 0;          /* package counter */
  
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
  
  /* loops through the package, counting them */
  pcap_loop(handle, -1, cnt_packet, (u_char*)&cnt);
  
  /* prints the package count */
  printf("%u\n", cnt);
  
  /* frees the compiled filter */
  pcap_freecode(&fp);
  
  /* closes the file */
  pcap_close(handle);
}

/**
 * Runs each filter in the specified pcap_file
 */
void run_filters(char *spoof_ip, char *request, char *interface){

  char filter[FILTER_SIZE];                                /* filter expression */
  char *proto, *tmp;                                       /* auxiliary strings */
  unsigned int i;                                          /* auxiliary counter */
  char *device;                                            /* device to sniff on */
  char errbuf[PCAP_ERRBUF_SIZE];                           /* pcap error messages buffer */
  pcap_t *handle;

  handle = pcap_open_live(interface,  /* device to sniff on */
       1500,  /* maximum number of bytes to capture per packet */
       1, /* promisc - 1 to set card in promiscuous mode, 0 to not */
       0, /* to_ms - amount of time to perform packet capture in milliseconds */
          /* 0 = sniff until error */
       errbuf); /* error message buffer if something goes wrong */

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
  
  /* apenas DNS */
  sprintf(filter, "udp");
  run_filter(filter, handle);
}

void parse_args(int argc, char *argv[], char *spoof_ip, char *request, char *pcap_interfacename){
  
  unsigned int i; /* iterator */
  
  /* invalid parameters count */
  /* if(argc != 6 && argc != 8){
    fprintf(stderr, "Too many or too few parameters found.\n");
    usage(argv[0]);
  }
 */
  for(i = 1; i < argc ; i++){
    if(!strcmp(argv[i], "--ip")){
      strncpy(spoof_ip, argv[++i], IP_SIZE-1);
      spoof_ip[IP_SIZE-1] = '\0';
    }

    if(!strcmp(argv[i], "--request")){
      strncpy(request, argv[++i], REQUEST_SIZE-1);
      request[REQUEST_SIZE-1] = '\0';
    }

    if(!strcmp(argv[i], "--interface")){
      strncpy(pcap_interfacename, argv[++i], PCAP_INTERFACENAME_SIZE-1);
      pcap_interfacename[PCAP_INTERFACENAME_SIZE-1] = '\0';
    }
  }
}

int main(int argc, char **argv){
  char spoof_ip[IP_SIZE];                /* ip address (xxx.xxx.xxx.xxx) */
  char request[REQUEST_SIZE];    /* request address (www.xxx.com.br) */
  char pcap_interfacename[PCAP_INTERFACENAME_SIZE]; /* interface name */
  
  parse_args(argc, argv, spoof_ip, request, pcap_interfacename);
  
  run_filters(spoof_ip, request, pcap_interfacename);
  
  return 0;
}
