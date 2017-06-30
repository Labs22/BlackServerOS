/*
    This file is part of pcapsipdump

    pcapsipdump is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    pcapsipdump is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    ---

    Project's home: http://pcapsipdump.sf.net/
*/

#ifdef sparc
#define __BIG_ENDIAN 1
#endif
#ifndef sparc
#include <endian.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef USE_REGEXP
#include <regex.h>
#endif

#include <pcap.h>

#include "calltable.h"
#include "pcapsipdump.h"

#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define MIN(x,y) ((x) < (y) ? (x) : (y))

int get_sip_peername(char *data, int data_len, const char *tag, char *caller, int caller_len);
int get_ip_port_from_sdp(char *sdp_text, in_addr_t *addr, unsigned short *port);
char * gettag(const void *ptr, unsigned long len, const char *tag, unsigned long *gettaglen);
uint32_t get_ssrc (void *ip_packet_data, bool is_rtcp);
#ifndef _GNU_SOURCE
void *memmem(const void* haystack, size_t hl, const void* needle, size_t nl);
#endif

calltable *ct;

void sigint_handler(int param)
{
    printf("SIGINT received, terminating\n");
    ct->do_cleanup(0);
    exit(1);
}

void sigterm_handler(int param)
{
    printf("SIGTERM received, terminating\n");
    ct->do_cleanup(0);
    exit(1);
}

#define RTPSAVE_NONE 0
#define RTPSAVE_RTP 1
#define RTPSAVE_RTP_RTCP 2
#define RTPSAVE_RTPEVENT 3

