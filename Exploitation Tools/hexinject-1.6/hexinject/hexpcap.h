/**
 * HEXPCAP HEADER
 *
 * Utilities to sniff and inject packets (in raw or hexstring formats)
 * via pcap libraries.
 */

#ifndef HEXPCAP_H_
#define HEXPCAP_H_

#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include <assert.h>
#include <arpa/inet.h>

#include "hexstring.h"

/**
 * Required definitions
 */
void do_cksum_ipv4 (uint8_t *, size_t);
void do_cksum_ipv6 (uint8_t *, size_t);
void do_size_ipv4 (uint8_t *, size_t);
void do_size_ipv6 (uint8_t *, size_t);

/**
 * Checksum (simple)
 */
uint16_t cksum_simple (uint16_t *buff, size_t len)
{
    
    uint32_t sum = 0;
    uint16_t answer = 0;

    while(len > 1) {
        sum += *buff++;
        len -= 2;
    }

    if (len) {
        sum += * (uint8_t *) buff;
    }

    while (sum>>16)
        sum = (sum & 0xffff) + (sum >> 16);

    answer = ~sum;

    return(answer);
}

/**
 * Checksum (with pseudo header)
 */
uint16_t cksum_pseudo_header (uint16_t protocol_code, uint16_t *src_addr, uint16_t *dest_addr, uint8_t addr_len, uint16_t *buff, uint16_t len)
{

    uint8_t i = 0;
    uint32_t sum = 0;
    uint16_t answer = 0;

    for(i = 0; i < (addr_len/2); i++) {
        sum += src_addr[i];
    }
    
    for(i = 0; i < (addr_len/2); i++) {
        sum += dest_addr[i];
    }

    sum += htons(protocol_code);

    sum += htons(len);

    while(len > 1) {
        sum += *buff++;
        len -= 2;
    }

    if (len) {
        sum += * (uint8_t *) buff;
    }

    while (sum>>16)
        sum = (sum & 0xffff) + (sum >> 16);

    answer = ~sum;

    return(answer);
}

/**
 * Do checksum of IPv4 and IPv6 data (next protocol dispatch)
 */
void do_cksum_ipv4_ipv6_next_protocol (uint8_t next_protocol, uint8_t *ip_header, uint8_t *raw, size_t size)
{

    uint8_t ip_version = 0;
    uint16_t *cksum = NULL;

    ip_version = ip_header[0] >> 4;

    switch(next_protocol) {

        // tcp
        case 0x06:
            if (size < 20) return; // size check
            cksum = (uint16_t *) &raw[16];
            goto PSEUDO_HEADER_CKSUM;

        // udp
        case 0x11:
            if (size < 8) return; // size check
            cksum = (uint16_t *) &raw[6];
            goto PSEUDO_HEADER_CKSUM;

        // icmpv6
        case 0x3a:
            if (size < 8) return; // size check
            cksum = (uint16_t *) &raw[2];

PSEUDO_HEADER_CKSUM:
            *cksum = 0;
            if(ip_version == 4) {
                *cksum = cksum_pseudo_header((uint16_t) next_protocol,
                                             (uint16_t *) &ip_header[12],
                                             (uint16_t *) &ip_header[16],
                                             4, (uint16_t *) raw, size);
            } else if (ip_version == 6) {
                *cksum = cksum_pseudo_header((uint16_t) next_protocol,
                                             (uint16_t *) &ip_header[8],
                                             (uint16_t *) &ip_header[24],
                                             16, (uint16_t *) raw, size);
            }
            break;

        // icmp
        case 0x01:
            if (size < 8) return; // size check
            cksum = (uint16_t *) &raw[2];
            goto SIMPLE_CKSUM;

        // igmp
        case 0x02:
            if (size < 8) return; // size check
            cksum = (uint16_t *) &raw[2];

SIMPLE_CKSUM:
            *cksum = 0;
            *cksum = cksum_simple((uint16_t *) raw, size);
            break;

        // encapsulated ipv4
        case 0x04:
            do_cksum_ipv4(raw, size);
            break;

        // encapsulated ipv6
        case 0x29:
            do_cksum_ipv6(raw, size); 
            break;

    }
 
}

