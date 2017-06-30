// Copyright (c) 2017 Daniel L. Robertson
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <stdlib.h>
#include <strings.h>

#include <nd.h>

#include <stdio.h>

static struct sylkie_packet*
sylkie_base_packet_create(const u_int8_t eth_src[ETH_ALEN],
                          const u_int8_t eth_dst[ETH_ALEN],
                          struct in6_addr* ip_src, struct in6_addr* ip_dst,
                          u_int16_t len, enum sylkie_error* err) {
    struct ethhdr eth;
    struct ip6_hdr ipv6;
    enum sylkie_error local_err;
    struct sylkie_packet* pkt = sylkie_packet_init();
    if (!pkt) {
        if (err) {
            *err = SYLKIE_NULL_INPUT;
        }
        return NULL;
    }

    memcpy(&eth.h_dest, eth_dst, ETH_ALEN);
    memcpy(&eth.h_source, eth_src, ETH_ALEN);
    eth.h_proto = htons(ETH_P_IPV6);

    local_err = sylkie_packet_add(pkt, SYLKIE_ETH, &eth, sizeof(struct ethhdr));
    if (local_err) {
        if (err) {
            *err = local_err;
        }
        return NULL;
    }

    bzero(&ipv6, sizeof(struct ip6_hdr));
    ipv6.ip6_vfc = 0x60;
    ipv6.ip6_nxt = 0x3a;
    ipv6.ip6_hops = 0xff;
    ipv6.ip6_plen =
        len ? htons(len + sizeof(struct icmp6_hdr)) : sizeof(struct icmp6_hdr);

    memcpy(&ipv6.ip6_dst, ip_dst, sizeof(struct in6_addr));
    memcpy(&ipv6.ip6_src, ip_src, sizeof(struct in6_addr));

    local_err =
        sylkie_packet_add(pkt, SYLKIE_IPv6, &ipv6, sizeof(struct ip6_hdr));
    if (local_err) {
        if (err) {
            *err = local_err;
        }
        return NULL;
    }

    if (err) {
        *err = SYLKIE_SUCCESS;
    }
    return pkt;
}

struct sylkie_packet* sylkie_neighbor_advert_create(
    const u_int8_t eth_src[ETH_ALEN], const u_int8_t eth_dst[ETH_ALEN],
    struct in6_addr* ip_src, struct in6_addr* ip_dst, struct in6_addr* tgt_ip,
    const u_int8_t tgt_hw[ETH_ALEN], enum sylkie_error* err) {
    enum sylkie_error local_err;
    static const u_int8_t options[2] = {0x02, 0x01};
    struct sylkie_packet* pkt = NULL;
    struct icmp6_hdr icmpv6;
    struct sylkie_buffer* buf =
        sylkie_buffer_init(sizeof(struct in6_addr) + 2 + ETH_ALEN);

    sylkie_buffer_add(buf, tgt_ip, sizeof(struct in6_addr));

    sylkie_buffer_add(buf, options, sizeof(options));

    sylkie_buffer_add(buf, tgt_hw, ETH_ALEN);

    bzero(&icmpv6, sizeof(struct icmp6_hdr));

    icmpv6.icmp6_type = ND_NEIGHBOR_ADVERT;
    icmpv6.icmp6_data8[0] = 0x20;

    pkt = sylkie_base_packet_create(eth_src, eth_dst, ip_src, ip_dst, buf->len,
                                    err);
    if (!pkt) {
        return NULL;
    }

    local_err = sylkie_packet_add(pkt, SYLKIE_ICMPv6, &icmpv6,
                                  sizeof(struct icmp6_hdr));
    if (local_err) {
        if (err) {
            *err = local_err;
        }
        return NULL;
    }

    local_err = sylkie_packet_add(pkt, SYLKIE_DATA, buf->data, buf->len);
    if (local_err) {
        if (err) {
            *err = local_err;
        }
        return NULL;
    }

    sylkie_buffer_free(buf);

    return pkt;
}

