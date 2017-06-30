#ifndef __PRETTYPACKET_H__
#define __PRETTYPACKET_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "hexstring.h"

/**
 * Default terminal rows
 */
static const int rows = 24;

/**
 * Default terminal columns
 */
static const int cols = 80;

/**
 * Example ARP packet
 */
static const uint8_t arp_packet[] = "\xFF\xFF\xFF\xFF\xFF\xFF\xAA\x00\x04\x00\x0A\x04\x08\x06\x00\x01\x08\x00\x06\x04\x00\x01\xAA\x00\x04\x00\x0A\x04\xC0\xA8\x01\x09\x00\x00\x00\x00\x00\x00\xC0\xA8\x01\x04";

/**
 * Example TCP packet
 */
static const uint8_t tcp_packet[] = "\x1C\xAF\xF7\x6B\x0E\x4D\xAA\x00\x04\x00\x0A\x04\x08\x00\x45\x00\x00\x34\x5A\xAE\x40\x00\x40\x06\x5E\x67\xC0\xA8\x01\x09\x58\xBF\x67\x3E\x9B\x44\x00\x50\x8E\xB5\xC6\xAC\x15\x93\x47\x9E\x80\x10\x00\x58\xA5\xA0\x00\x00\x01\x01\x08\x0A\x00\x09\xC3\xB2\x42\x5B\xFA\xD6";

/**
 * Example of IPv6 TCP packet
 */
static const uint8_t ipv6_tcp_packet[] = "\xc5\x00\x00\x00\x82\xc4\x00\x12\x1e\xf2\x61\x3d\x86\xdd\x60\x0A\x0B\x0C\x00\x20\x06\xf6\x24\x02\xf0\x00\x00\x01\x8e\x01\x00\x00\x00\x00\x00\x00\x55\x55\x26\x07\xfc\xd0\x01\x00\x23\x00\x00\x00\x00\x00\xb1\x08\x2a\x6b\x9b\x44\x00\x50\x8e\xb5\xc6\xac\x15\x93\x47\x9e\x80\x10\x00\x58\x0d\xa9\x00\x00\x01\x01\x08\x0a\x00\x09\xc3\xb2\x42\x5b\xfa\xd6";

/**
 * Example ICMP packet
 */
static const uint8_t icmp_packet[] = "\x1C\xAF\xF7\x6B\x0E\x4D\xAA\x00\x04\x00\x0A\x04\x08\x00\x45\x00\x00\x54\x00\x00\x40\x00\x40\x01\x54\x4E\xC0\xA8\x01\x09\xC0\xA8\x64\x01\x08\x00\x34\x98\xD7\x10\x00\x01\x5B\x68\x98\x4C\x00\x00\x00\x00\x2D\xCE\x0C\x00\x00\x00\x00\x00\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2A\x2B\x2C\x2D\x2E\x2F\x30\x31\x32\x33\x34\x35\x36\x37";

/**
 * Example of ICMPv6 packet
 */
static const uint8_t ipv6_icmp_packet[] = "\x33\x33\x00\x00\x00\x16\x08\x00\x27\xd4\x10\xbb\x86\xdd\x60\x00\x00\x00\x00\x38\x00\x01\xfe\x80\x00\x00\x00\x00\x00\x00\x0a\x00\x27\xff\xfe\xd4\x10\xbb\xff\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x16\x3a\x00\x05\x02\x00\x00\x01\x00\x8f\x00\x2b\x5a\x00\x00\x00\x02\x04\x00\x00\x00\xff\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x03\x04\x00\x00\x00\xff\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x02";

/**
 * Example UDP packet
 */
static const uint8_t udp_packet[] = "\x1C\xAF\xF7\x6B\x0E\x4D\xAA\x00\x04\x00\x0A\x04\x08\x00\x45\x00\x00\x3C\x9B\x23\x00\x00\x40\x11\x70\xBC\xC0\xA8\x01\x09\xD0\x43\xDC\xDC\x91\x02\x00\x35\x00\x28\x6F\x0B\xAE\x9C\x01\x00\x00\x01\x00\x00\x00\x00\x00\x00\x03\x77\x77\x77\x06\x67\x6F\x6F\x67\x6C\x65\x03\x63\x6F\x6D\x00\x00\x01\x00\x01";