/**
 * Do checksum of IPv4 packet
 */
void do_cksum_ipv4 (uint8_t *raw, size_t size)
{

    uint8_t hdr_size = 0, next_protocol = 0;
    uint16_t *cksum = NULL;

    // ipv4 header size
    hdr_size = (raw[0] & 0x0F) * 4;
    if(hdr_size > size) return;

    // ipv4 checksum
    cksum = (uint16_t *) &raw[10];
    *cksum = 0;

    *cksum = cksum_simple((uint16_t *) raw, hdr_size);

    // do checksum of next protocol
    next_protocol = raw[9];
    do_cksum_ipv4_ipv6_next_protocol(next_protocol, raw, raw + hdr_size, size - hdr_size);

}

/**
 * Do checksum of IPv6 packet
 */
void do_cksum_ipv6 (uint8_t *raw, size_t size)
{

    uint8_t next_protocol = 0;
    uint8_t *next_header = NULL;

    next_protocol = raw[6];
    next_header = raw + 40;

    // skip ipv6 extension headers
    while (1) {
        switch (next_protocol) {

	case 0:   // hop-by-hop options
        case 43:  // routing options
        case 44:  // fragment options
        case 50:  // encapsulating security payload
        case 51:  // authentication header
        case 60:  // destination options
        case 135: // mobility options
            next_protocol = next_header[0];
            next_header += 8 + (next_header[1] * 8);
            if((next_header - raw) > (size - 8)) return; // check size
            break;

        default: // next protocol header
            do_cksum_ipv4_ipv6_next_protocol(next_protocol, raw, next_header, size - (next_header - raw));
            return;

        }
    }

}

/**
 * Do checksum (if the packet requires it...)
 */
void do_cksum (uint8_t *raw, size_t size)
{
    
    // is ipv4?
    if ( size >= 34 && raw[12]==0x08  && raw[13]==0x00  ) {
        do_cksum_ipv4(raw + 14, size - 14);
    }

    // is ipv6?
    else if ( size >= 54 && raw[12]==0x86  && raw[13]==0xDD ) {
        do_cksum_ipv6(raw + 14, size - 14);
    }

}

/**
 * Adjust size fields of IPv4 and IPv6 data (next protocol dispatch)
 */
void do_size_ipv4_ipv6_next_protocol (uint8_t next_protocol, uint8_t *raw, size_t size)
{

    uint16_t *len_field = NULL;

    switch(next_protocol) {

        // udp
        case 0x11:
            if (size < 8) return; // size check
            len_field = (uint16_t *) &raw[4];
            *len_field = htons(size);
            break;

        // encapsulated ipv4
        case 0x04:
            do_size_ipv4(raw, size);
            break;

        // encapsulated ipv6
        case 0x29:
            do_size_ipv6(raw, size); 
            break;

    }
 
}

/**
 * Adjust packet size fields of IPv4 packet
 */
void do_size_ipv4 (uint8_t *raw, size_t size)
{

    uint8_t hdr_size = 0, next_protocol = 0;
    uint16_t *len_field = NULL;

    // ipv4 header size
    hdr_size = (raw[0] & 0x0F) * 4;
    if(hdr_size > size) return;

    // ipv4 size field
    len_field = (uint16_t *) &raw[2];
    *len_field = htons(size);

    // do size of next protocol
    next_protocol = raw[9];
    do_size_ipv4_ipv6_next_protocol(next_protocol, raw + hdr_size, size - hdr_size);

}

/**
 * Adjust packet size fields of IPv6 packet
 */
