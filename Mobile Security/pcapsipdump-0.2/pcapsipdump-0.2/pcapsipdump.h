/*
    This file is part of pcapsipdump

    This file is based on linux kernel, namely:
    - udp.h by Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
    - ip.h by Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>

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

#define PCAPSIPDUMP_VERSION "0.2"

struct iphdr {
#if defined(__LITTLE_ENDIAN)
	uint8_t	ihl:4,
		version:4;
#elif defined (__BIG_ENDIAN)
	uint8_t	version:4,
  		ihl:4;
#else
#error Endian not defined
#endif
	uint8_t	tos;
	uint16_t	tot_len;
	uint16_t	id;
	uint16_t	frag_off;
	uint8_t	ttl;
	uint8_t	protocol;
	uint16_t	check;
	uint32_t	saddr;
	uint32_t	daddr;
	/*The options start here. */
};


struct udphdr {
	uint16_t	source;
	uint16_t	dest;
	uint16_t	len;
	uint16_t	check;
};

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

struct ether_header
{
  uint8_t  ether_dhost[ETH_ALEN];	/* destination eth addr	*/
  uint8_t  ether_shost[ETH_ALEN];	/* source ether addr	*/
  uint16_t ether_type;		        /* packet type ID field	*/
} __attribute__ ((__packed__));
