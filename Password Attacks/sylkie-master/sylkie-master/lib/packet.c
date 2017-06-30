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

#include <errors.h>
#include <nd.h>
#include <packet.h>
#include <proto_list.h>

static enum sylkie_error
sylkie_generic_to_buffer(struct sylkie_buffer* buf,
                         const struct sylkie_packet* pkt,
                         const struct sylkie_proto_node* node) {
    if (node && !sylkie_buffer_add(buf, node->hdr.data, node->hdr.len)) {
        return SYLKIE_SUCCESS;
    } else {
        return SYLKIE_ERR_FATAL;
    }
}

static struct {
    enum sylkie_proto_type type;
    enum sylkie_error (*fn)(struct sylkie_buffer* buf,
                            const struct sylkie_packet* pkt,
                            const struct sylkie_proto_node* node);
} s_to_buffer_cmds[] = {
    {SYLKIE_ETH, sylkie_generic_to_buffer},
    {SYLKIE_IPv6, sylkie_generic_to_buffer},
    {SYLKIE_ICMPv6, sylkie_icmpv6_to_buffer},
    {SYLKIE_DATA, sylkie_generic_to_buffer},
};

struct sylkie_buffer*
sylkie_packet_cache_refresh(struct sylkie_packet_cache* cache,
                            struct sylkie_buffer* buf) {
    cache->buf = sylkie_buffer_clone(buf);
    if (cache->buf) {
        cache->dirty = false;
        return cache->buf;
    }
    return NULL;
}

struct sylkie_packet* sylkie_packet_init() {
    struct sylkie_packet* pkt = malloc(sizeof(struct sylkie_packet));
    pkt->lst = sylkie_proto_list_init();
    pkt->cache.buf = NULL;
    pkt->cache.dirty = true;
    return pkt;
}

enum sylkie_error sylkie_packet_add(struct sylkie_packet* pkt,
                                    enum sylkie_proto_type type, void* data,
                                    size_t sz) {
    pkt->cache.dirty = true;
    return sylkie_proto_list_add(pkt->lst, type, data, sz);
}

struct sylkie_buffer* sylkie_packet_to_buffer(struct sylkie_packet* pkt,
                                              enum sylkie_error* err) {
    struct sylkie_buffer* buf = NULL;
    struct sylkie_proto_node* node = NULL;

    if (!pkt->cache.dirty) {
        if (err) {
            *err = SYLKIE_SUCCESS;
        }
        return sylkie_buffer_clone(pkt->cache.buf);
    }

    buf = sylkie_buffer_init(0);
    if (!buf) {
        if (err) {
            *err = SYLKIE_NO_MEM;
        }
    }

    SYLKIE_HEADER_LIST_FOREACH(pkt->lst, node) {
        if (node->hdr.type < SYLKIE_INVALID_HDR_TYPE &&
            s_to_buffer_cmds[node->hdr.type].fn) {
            (s_to_buffer_cmds[node->hdr.type].fn)(buf, pkt, node);
        }
    }

    if (!sylkie_packet_cache_refresh(&pkt->cache, buf)) {
        if (err) {
            *err = SYLKIE_NO_MEM;
        }
    }

    return buf;
}

void sylkie_packet_free(struct sylkie_packet* pkt) {
    if (pkt) {
        if (pkt->cache.buf) {
            sylkie_buffer_free(pkt->cache.buf);
        }
        sylkie_proto_list_free(pkt->lst);
        free(pkt);
        pkt = NULL;
    }
}