/**
 * Example IGMP packet
 */
static const uint8_t igmp_packet[] = "\x1C\xAF\xF7\x6B\x0E\x4D\xAA\x00\x04\x00\x0A\x04\x08\x00\x45\x00\x00\x1C\x00\x00\x40\x00\x40\x02\x54\x4E\xC0\xA8\x01\x09\xC0\xA8\x64\x01\x11\xFF\x0D\xFF\xE0\x00\x00\x01";

/**
 * Example of IPv6 in IPv4 encapsulation
 */
static const uint8_t ipv6_in_ipv4_packet[] = "\x00\x90\x1a\x41\x65\x41\x00\x16\xcf\x41\x9c\x20\x08\x00\x45\x00\x00\x50\x93\x5a\x00\x00\x80\x29\x67\xc6\x46\x37\xd5\xd3\xc0\x58\x63\x01\x60\x00\x00\x00\x00\x14\x06\x80\x20\x02\x46\x37\xd5\xd3\x00\x00\x00\x00\x00\x00\x46\x37\xd5\xd3\x20\x01\x48\x60\x00\x00\x20\x01\x00\x00\x00\x00\x00\x00\x00\x68\x05\x07\x00\x50\x22\xec\x58\x2e\x3a\xc0\x18\xc5\x50\x10\x42\x48\xb8\xb3\x00\x00";

/**
 * Example Spanning Tree Protocol (STP) packet
 */
static const uint8_t stp_packet[]="\x01\x80\xc2\x00\x00\x00\x00\x1c\x0e\x87\x85\x04\x00\x26\x42\x42\x03\x00\x00\x00\x00\x00\x80\x64\x00\x1c\x0e\x87\x78\x00\x00\x00\x00\x04\x80\x64\x00\x1c\x0e\x87\x85\x00\x80\x04\x01\x00\x14\x00\x02\x00\x0f\x00";

// functions that need prototypes
void layer_2_dispatcher(const uint8_t *, int, uint64_t);
void layer_3_dispatcher(const uint8_t *, int, uint64_t);
void layer_4_dispatcher(const uint8_t *, int, uint64_t);

/**
 * Extract protocol number (8bit version)
 *
 * @param packet_buffer raw packet captured from the network
 * @param counter protocol number offset
 * @return protocol number in host format
*/
static uint8_t protocol_8bit_extract(const uint8_t *packet_buffer, int counter) {
    return *(packet_buffer + counter);
}

/**
 * Extract protocol number (16bit version)
 *
 * @param packet_buffer raw packet captured from the network
 * @param counter protocol number offset
 * @return protocol number in host format
 */
static uint16_t protocol_16bit_extract(const uint8_t *packet_buffer, int counter) {
    return ntohs(*((uint16_t *)(packet_buffer + counter)));
}

/**
 * Extract protocol type from ethernet Destination MAC Address (48bit)
 * @param packet_buffer raw packet captured from the network
 * @param counter protocol number offset
 * @return protocol number in host format
 */
static uint64_t protocol_48bit_extract(const uint8_t *packet_buffer, int counter) {
    uint64_t value = 0;

    int i;
    for(i=0; i < 6; i++) {
	uint8_t byte =  *((uint8_t *)(packet_buffer + counter + i));

	value = byte + (value * 256);
    }

    return value;
}

/**
 * Diplay a single field of an header
 *
 * @param packet_buffer raw packet captured from the network, starting at the part to process
 * @param field_size size in bytes of the field to print
 * @param counter read bytes counter
 * @param field_text description of the field
 */
static void field_print (const uint8_t *packet_buffer, int field_size, int *counter, const char *field_text) {
    
    char *tmp_hexstr = raw_to_hexstr(packet_buffer + *counter, field_size);
    *counter += field_size;

    printf(" %-24s %s\n", tmp_hexstr, field_text);
    
    free(tmp_hexstr);
}