int main(int argc, char *argv[])
{

    pcap_t *handle;/* Session handle */
    const char *opt_chdir;/* directory to write dump */
    char *ifname;/* interface to sniff on */
    char *fname;/* pcap file to read on */
    char errbuf[PCAP_ERRBUF_SIZE];/* Error string */
    struct bpf_program fp;/* The compiled filter */
    char filter_exp[] = "udp";/* The filter expression */
    bpf_u_int32 mask;/* Our netmask */
    bpf_u_int32 net;/* Our IP */
    struct pcap_pkthdr *pkt_header; /* The header that pcap gives us */
    const u_char *pkt_data; /* The actual packet */
    unsigned long last_cleanup=0;
    int res;
    int offset_to_ip=0;
    int opt_fork=1;
    int opt_promisc=1;
    int opt_packetbuffered=0;
    int opt_t38only=0;
    int opt_rtpsave=RTPSAVE_RTP_RTCP;
    int verbosity=0;
    bool number_filter_matched=false;
#ifdef USE_REGEXP
    regex_t number_filter;
    number_filter.allocated=0;
#else
    char number_filter[128];
    number_filter[0]=0;
#endif

    ifname=NULL;
    fname=NULL;
    opt_chdir="/var/spool/pcapsipdump";

    while(1) {
        char c;

        c = getopt (argc, argv, "i:r:d:v:n:R:fpUt");
        if (c == -1)
            break;

        switch (c) {
            case 'i':
                ifname=optarg;
                break;
            case 'v':
                verbosity=atoi(optarg);
                break;
            case 'n':
#ifdef USE_REGEXP
                regcomp(&number_filter,optarg,0);
#else
                strcpy(number_filter,optarg);
#endif
                break;
            case 'R':
                if (strcasecmp(optarg,"none")==0){
                    opt_rtpsave=RTPSAVE_NONE;
                }else if (strcasecmp(optarg,"rtpevent")==0){
                    opt_rtpsave=RTPSAVE_RTPEVENT;
                }else if (strcasecmp(optarg,"t38")==0){
                    opt_t38only=1;
                }else if (strcasecmp(optarg,"all")==0 ||
                          strcasecmp(optarg,"rtp+rtcp")==0){
                    opt_rtpsave=RTPSAVE_RTP_RTCP;
                }else if (strcasecmp(optarg,"rtp")==0){
                    opt_rtpsave=RTPSAVE_RTP;
                }else{
                    printf("Unrecognized RTP filter specification: '%s'\n",optarg);
	            return 1;
                }
                break;
            case 't':
                opt_t38only=1;
                break;
            case 'r':
                fname=optarg;
                break;
            case 'd':
                opt_chdir=optarg;
                break;
            case 'f':
                opt_fork=0;
                break;
            case 'p':
                opt_promisc=0;
                break;
            case 'U':
		opt_packetbuffered=1;
                break;
        }
    }

    // allow interface to be specified without '-i' option - for sake of compatibility
    if (optind < argc) {
	ifname = argv[optind];
    }

    if ((fname==NULL)&&(ifname==NULL)){
	printf( "pcapsipdump version %s\n"
		"Usage: pcapsipdump [-fpU] [-i <interface>] [-r <file>] [-d <working directory>] [-v level] [-R filter]\n"
		" -f   Do not fork or detach from controlling terminal.\n"
		" -p   Do not put the interface into promiscuous mode.\n"
		" -R   RTP filter. Possible values: 'rtp+rtcp' (default), 'rtp', 'rtpevent', 't38', or 'none'.\n"
		" -U   Make .pcap files writing 'packet-buffered' - slower method,\n"
		"      but you can use partitially written file anytime, it will be consistent.\n"
		" -v   Set verbosity level (higher is more verbose).\n"
		" -n   Number-filter. Only calls to/from specified number will be recorded\n"
#ifdef USE_REGEXP
		"      Argument is regular expression. See 'man 7 regex' for details\n"
#endif
		" -t   T.38-filter. Only calls, containing T.38 payload indicated in SDP will be recorded\n"
		,PCAPSIPDUMP_VERSION);
	return 1;
    }

    ct = new calltable;
    if (opt_t38only){
        ct->erase_non_t38=1;
    }
    signal(SIGINT,sigint_handler);
    signal(SIGTERM,sigterm_handler);

    if (ifname){
	printf("Capturing on interface: %s\n", ifname);
	/* Find the properties for interface */
	if (pcap_lookupnet(ifname, &net, &mask, errbuf) == -1) {
	    fprintf(stderr, "Couldn't get netmask for interface %s: %s\n", ifname, errbuf);
	    net = 0;
	    mask = 0;
	}
	handle = pcap_open_live(ifname, 1600, opt_promisc, 1000, errbuf);
	if (handle == NULL) {
	    fprintf(stderr, "Couldn't open interface '%s': %s\n", ifname, errbuf);
	    return(2);
	}
    }else{
	printf("Reading file: %s\n", fname);
        net = 0;
        mask = 0;
	handle = pcap_open_offline(fname, errbuf);
	if (handle == NULL) {
	    fprintf(stderr, "Couldn't open pcap file '%s': %s\n", ifname, errbuf);
	    return(2);
	}
    }

    chdir(opt_chdir);

    /* Compile and apply the filter */
    if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
	fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
	return(2);
    }
    if (pcap_setfilter(handle, &fp) == -1) {
	fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
	return(2);
    }

    if (opt_fork){
	// daemonize
	if (fork()) exit(0);
    }

    {
	int dlt=pcap_datalink(handle);
	switch (dlt){
	    case DLT_EN10MB :
		offset_to_ip=sizeof(struct ether_header);
		break;
	    case DLT_LINUX_SLL :
		offset_to_ip=16;
		break;
	    case DLT_RAW :
		offset_to_ip=0;
		break;
	    default : {
		printf("Unknown interface type (%d).\n",dlt);
		return 3;
	    }
	}
    }


    /* Retrieve the packets */
    while((res = pcap_next_ex( handle, &pkt_header, &pkt_data)) >= 0){
	{
	    struct iphdr *header_ip;
	    struct udphdr *header_udp;
	    char *data;
	    char *s;
	    char str1[1024],str2[1024];
	    unsigned long datalen;
	    unsigned long l;
	    int idx;

            if(res == 0)
                /* Timeout elapsed */
                continue;

	    if (pkt_header->ts.tv_sec-last_cleanup>15){
		if (last_cleanup>=0){
		    ct->do_cleanup(pkt_header->ts.tv_sec);
		}
		last_cleanup=pkt_header->ts.tv_sec;
	    }
	    header_ip=(iphdr *)((char*)pkt_data+offset_to_ip);
	    if (header_ip->protocol==17){//UPPROTO_UDP=17
                int idx_leg=0;
                int idx_rtp=0;
                int save_this_rtp_packet=0;
                int is_rtcp=0;
                uint16_t rtp_port_mask=0xffff;

                header_udp=(udphdr *)((char*)header_ip+sizeof(*header_ip));
                data=(char *)header_udp+sizeof(*header_udp);
                datalen=pkt_header->len-((unsigned long)data-(unsigned long)pkt_data);

                if (opt_rtpsave==RTPSAVE_RTP){
                    save_this_rtp_packet=1;
                }else if (opt_rtpsave==RTPSAVE_RTP_RTCP){
                    save_this_rtp_packet=1;
                    rtp_port_mask=0xfffe;
                    is_rtcp=(htons(header_udp->source) & 1) && (htons(header_udp->dest) & 1);
                }else if (opt_rtpsave==RTPSAVE_RTPEVENT &&
                           datalen==18 && (data[0]&0xff) == 0x80 && (data[1]&0x7d) == 0x65){
                    save_this_rtp_packet=1;
                }else{
                    save_this_rtp_packet=0;
                }

                if (save_this_rtp_packet &&
                        ct->find_ip_port_ssrc(
                            header_ip->daddr,htons(header_udp->dest) & rtp_port_mask,
                            get_ssrc(data,is_rtcp),
                            &idx_leg,&idx_rtp)){
                    if (ct->table[idx_leg].f_pcap!=NULL) {
                        ct->table[idx_leg].last_packet_time=pkt_header->ts.tv_sec;
                        pcap_dump((u_char *)ct->table[idx_leg].f_pcap,pkt_header,pkt_data);
                        if (opt_packetbuffered) {pcap_dump_flush(ct->table[idx_leg].f_pcap);}
                    }
                }else if (save_this_rtp_packet &&
                        ct->find_ip_port_ssrc(
                            header_ip->saddr,htons(header_udp->source) & rtp_port_mask,
                            get_ssrc(data,is_rtcp),
                            &idx_leg,&idx_rtp)){
                    if (ct->table[idx_leg].f_pcap!=NULL) {
                        ct->table[idx_leg].last_packet_time=pkt_header->ts.tv_sec;
                        pcap_dump((u_char *)ct->table[idx_leg].f_pcap,pkt_header,pkt_data);
                        if (opt_packetbuffered) {pcap_dump_flush(ct->table[idx_leg].f_pcap);}
                    }
                }else if (htons(header_udp->source)==5060||
                    htons(header_udp->dest)==5060){
                    char caller[256];
                    char called[256];
                    char sip_method[256];

                    //figure out method
                    memcpy(sip_method,data,sizeof(sip_method)-1);
                    sip_method[sizeof(sip_method)-1]=' ';
                    if (strchr(sip_method,' ')!=NULL){
                        *strchr(sip_method,' ')='\0';
                    }else{
                        sip_method[0]='\0';
                        if (verbosity>=2){
                            printf("Empty SIP method!\n");
                        }
                    }

		    data[datalen]=0;
		    get_sip_peername(data,datalen,"From:",caller,sizeof(caller));
		    get_sip_peername(data,datalen,"To:",called,sizeof(called));
		    s=gettag(data,datalen,"Call-ID:",&l);
                    number_filter_matched=false;
#ifdef USE_REGEXP
                    {
                        regmatch_t pmatch[1];
                        if ((number_filter.allocated==0) ||
                            (regexec(&number_filter, caller, 1, pmatch, 0)==0) ||
                            (regexec(&number_filter, called, 1, pmatch, 0)==0)) {
                            number_filter_matched=true;
                        }
                    }
#else
                    if (number_filter[0]==0||(strcmp(number_filter,caller)==0)||(strcmp(number_filter,called)==0)) {
                        number_filter_matched=true;
                    }
#endif
		    if (s!=NULL && ((idx=ct->find_by_call_id(s,l))<0) && number_filter_matched){
			if ((idx=ct->add(s,l,pkt_header->ts.tv_sec))<0){
			    printf("Too many simultaneous calls. Ran out of call table space!\n");
			}else{
			    if ((strcmp(sip_method,"INVITE")==0)||(strcmp(sip_method,"OPTIONS")==0)||(strcmp(sip_method,"REGISTER")==0)){
				struct tm *t;
				t=localtime(&pkt_header->ts.tv_sec);
				sprintf(str2,"%04d%02d%02d",
					t->tm_year+1900,t->tm_mon+1,t->tm_mday);
				mkdir(str2,0700);
				sprintf(str2,"%04d%02d%02d/%02d",
					t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour);
				mkdir(str2,0700);
				sprintf(str2,"%04d%02d%02d/%02d/%04d%02d%02d-%02d%02d%02d-%s-%s",
					t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,
					t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec,caller,called);
				memcpy(str1,s,l);
				str1[l]='\0';
				strcat(str2,"-");
				strcat(str2,str1);
				strcat(str2,".raw");
				ct->table[idx].f=NULL;
				str1[l]='\0';
				*strstr(str2,".raw")='\0';
				strcat(str2,".pcap");
				ct->table[idx].f_pcap=pcap_dump_open(handle,str2);
				strncpy(ct->table[idx].fn_pcap,str2,sizeof(ct->table[idx].fn_pcap));
			    }else{
				if (verbosity>=2){
				    printf("Unknown SIP method:'%s'!\n",sip_method);
				}
				ct->table[idx].f=NULL;
				ct->table[idx].f_pcap=NULL;
			    }
			}
		    }

                    // idx holds a valid pointer to open leg at this point
                    if (strcmp(sip_method,"BYE")==0){
                        ct->table[idx].had_bye=1;
                    }
		    s=gettag(data,datalen,"Content-Type:",&l);
		    if(idx>=0 && l>0 && strncasecmp(s,"application/sdp",l)==0 && strstr(data,"\r\n\r\n")!=NULL){
			in_addr_t tmp_addr;
			unsigned short tmp_port;
			if (!get_ip_port_from_sdp(strstr(data,"\r\n\r\n")+1,&tmp_addr,&tmp_port)){
			    ct->add_ip_port(idx,tmp_addr,tmp_port);
			}else{
			    if (verbosity>=2){
				printf("Can't get ip/port from SDP:\n%s\n\n",strstr(data,"\r\n\r\n")+1);
			    }
			}
			if (opt_t38only && memmem(data,datalen,"udptl t38",9)!=NULL){
			    ct->table[idx].had_t38=1;
			}
		    }

		    if (ct->table[idx].f_pcap!=NULL){
			pcap_dump((u_char *)ct->table[idx].f_pcap,pkt_header,pkt_data);
			if (opt_packetbuffered) {pcap_dump_flush(ct->table[idx].f_pcap);}
		    }
		}else{
		    if (verbosity>=3){
			char st1[16];
			char st2[16];
			struct in_addr in;

			in.s_addr=header_ip->saddr;
			strcpy(st1,inet_ntoa(in));
			in.s_addr=header_ip->daddr;
			strcpy(st2,inet_ntoa(in));
			printf ("Skipping udp packet %s:%d->%s:%d\n",st1,htons(header_udp->source),st2,htons(header_udp->dest));
		    }
		}
	    }
	}
    }
    /* flush / close files */
    ct->do_cleanup(1<<31);
    /* And close the session */
    pcap_close(handle);
    return(0);
}