void do_size_ipv6 (uint8_t *raw, size_t size)
{

    uint8_t next_protocol = 0;
    uint8_t *next_header = NULL;

    uint16_t *len_field = NULL;

    // set payload size
    len_field = (uint16_t *) &raw[4];
    *len_field = htons(size - 40);

    // select next header
    next_protocol = raw[6];
    next_header = raw + 40;

    // skip ipv6 extension headers
    while (1) {
        switch (next_protocol) {

	case 0:   // hop-by-hop options
        case 43:  // routing options
        case 44:  // fragment options
        case 50:  // encapsulating security payload
        case 51:  // authentication header
        case 60:  // destination options
        case 135: // mobility options
            next_protocol = next_header[0];
            next_header += 8 + (next_header[1] * 8);
            if((next_header - raw) > (size - 8)) return; // check size
            break;

        default: // next protocol header
            do_size_ipv4_ipv6_next_protocol(next_protocol, next_header, size - (next_header - raw));
            return;

        }
    }

}

/**
 * Adjust packet size fields (if the packet requires it...)
 */
void do_size (uint8_t *raw, size_t size)
{

    // is ipv4?
    if ( size >= 34 && raw[12]==0x08  && raw[13]==0x00  ) {
        do_size_ipv4(raw + 14, size - 14);
    }

    // is ipv6?
    else if ( size >= 54 && raw[12]==0x86  && raw[13]==0xDD ) {
        do_size_ipv6(raw + 14, size - 14);
    }
    
    uint16_t *len_field = NULL;

    // is ip?
    if ( size >= 34 && raw[12]==0x08  && raw[13]==0x00  ) {
       
        // ip total length
        len_field = (uint16_t *) &raw[16];

        *len_field = size - 14; // size - ethernet header
        *len_field = htons(*len_field);

        // next protocol
        switch(raw[23]) {

            // tcp
            case 0x06:
                if (size < 54) return; // size check
                // tcp uses header length field
                break;

            // udp
            case 0x11:
                if (size < 42) return; // size check
                len_field = (uint16_t *) &raw[38];
                *len_field = size - 14 - ((raw[14] & 0xF) * 4); // size - ethernet header - ip header
                *len_field = htons(*len_field);
                break;

            // icmp
            case 0x01:
                //if (size < 42) return; // size check
                // no size field
                break;

            // igmp
            case 0x02:
                //if (size < 42) return; // size check
                // no size field
                break;
        }
    }
}

/**
 * Inject a raw buffer to the network
 */
int inject_raw(pcap_t *fp, uint8_t *raw, size_t size, int disable_cksum, int disable_size)
{

    assert(fp != NULL);
    assert(raw != NULL);

    int err = 0;
    
    /* packet size (if enabled) */
    if(!disable_size) do_size (raw, size);

    /* checksum */
    if(!disable_cksum) do_cksum (raw, size);

    /* Send down the packet */
    err = pcap_sendpacket(fp, (unsigned char *) raw, size);

    return err;
}

/**
 * Inject an hexstring to the network
 */
int inject_hexstr(pcap_t *fp, char *hexstr, int disable_cksum, int disable_size)
{

    assert(fp != NULL);
    assert(hexstr != NULL);

    int err = 0;
    int size = 0;
    uint8_t *raw = NULL;

    raw = hexstr_to_raw(hexstr, &size);

    /* Send down the packet */
    err = inject_raw(fp, raw, size, disable_cksum, disable_size);

    free(raw);

    return err;
}

/**
 * Sniff a packet from the network as an hexstring. The hexstring must be manually free()d.
 */
char *sniff_hexstr(pcap_t *fp)
{

    assert(fp != NULL);

    struct pcap_pkthdr hdr;
    char *hexstr = NULL;
    uint8_t *raw    = NULL;

    /* Sniff the packet */
    raw = (uint8_t *) pcap_next(fp, &hdr);

    if(raw == NULL)
        return NULL;

    hexstr = raw_to_hexstr(raw, hdr.len);

    return hexstr;
}

/**
 * Sniff a packet from the network as a raw buffer. The buffer must NOT be modified or free()d.
 */
const uint8_t *sniff_raw(pcap_t *fp, size_t *size)
{

    assert(fp != NULL);
    assert(size != NULL);

    struct pcap_pkthdr hdr;
    const uint8_t *raw = NULL;

    /* Sniff the packet */
    raw = (const uint8_t *) pcap_next(fp, &hdr);

    *size = hdr.len;

    return raw;
}

#endif /* HEXPCAP_H_ */