/**
 * Print the payload part of the packet
 *
 * @param packet_buffer raw packet captured from the network, starting at the part to process
 * @param size packet_buffer size
 */
void payload_print (const uint8_t *packet_buffer, int size) {
    
    if (size < 1) {
        return;
    }
    
    puts("\nPayload or Trailer:");

    int bytes_per_row = cols / BYTE_MULT;

    int i, j=0;

    // new line
    while (j < size) {
       
        // bytes in the line
        for (i = 0; (i < bytes_per_row) && (j < size); i++, j++) { // columns
            char str[BYTE_MULT];

            hex_to_str(packet_buffer[j], str);

            printf(" %s", str);
        }
        
        puts("");
    }

}

/**
 * Print the TCP header of the packet
 *
 * @param packet_buffer raw packet captured from the network, starting at the part to process
 * @param size packet_buffer size
 */
void tcp_print (const uint8_t *packet_buffer, int size) {
    int counter = 0;

    puts("\nTCP Header:");

    if (size < 8) {
        puts (" invalid header size");
        return;
    }

    // print header fields
    field_print(packet_buffer, 2, &counter, "Source port");
    field_print(packet_buffer, 2, &counter, "Destination port");
    field_print(packet_buffer, 4, &counter, "Sequence number");
    field_print(packet_buffer, 4, &counter, "Acknowledgement number");
    field_print(packet_buffer, 1, &counter, "Header length");
    field_print(packet_buffer, 1, &counter, "Flags");
    field_print(packet_buffer, 2, &counter, "Window");
    field_print(packet_buffer, 2, &counter, "Checksum");
    field_print(packet_buffer, 2, &counter, "Urgent pointer");

    // print remaining payload
    payload_print(packet_buffer + counter, size - counter);
}

/**
 * Print the UDP header of the packet
 *
 * @param packet_buffer raw packet captured from the network, starting at the part to process
 * @param size packet_buffer size
 */
void udp_print (const uint8_t *packet_buffer, int size) {
    int counter = 0;

    puts("\nUDP Header:");

    if (size < 8) {
        puts (" invalid header size");
        return;
    }

    // print header fields
    field_print(packet_buffer, 2, &counter, "Source port");
    field_print(packet_buffer, 2, &counter, "Destination port");
    field_print(packet_buffer, 2, &counter, "Length");
    field_print(packet_buffer, 2, &counter, "Checksum");

    // print remaining payload
    payload_print(packet_buffer + counter, size - counter);
}

/**
 * Print the ICMP header of the packet
 *
 * @param packet_buffer raw packet captured from the network, starting at the part to process
 * @param size packet_buffer size
 */
void icmp_print (const uint8_t *packet_buffer, int size) {
    int counter = 0;

    puts("\nICMP Header:");

    if (size < 8) {
        puts (" invalid header size");
        return;
    }

    // print header fields
    field_print(packet_buffer, 1, &counter, "Type");
    field_print(packet_buffer, 1, &counter, "Code");
    field_print(packet_buffer, 2, &counter, "Checksum");
    field_print(packet_buffer, 2, &counter, "ID");
    field_print(packet_buffer, 2, &counter, "Sequence number");

    // print remaining payload
    payload_print(packet_buffer + counter, size - counter);
}

/**
 * Print the ICMPv6 header of the packet
 *
 * @param packet_buffer raw packet captured from the network, starting at the part to process
 * @param size packet_buffer size
 */
void icmp6_print (const uint8_t *packet_buffer, int size) {
    int counter = 0;

    puts("\nICMPv6 Header:");

    if (size < 8) {
        puts (" invalid header size");
        return;
    }

    // print header fields
    field_print(packet_buffer, 1, &counter, "Type");
    field_print(packet_buffer, 1, &counter, "Code");
    field_print(packet_buffer, 2, &counter, "Checksum");

    // print remaining payload
    payload_print(packet_buffer + counter, size - counter);
}

/**
 * Print the IGMP header of the packet
 *
 * @param packet_buffer raw packet captured from the network, starting at the part to process
 * @param size packet_buffer size
 */