int get_sip_peername(char *data, int data_len, const char *tag, char *peername, int peername_len){
    unsigned long r,r2,peername_tag_len;
    char *peername_tag=gettag(data,data_len,tag,&peername_tag_len);
    if ((r=(unsigned long)memmem(peername_tag,peername_tag_len,"sip:",4))==0){
	goto fail_exit;
    }
    r+=4;
    if ((r2=(unsigned long)memmem(peername_tag,peername_tag_len,"@",1))==0){
	goto fail_exit;
    }
    if (r2<=r){
	goto fail_exit;
    }
    memcpy(peername,(void*)r,r2-r);
    memset(peername+(r2-r),0,1);
    return 0;
fail_exit:
    strcpy(peername,"empty");
    return 1;
}

int get_ip_port_from_sdp(char *sdp_text, in_addr_t *addr, unsigned short *port){
    unsigned long l;
    char *s;
    char s1[20];
    s=gettag(sdp_text,strlen(sdp_text),"c=IN IP4 ",&l);
    memset(s1,'\0',sizeof(s1));
    memcpy(s1,s,MIN(l,19));
    if ((long)(*addr=inet_addr(s1))==-1){
	*addr=0;
	*port=0;
	return 1;
    }
    s=gettag(sdp_text,strlen(sdp_text),"m=audio ",&l);
    if (l==0){
        s=gettag(sdp_text,strlen(sdp_text),"m=image ",&l);
    }
    if (l==0 || (*port=atoi(s))==0){
	*port=0;
	return 1;
    }
    return 0;
}

