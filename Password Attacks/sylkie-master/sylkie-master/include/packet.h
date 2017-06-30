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

#ifndef SYLKIE_INCLUDE_PACKET_H
#define SYLKIE_INCLUDE_PACKET_H

#include <stdbool.h>
#include <stdlib.h>

#include "proto_list.h"

struct sylkie_packet_cache {
    struct sylkie_buffer* buf;
    bool dirty;
};

struct sylkie_packet {
    struct sylkie_proto_list* lst;
    struct sylkie_packet_cache cache;
};

struct sylkie_packet* sylkie_packet_init();

enum sylkie_error sylkie_packet_add(struct sylkie_packet* pkt,
                                    enum sylkie_proto_type type, void* data,
                                    size_t sz);

struct sylkie_buffer* sylkie_packet_to_buffer(struct sylkie_packet* pkt,
                                              enum sylkie_error* err);

void sylkie_packet_free(struct sylkie_packet* pkt);

#endif