void igmp_print (const uint8_t *packet_buffer, int size) {
    int counter = 0;

    puts("\nIGMP Header:");

    if (size < 8) {
        puts (" invalid header size");
        return;
    }

    // print header fields
    field_print(packet_buffer, 1, &counter, "Type");
    field_print(packet_buffer, 1, &counter, "Max response time");
    field_print(packet_buffer, 2, &counter, "Checksum");
    field_print(packet_buffer, 4, &counter, "Group address");

    // print remaining payload
    payload_print(packet_buffer + counter, size - counter);
}

/**
 * Print the IPv4 header of the packet
 *
 * @param packet_buffer raw packet captured from the network, starting at the part to process
 * @param size packet_buffer size
 */
void ip_print (const uint8_t *packet_buffer, int size) {
    int counter = 0;

    puts("\nIPv4 Header:");

    if (size < 20) {
        puts (" invalid header size");
        return;
    }

    // print header fields
    field_print(packet_buffer, 1, &counter, "Version / Header length");
    field_print(packet_buffer, 1, &counter, "ToS / DFS");
    field_print(packet_buffer, 2, &counter, "Total length");
    field_print(packet_buffer, 2, &counter, "ID");
    field_print(packet_buffer, 2, &counter, "Flags / Fragment offset");
    field_print(packet_buffer, 1, &counter, "TTL");

    int next_protocol = protocol_8bit_extract(packet_buffer, counter);
    field_print(packet_buffer, 1, &counter, "Protocol");

    field_print(packet_buffer, 2, &counter, "Checksum");
    field_print(packet_buffer, 4, &counter, "Source address");
    field_print(packet_buffer, 4, &counter, "Destination address");

    // go up to the next layer
    layer_4_dispatcher(packet_buffer + counter, size - counter, next_protocol);
}

/**
 * Print the IPv6 header of the packet
 *
 * @param packet_buffer raw packet captured from the network, starting at the part to process
 * @param size packet_buffer size
 */
void ipv6_print (const uint8_t *packet_buffer, int size) {

    int next_protocol = 0, eh_size = 0;
    int counter = 0;

    puts("\nIPv6 Header:");

    if (size < 40) {
        puts (" invalid header size");
        return;
    }

    uint8_t buffer[8] = { 0 };
    int counter_tmp = 0;

    // print header fields

    buffer[0] = *(packet_buffer + counter) >> 4; 
    counter_tmp = 0;
    field_print(buffer, 1, &counter_tmp, "Version");

    buffer[0] = (*(packet_buffer + counter) << 4) | (*(packet_buffer + counter + 1) >> 4);
    counter_tmp = 0;
    field_print(buffer, 1, &counter_tmp, "Traffic class");

    buffer[0] = *(packet_buffer + counter + 1) & 0x0F; 
    buffer[1] = *(packet_buffer + counter + 2); 
    buffer[2] = *(packet_buffer + counter + 3); 
    counter_tmp = 0;
    field_print(buffer, 3, &counter_tmp, "Flow label");

    counter += 4;

    field_print(packet_buffer, 2, &counter, "Payload length");

    next_protocol = protocol_8bit_extract(packet_buffer, counter);
    field_print(packet_buffer, 1, &counter, "Next header");

    field_print(packet_buffer, 1, &counter, "Hop limit");

    field_print(packet_buffer, 8, &counter, "Source address");
    field_print(packet_buffer, 8, &counter, "(...)");

    field_print(packet_buffer, 8, &counter, "Destination address");
    field_print(packet_buffer, 8, &counter, "(...)");

    // print ipv6 extension headers
    while (1) {
        switch (next_protocol) {

        case 0:   // hop-by-hop options
            puts("\nIPv6 Hop-by-Hop Options:");
            goto EXTENSION_HEADER_FIELDS;

        case 43:  // routing options
            puts("\nIPv6 Routing Routing:");
            goto EXTENSION_HEADER_FIELDS;

        case 44:  // fragment options
            puts("\nIPv6 Fragment Options:");
            goto EXTENSION_HEADER_FIELDS;

        case 50:  // encapsulating security payload
            puts("\nIPv6 Encapsulating Security Payload:");
            goto EXTENSION_HEADER_FIELDS;

        case 51:  // authentication header
            puts("\nIPv6 Authentication Header:");
            goto EXTENSION_HEADER_FIELDS;

        case 60:  // destination options
            puts("\nIPv6 Destination Options:");
            goto EXTENSION_HEADER_FIELDS;

        case 135: // mobility options
            puts("\nIPv6 Mobility Options:");

EXTENSION_HEADER_FIELDS:

            if (size - counter < 8) {
                puts (" invalid header size");
                return;
            }

            next_protocol = protocol_8bit_extract(packet_buffer, counter);
            field_print(packet_buffer, 1, &counter, "Next header");

            eh_size = protocol_8bit_extract(packet_buffer, counter);
            eh_size = eh_size * 8;
            field_print(packet_buffer, 1, &counter, "Extension Header Length");

            field_print(packet_buffer, 6, &counter, "Options");

            if (size - counter < eh_size) {
                puts (" invalid header size");
                return;
            }

            int i;
            for(i = 0; i < (eh_size/8); i++) {
                field_print(packet_buffer, 8, &counter, i ? "(...)" : "Extension Header Data");
            }

            break;

        default: // go up to the next layer
            layer_4_dispatcher(packet_buffer + counter, size - counter, next_protocol);
            return;

        }
    }

}