char * gettag(const void *ptr, unsigned long len, const char *tag, unsigned long *gettaglen){
    unsigned long register r,l,tl;
    char *rc;

    tl=strlen(tag);
    r=(unsigned long)memmem(ptr,len,tag,tl);
    if(r==0){
        l=0;
    }else{
        r+=tl;
        l=(unsigned long)memmem((void *)r,len-(r-(unsigned long)ptr),"\r\n",2);
        if (l>0){
            l-=r;
        }else{
            l=0;
            r=0;
        }
    }
    rc=(char*)r;
    if (rc){
        while (rc[0]==' '){
            rc++;
            l--;
        }
    }
    *gettaglen=l;
    return rc;
}

inline uint32_t get_ssrc (void *udp_packet_data_pointer, bool is_rtcp){
    if (is_rtcp){
        return ntohl(*(uint32_t*)((uint8_t*)udp_packet_data_pointer+4));
    }else{
        return ntohl(*(uint32_t*)((uint8_t*)udp_packet_data_pointer+8));
    }
}

#ifndef _GNU_SOURCE
void *memmem(const void* haystack, size_t hl, const void* needle, size_t nl) {
    int i;

    if (nl>hl) return 0;
    for (i=hl-nl+1; i; --i) {
	if (!memcmp(haystack,needle,nl)){
	    return (char*)haystack;
	}
	haystack=(void*)((char*)haystack+1);
    }
    return 0;
}
#endif
