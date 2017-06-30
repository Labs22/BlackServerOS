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

#ifndef SYLKIE_INCLUDE_SENDER_H
#define SYLKIE_INCLUDE_SENDER_H

#include <linux/if_packet.h>
#include <sys/types.h>

#include "buffer.h"
#include "errors.h"
#include "packet.h"

struct sylkie_sender {
    int fd;
    int mtu;
    struct sockaddr_ll addr;
};

struct sylkie_sender* sylkie_sender_init(const char* iface,
                                         enum sylkie_error* err);

int sylkie_sender_send_buffer(struct sylkie_sender* sender,
                              const struct sylkie_buffer* buf, int flags,
                              enum sylkie_error* err);

int sylkie_sender_send_packet(struct sylkie_sender* sender,
                              struct sylkie_packet* pkt, int flags,
                              enum sylkie_error* err);

int sylkie_sender_send(struct sylkie_sender* sender, const u_int8_t* data,
                       size_t len, int flags, enum sylkie_error* err);

void sylkie_sender_free(struct sylkie_sender* sender);

void sylkie_print_sender(struct sylkie_sender* sender);

#endif