/**
 * Print the ARP header of the packet
 *
 * @param packet_buffer raw packet captured from the network, starting at the part to process
 * @param size packet_buffer size
 */
void arp_print (const uint8_t *packet_buffer, int size) {
    int counter = 0;

    puts("\nARP Header:");

    if (size < 28) {
        puts (" invalid header size");
        return;
    }

    // print header fields
    field_print(packet_buffer, 2, &counter, "Hardware type");
    field_print(packet_buffer, 2, &counter, "Protocol type");

    int hs = *(packet_buffer + counter);
    field_print(packet_buffer, 1, &counter, "Hardware size");

    int ps = *(packet_buffer + counter);
    field_print(packet_buffer, 1, &counter, "Protocol size");

    field_print(packet_buffer, 2, &counter, "Opcode");

    field_print(packet_buffer, hs, &counter, "Sender hardware address");
    field_print(packet_buffer, ps, &counter, "Sender protocol address");
    field_print(packet_buffer, hs, &counter, "Target hardware address");
    field_print(packet_buffer, ps, &counter, "Target protocol address");

    // print remaining payload
    payload_print(packet_buffer + counter, size - counter);
}

/**
 * Print the ETHERNET header of the packet
 *
 * @param packet_buffer raw packet captured from the network, starting at the part to process
 * @param size packet_buffer size
 */
void ethernet_print (const uint8_t *packet_buffer, int size) {
    int counter = 0;

    puts("\nEthernet Header:");

    if (size < 14) {
        puts (" invalid header size");
        return;
    }

    // print header fields
    uint64_t dst_mac = protocol_48bit_extract(packet_buffer, counter);
    field_print(packet_buffer, 6, &counter, "Destination hardware address");
    field_print(packet_buffer, 6, &counter, "Source hardware address");

    int next_protocol = protocol_16bit_extract(packet_buffer, counter);
    field_print(packet_buffer, 2, &counter, "Lenght/Type");

    /*
     * if the last field value is less or equal to 1500 is a lenght
     * otherwise is a protocol type (check IEEE 802.3 documentation...)
     */

    if (next_protocol > 1500) {

	// go up to the next layer
	layer_3_dispatcher(packet_buffer + counter, size - counter, next_protocol);
    
    } else {

	// remain on the same layer
	layer_2_dispatcher(packet_buffer + counter, size - counter, dst_mac);
	
    }
}

/**
 * Print the ISL header of the packet
 *
 * @param packet_buffer raw packet captured from the network, starting at the part to process
 * @param size packet_buffer size
 */
