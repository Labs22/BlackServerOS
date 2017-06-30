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

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>

#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <packet.h>
#include <sender.h>

struct sylkie_sender* sylkie_sender_init(const char* iface,
                                         enum sylkie_error* err) {
    struct ifreq ifr;
    int name_len = strlen(iface);
    struct sylkie_sender* sender = malloc(sizeof(struct sylkie_sender));

    if (!sender) {
        if (err) {
            *err = SYLKIE_NO_MEM;
        }
        return NULL;
    }

    if ((sender->fd = socket(AF_PACKET, SOCK_RAW, ETH_P_ALL)) < 0) {
        if (err) {
            *err = SYLKIE_SYSCALL_FAILED;
        }
        free(sender);
        return NULL;
    }

    memset(&sender->addr, 0, sizeof(struct sockaddr_ll));
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, iface, name_len);
    if (ioctl(sender->fd, SIOCGIFHWADDR, &ifr)) {
        if (err) {
            *err = SYLKIE_SYSCALL_FAILED;
        }
        close(sender->fd);
        free(sender);
        return NULL;
    }
    sender->addr.sll_halen = ETH_ALEN;
    memcpy(&sender->addr.sll_addr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);

    if (ioctl(sender->fd, SIOCGIFMTU, &ifr)) {
        if (err) {
            *err = SYLKIE_SYSCALL_FAILED;
        }
        close(sender->fd);
        free(sender);
        return NULL;
    }
    sender->mtu = ifr.ifr_mtu;

    if (ioctl(sender->fd, SIOCGIFINDEX, &ifr)) {
        if (err) {
            *err = SYLKIE_SYSCALL_FAILED;
        }
        close(sender->fd);
        free(sender);
        return NULL;
    }
    sender->addr.sll_ifindex = ifr.ifr_ifindex;

    if (err) {
        *err = SYLKIE_SUCCESS;
    }
    return sender;
}

int sylkie_sender_send(struct sylkie_sender* sender, const u_int8_t* data,
                       size_t len, int flags, enum sylkie_error* err) {
    int res;
    if (!sender) {
        if (err) {
            *err = SYLKIE_NULL_INPUT;
        }
        return 0;
    }
    if ((res = sendto(sender->fd, data, len, flags,
                      (struct sockaddr*)&sender->addr,
                      sizeof(struct sockaddr_ll))) < 0) {
        if (err) {
            *err = SYLKIE_SYSCALL_FAILED;
        }
    } else if (err) {
        *err = SYLKIE_SUCCESS;
    }
    return res;
}

int sylkie_sender_send_buffer(struct sylkie_sender* sender,
                              const struct sylkie_buffer* buf, int flags,
                              enum sylkie_error* err) {
    return sylkie_sender_send(sender, buf->data, buf->len, flags, err);
}

int sylkie_sender_send_packet(struct sylkie_sender* sender,
                              struct sylkie_packet* pkt, int flags,
                              enum sylkie_error* err) {
    int res = 0;
    struct sylkie_buffer* buf = NULL;
    if (!pkt) {
        if (err) {
            *err = SYLKIE_NULL_INPUT;
        }
        return -1;
    }

    buf = sylkie_packet_to_buffer(pkt, err);

    if (buf) {
        if (buf->len > sender->mtu) {
            if (err) {
                *err = SYLKIE_TOO_LARGE;
            }
            return -1;
        } else {
            res = sylkie_sender_send_buffer(sender, buf, flags, err);
            sylkie_buffer_free(buf);
            return res;
        }
    } else {
        if (err) {
            *err = SYLKIE_NOT_FOUND;
        }
        return -1;
    }
}

// Helpful for debugging
void sylkie_print_sender(struct sylkie_sender* sender) {
    if (!sender) {
        return;
    }
    int i;
    struct sockaddr_ll* dev = &sender->addr;
    printf("sender (%p)\nfd: %d\nmtu: %d\naddr:\n", sender, sender->fd,
           sender->mtu);
    printf("\tindex: %d\n\thalen: %d\n\tmac: ", dev->sll_ifindex,
           dev->sll_halen);
    for (i = 0; i < dev->sll_halen - 1; ++i) {
        printf("%02x:", dev->sll_addr[i]);
    }
    printf("%02x\n", dev->sll_addr[i]);
}

void sylkie_sender_free(struct sylkie_sender* sender) {
    if (sender) {
        if (sender->fd >= 0) {
            close(sender->fd);
        }
        free(sender);
        sender = NULL;
    }
}
