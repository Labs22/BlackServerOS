/*
 * prettypacket.c
 *
 *  Created on: 03/dec/2012
 *      Author: Acri Emanuele <crossbower@tuta.io>
 *
 * Disassemble network packet and print their fields.
 * Uses the stdin to receive raw packet data. Prints on stdout.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "prettypacket.h"

#define VERSION "1.6"

#define BUFFER_SIZE 8192

/**
 * Packet type arguments
 */
enum packet_type {
    no_type = 0,
    tcp,
    udp,
    icmp,
    igmp,
    arp,
    stp,
    ipv6_tcp,
    ipv6_icmp,
    ipv6_in_ipv4
};

/**
 * Packets disassembling loop
 */
int dis_packet_loop()
{

    uint8_t buffer[BUFFER_SIZE];
    ssize_t size;

    int input = fileno(stdin);

    while (!feof(stdin)) {

        /* Read raw */
        size = read(input, buffer, BUFFER_SIZE);

        /* Print sisassembled packet */
        layer_2_dispatcher(buffer, size, 0);

        /* Print end of packet */
        puts("\n ----------- ");
        fflush(stdout);

    }

    puts("");

    return 0;
}

/**
 * Dump example packet
 */
void dump_example_packet (enum packet_type type) {

    const uint8_t *buffer = NULL;
    char *hexstr = NULL;
    int size = 0;

    switch (type) {
    case tcp: buffer = tcp_packet; size = sizeof(tcp_packet)-1; break;
    case udp: buffer = udp_packet; size = sizeof(udp_packet)-1; break;
    case icmp: buffer = icmp_packet; size = sizeof(icmp_packet)-1; break;
    case igmp: buffer = igmp_packet; size = sizeof(igmp_packet)-1; break;
    case arp: buffer = arp_packet; size = sizeof(arp_packet)-1; break;
    case stp: buffer = stp_packet; size = sizeof(stp_packet)-1; break;
    case ipv6_tcp: buffer = ipv6_tcp_packet; size = sizeof(ipv6_tcp_packet)-1; break;
    case ipv6_icmp: buffer = ipv6_icmp_packet; size = sizeof(ipv6_icmp_packet)-1; break;
    case ipv6_in_ipv4: buffer = ipv6_in_ipv4_packet; size = sizeof(ipv6_in_ipv4_packet)-1; break;
    case no_type: assert(0);
    }

    /* Write hexstring output */
    hexstr = raw_to_hexstr(buffer, size);

    fprintf(stdout, "%s\n", hexstr);
    fflush(stdout);

    free(hexstr);
}

/**
 * Print disassembled example packet
 */
void print_dis_example_packet (enum packet_type type) {
    switch (type) {
    case tcp: layer_2_dispatcher(tcp_packet, sizeof(tcp_packet)-1, 0); break;
    case udp: layer_2_dispatcher(udp_packet, sizeof(udp_packet)-1, 0); break;
    case icmp: layer_2_dispatcher(icmp_packet, sizeof(icmp_packet)-1, 0); break;
    case igmp: layer_2_dispatcher(igmp_packet, sizeof(igmp_packet)-1, 0); break;
    case arp: layer_2_dispatcher(arp_packet, sizeof(arp_packet)-1, 0); break;
    case stp: layer_2_dispatcher(stp_packet, sizeof(stp_packet)-1, 0); break;
    case ipv6_tcp: layer_2_dispatcher(ipv6_tcp_packet, sizeof(ipv6_tcp_packet)-1, 0); break;
    case ipv6_icmp: layer_2_dispatcher(ipv6_icmp_packet, sizeof(ipv6_icmp_packet)-1, 0); break;
    case ipv6_in_ipv4: layer_2_dispatcher(ipv6_in_ipv4_packet, sizeof(ipv6_in_ipv4_packet)-1, 0); break;
    case no_type: assert(0);
    }
    puts("");
}

/**
 * Convert string to packet_type
 */
enum packet_type str_to_packet_type(const char *str)
{
    if (strcmp(str, "tcp") == 0)
        return tcp;
    if (strcmp(str, "udp") == 0)
        return udp;
    if (strcmp(str, "icmp") == 0)
        return icmp;
    if (strcmp(str, "igmp") == 0)
        return igmp;
    if (strcmp(str, "arp") == 0)
        return arp;
    if (strcmp(str, "stp") == 0)
        return stp;
    if (strcmp(str, "ipv6_tcp") == 0)
        return ipv6_tcp;
    if (strcmp(str, "ipv6_icmp") == 0)
        return ipv6_icmp;
    if (strcmp(str, "ipv6_in_ipv4") == 0)
        return ipv6_in_ipv4;
    return no_type;
}

/**
 * Usage
 */
void usage(char *progname) {
    printf("PrettyPacket " VERSION " [disassembler for raw network packets]\n"
           "written by: Emanuele Acri <crossbower@tuta.io>\n\n"
           "Usage:\n"
           "\t%s [-x|-d|-h]\n"
           "\nOptions:\n"
           "\t-x <type>  print example packet, to see its structure\n"
           "\t-d <type>  dump example packet as hexstring\n"
           "\t           (types: tcp, udp, icmp, igmp, arp, stp,\n"
           "\t                   ipv6_tcp, ipv6_icmp, ipv6_in_ipv4)\n"
           "\t-h         this help screen\n", progname);
    exit(0);
}

/**
 * Main function
 */
int main(int argc, char **argv) {

    enum packet_type type = no_type;
    int dump = 0;

    // check arguments
    if(argc > 1) {

        // print example packet
        if(!strcmp(argv[1], "-x")) {

            if(argc < 3) usage(argv[0]);

            type = str_to_packet_type(argv[2]);

            if(type == no_type) usage(argv[0]);
        }

        // dump example packet
        else if(!strcmp(argv[1], "-d")) {

            if(argc < 3) usage(argv[0]);

            type = str_to_packet_type(argv[2]);
            dump = 1;

            if(type == no_type) usage(argv[0]);
        }

        // unknown options
        else {
            usage(argv[0]);
        } 
    }

    // example packet
    if(type != no_type) {
        if(dump)
            dump_example_packet(type);
        else
            print_dis_example_packet(type);
    }

    // disassemble packets
    else {
        dis_packet_loop();
    }

    return 0;
}