void isl_print (const uint8_t *packet_buffer, int size) {
    int counter = 0;

    puts("\nISL Header:");

    if (size < 30) {
        puts (" invalid header size");
        return;
    }

    // print header fields
    field_print(packet_buffer, 5, &counter, "Destination");


    int next_protocol = protocol_8bit_extract(packet_buffer, counter);
    next_protocol >>= 4;

    field_print(packet_buffer, 1, &counter, "Type/User");
    field_print(packet_buffer, 6, &counter, "Source");
    field_print(packet_buffer, 2, &counter, "Length");
    field_print(packet_buffer, 1, &counter, "DSAP");
    field_print(packet_buffer, 1, &counter, "SSAP");
    field_print(packet_buffer, 1, &counter, "Control");
    field_print(packet_buffer, 3, &counter, "HSA");
    field_print(packet_buffer, 2, &counter, "Vlan ID/BPDU");
    field_print(packet_buffer, 2, &counter, "Index");
    field_print(packet_buffer, 2, &counter, "RES");

    /*
     * Note: we subtrack 4 to the size of the packet to exclude
     * the final frame check sequence
     */

    if (next_protocol == 0) {

	// go up to the next layer
	ethernet_print(packet_buffer + counter, size - counter - 4);
    
    } else {

	// go up to the next layer
	payload_print(packet_buffer + counter, size - counter - 4);
	
    }

    counter = size - 4;

    puts("\nISL Header (end):");

    field_print(packet_buffer, 4, &counter, "Frame check seq.");
}

/**
 * Print the DTP header of the packet
 *
 * @param packet_buffer raw packet captured from the network, starting at the part to process
 * @param size packet_buffer size
 */
void dtp_print (const uint8_t *packet_buffer, int size) {
    int counter = 0;

    puts("\nDinamic Trunking Protocol Header:");

    if (size < 29) {
        puts (" invalid header size");
        return;
    }

    // print header fields
    field_print(packet_buffer, 1, &counter, "Version");
    field_print(packet_buffer, 8, &counter, "Domain");

    field_print(packet_buffer, 5, &counter, "Status");
    field_print(packet_buffer, 5, &counter, "DTP Type");

    field_print(packet_buffer, 8, &counter, "Neighbor");
    field_print(packet_buffer, 2, &counter, ""); // splitted since too long...

    // print remaining payload
    payload_print(packet_buffer + counter, size - counter);
}

/**
 * Print the STP header of the packet
 *
 * @param packet_buffer raw packet captured from the network, starting at the part to process
 * @param size packet_buffer size
 */
void stp_print (const uint8_t *packet_buffer, int size) {
    int counter = 0;

    puts("\nSpanning Tree Protocol Header:");

    if (size < 35) {
        puts (" invalid header size");
        return;
    }

    // print header fields
    field_print(packet_buffer, 2, &counter, "Protocol Identifier");
    field_print(packet_buffer, 1, &counter, "Protocol Version Identifier");

    field_print(packet_buffer, 1, &counter, "BPDU Type");
    field_print(packet_buffer, 1, &counter, "BPDU Flags");

    field_print(packet_buffer, 2, &counter, "Root Priority/System ID Extension");
    field_print(packet_buffer, 6, &counter, "Root System ID");

    field_print(packet_buffer, 4, &counter, "Root Path Cost");

    field_print(packet_buffer, 2, &counter, "Bridge Priority/System ID Extension");
    field_print(packet_buffer, 6, &counter, "Bridge System ID");

    field_print(packet_buffer, 2, &counter, "Port Identifier");
    field_print(packet_buffer, 2, &counter, "Message Age");
    field_print(packet_buffer, 2, &counter, "Max Age");
    field_print(packet_buffer, 2, &counter, "Hello Time");
    field_print(packet_buffer, 2, &counter, "Forward Delay");

    // print remaining payload
    payload_print(packet_buffer + counter, size - counter);
}

/**
 * Print the LLC header of the packet
 *
 * @param packet_buffer raw packet captured from the network, starting at the part to process
 * @param size packet_buffer size
 */
