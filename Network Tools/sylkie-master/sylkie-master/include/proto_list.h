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

#ifndef SYLKIE_INCLUDE_HEADERS_H
#define SYLKIE_INCLUDE_HEADERS_H

#include <sys/types.h>

#include "errors.h"

enum sylkie_proto_type {
    SYLKIE_ETH = 0,
    SYLKIE_IPv6,
    SYLKIE_ICMPv6, // Usefull to ensure checksums are calculated correctly
    SYLKIE_DATA,
    SYLKIE_INVALID_HDR_TYPE,
};

struct sylkie_proto {
    enum sylkie_proto_type type;
    void* data;
    size_t len;
};

struct sylkie_proto_node {
    struct sylkie_proto hdr;
    struct sylkie_proto_node* next;
    struct sylkie_proto_node* prev;
};

struct sylkie_proto_list {
    struct sylkie_proto_node* head;
    struct sylkie_proto_node* tail;
};

int sylkie_proto_init(struct sylkie_proto* hdr, enum sylkie_proto_type type,
                      void* data, size_t len);

void sylkie_proto_node_free(struct sylkie_proto_node* node);

struct sylkie_proto_list* sylkie_proto_list_init();

enum sylkie_error sylkie_proto_list_add(struct sylkie_proto_list* lst,
                                        enum sylkie_proto_type type, void* data,
                                        size_t len);

enum sylkie_error sylkie_proto_list_add_node(struct sylkie_proto_list* lst,
                                             struct sylkie_proto_node* node);

enum sylkie_error sylkie_proto_list_rm_node(struct sylkie_proto_list* lst,
                                            struct sylkie_proto_node* node);

enum sylkie_error sylkie_proto_list_rm(struct sylkie_proto_list* lst,
                                       enum sylkie_proto_type type);

void sylkie_proto_list_free(struct sylkie_proto_list* lst);

#define SYLKIE_HEADER_LIST_FOREACH(lst, node)                                  \
    for (node = lst->head; node; node = node->next)

#endif