struct sylkie_packet* sylkie_router_advert_create(
    const u_int8_t eth_src[ETH_ALEN], const u_int8_t eth_dst[ETH_ALEN],
    struct in6_addr* ip_src, struct in6_addr* ip_dst, struct in6_addr* tgt_ip,
    u_int8_t prefix, const u_int8_t tgt_hw[ETH_ALEN], enum sylkie_error* err) {
    struct icmp6_hdr icmpv6;
    enum sylkie_error local_err;
    struct sylkie_buffer* buf = sylkie_buffer_init(2 + ETH_ALEN + 8);
    struct sylkie_packet* pkt = NULL;
    static const u_int8_t source_options[2] = {0x01, 0x01};
    static const u_int8_t prefix_options[2] = {0x03, 0x04};

    bzero(&icmpv6, sizeof(struct icmp6_hdr));

    icmpv6.icmp6_type = ND_ROUTER_ADVERT;
    sylkie_buffer_add_value(buf, 0x00, 8);
    sylkie_buffer_add(buf, prefix_options, sizeof(prefix_options));
    sylkie_buffer_add_value(buf, prefix, 1);
    sylkie_buffer_add_value(buf, 0x00, 13);
    sylkie_buffer_add(buf, tgt_ip, sizeof(struct in6_addr));
    sylkie_buffer_add(buf, source_options, sizeof(source_options));
    sylkie_buffer_add(buf, tgt_hw, ETH_ALEN);

    pkt = sylkie_base_packet_create(eth_src, eth_dst, ip_src, ip_dst, buf->len,
                                    err);
    if (!pkt) {
        return NULL;
    }

    local_err = sylkie_packet_add(pkt, SYLKIE_ICMPv6, &icmpv6,
                                  sizeof(struct icmp6_hdr));
    if (local_err) {
        if (err) {
            *err = local_err;
        }
        return NULL;
    }

    local_err = sylkie_packet_add(pkt, SYLKIE_DATA, buf->data, buf->len);
    if (local_err) {
        if (err) {
            *err = local_err;
        }
        return NULL;
    }

    sylkie_buffer_free(buf);

    return pkt;
}

static void icmp_set_checksum(const struct sylkie_proto_node* node) {
    struct ip6_hdr* ipv6;
    struct icmp6_hdr* icmpv6;
    u_int32_t sum = 0;
    u_int16_t* tmp;
    int i;
    int len;

    if (!node->prev || !node->prev->hdr.data ||
        node->prev->hdr.type != SYLKIE_IPv6) {
        return;
    }

    ipv6 = node->prev->hdr.data;
    tmp = (u_int16_t*)&ipv6->ip6_src;
    for (i = 0; i < (sizeof(struct in6_addr) / 2); ++i) {
        sum += tmp[i];
    }

    tmp = (u_int16_t*)&ipv6->ip6_dst;
    for (i = 0; i < (sizeof(struct in6_addr) / 2); ++i) {
        sum += tmp[i];
    }

    sum += ipv6->ip6_plen;
    sum += htons(ipv6->ip6_nxt);

    if (node->hdr.type != SYLKIE_ICMPv6 || !node->hdr.data) {
        return;
    }

    icmpv6 = node->hdr.data;
    icmpv6->icmp6_cksum = 0;
    tmp = (u_int16_t*)icmpv6;
    for (i = 0; i < (node->hdr.len / 2); ++i) {
        sum += tmp[i];
    }

    if (node->next) {
        if (node->next->hdr.type != SYLKIE_DATA || !node->next->hdr.data) {
            return;
        }

        tmp = (u_int16_t*)node->next->hdr.data;
        len = node->next->hdr.len;
        for (i = 0; i < len / 2; ++i) {
            sum += tmp[i];
        }
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += sum >> 16;

    icmpv6->icmp6_cksum = ~sum;
}

enum sylkie_error
sylkie_icmpv6_to_buffer(struct sylkie_buffer* buf,
                        const struct sylkie_packet* pkt,
                        const struct sylkie_proto_node* node) {
    if (node) {
        icmp_set_checksum(node);
        if (!sylkie_buffer_add(buf, node->hdr.data, node->hdr.len)) {
            return SYLKIE_SUCCESS;
        } else {
            return SYLKIE_ERR_FATAL;
        }
    } else {
        return SYLKIE_ERR_FATAL;
    }
}