void llc_print (const uint8_t *packet_buffer, int size) {
    int counter = 0;

    puts("\nLogical-Link Control Header:");

    if (size < 3) {
        puts (" invalid header size");
        return;
    }

    // print header fields
    int dsap = protocol_8bit_extract(packet_buffer, counter);
    field_print(packet_buffer, 1, &counter, "DSAP");

    int ssap = protocol_8bit_extract(packet_buffer, counter);
    field_print(packet_buffer, 1, &counter, "SSAP");

    field_print(packet_buffer, 1, &counter, "Control field");

    if (dsap == 0x42 && ssap == 0x42) {

	// spanning tree protocol
	stp_print(packet_buffer + counter, size - counter);

    } else if (dsap == 0xaa && ssap == 0xaa) {
    
	if (size < 8) {
	    puts (" invalid header size");
	    return;
	}

	// continue printing LLC fields
	field_print(packet_buffer, 3, &counter, "Organization code");
	
	int pid = protocol_16bit_extract(packet_buffer, counter);
	field_print(packet_buffer, 2, &counter, "PID");

	if (pid == 0x2004) {

	    // dinamic trunking protocol
	    dtp_print(packet_buffer + counter, size - counter);

	} else {

	    // print remaining payload
	    payload_print(packet_buffer + counter, size - counter);

	}

    }
	
}

/**
 * Determine the packet type and call the appropriate function to disassemble it.
 * Operates on layer 2 (OSI model) packet's headers.
 *
 * @param packet_buffer raw packet captured from the network, starting at layer 2
 * @param size packet_buffer size
 * @param protocol protocol number
 */
void layer_2_dispatcher (const uint8_t *packet_buffer, int size, uint64_t protocol) {

    uint64_t llc1 = 0x0180C20000LLU, llc2 = 0x01000CCCCCCCLLU;

    if (size < 1) {
        return;
    }

    if (memcmp(packet_buffer, "\x01\x00\x0C\x00\x00", 5)==0 ||
	memcmp(packet_buffer, "\x03\x00\x0c\x00\x00", 5)==0 ) {

	isl_print(packet_buffer, size);

    } else if ((protocol / 256) == llc1) {

	llc_print(packet_buffer, size); // spanning tree

    } else if (protocol == llc2) {

	llc_print(packet_buffer, size);

    } else {

	ethernet_print(packet_buffer, size);

    }

}

/**
 * Determine the packet type and call the appropriate function to disassemble it.
 * Operates on layer 3 (OSI model) packet's headers.
 *
 * @param packet_buffer raw packet captured from the network, starting at layer 3
 * @param size packet_buffer size
 * @param protocol protocol number
 */
void layer_3_dispatcher (const uint8_t *packet_buffer, int size, uint64_t protocol) {

    if (size < 1) {
        return;
    }

    /*
     * if the last field value (of an ethernet header) is less or equal to 1500
     * then is a lenght otherwise is a protocol type (check IEEE 802.3 documentation...)
     */

    if (protocol <= 0xffff) { // check if it's a 16bit field
	                      // (i.e. last ethernet field was a protocol)
        switch (protocol) {

            case 0x0800: ip_print(packet_buffer, size); break;
            case 0x0806: arp_print(packet_buffer, size); break;
            case 0x86dd: ipv6_print(packet_buffer, size); break;

            default: payload_print(packet_buffer, size);

        }

    } else {
	payload_print(packet_buffer, size);
    }

}

/**
 * Determine the packet type and call the appropriate function to disassemble it.
 * Operates on layer 4 (OSI model) packet's headers.
 *
 * @param packet_buffer raw packet captured from the network, starting at layer 4
 * @param size packet_buffer size
 * @param protocol protocol number
 */
void layer_4_dispatcher (const uint8_t *packet_buffer, int size, uint64_t protocol) {
    
    if (size < 1) {
        return;
    }

    switch (protocol) {
        case 0x01: icmp_print(packet_buffer, size); break;
        case 0x02: igmp_print(packet_buffer, size); break;
        case 0x04: ip_print(packet_buffer, size); break;
        case 0x06: tcp_print(packet_buffer, size); break;
        case 0x11: udp_print(packet_buffer, size); break;
        case 0x29: ipv6_print(packet_buffer, size); break;
        case 0x3A: icmp6_print(packet_buffer, size); break;
        default: payload_print(packet_buffer, size);
    }

}


#endif /* __PRETTYPACKET_H__ */

