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
#include <string.h>
#include <strings.h>

#include <proto_list.h>

// sylkie_proto_node methods

static void rm_proto_node(struct sylkie_proto_list* lst,
                          struct sylkie_proto_node* node) {
    if (node->next) {
        if (node == lst->head) {
            lst->head = node->next;
        }
        node->next->prev = node->prev;
    }
    if (node->prev) {
        if (node == lst->tail) {
            lst->tail = node->prev;
        }
        node->prev->next = node->next;
    }
}

int sylkie_proto_init(struct sylkie_proto* hdr, enum sylkie_proto_type type,
                      void* data, size_t len) {
    hdr->type = type;
    if (len) {
        hdr->len = len;
        hdr->data = malloc(len);
        if (!hdr->data) {
            return -1;
        }
        memcpy(hdr->data, data, len);
    } else {
        hdr->len = 0;
        hdr->data = NULL;
    }
    return 0;
}

void sylkie_proto_node_free(struct sylkie_proto_node* node) {
    if (node) {
        if (node->hdr.data) {
            free(node->hdr.data);
        }
        free(node);
        node = NULL;
    }
}

struct sylkie_proto_list* sylkie_proto_list_init() {
    struct sylkie_proto_list* lst = malloc(sizeof(struct sylkie_proto_list));
    bzero(lst, sizeof(struct sylkie_proto_list));
    return lst;
}

enum sylkie_error sylkie_proto_list_add(struct sylkie_proto_list* lst,
                                        enum sylkie_proto_type hdr_type,
                                        void* data, size_t len) {
    if (lst) {
        struct sylkie_proto_node* node =
            malloc(sizeof(struct sylkie_proto_node));
        if (node) {
            if (sylkie_proto_init(&node->hdr, hdr_type, data, len)) {
                free(node);
                return SYLKIE_NO_MEM;
            }
            return sylkie_proto_list_add_node(lst, node);
        } else {
            return SYLKIE_NO_MEM;
        }
    } else {
        return SYLKIE_NO_MEM;
    }
}

enum sylkie_error sylkie_proto_list_add_node(struct sylkie_proto_list* lst,
                                             struct sylkie_proto_node* node) {
    struct sylkie_proto_node* tmp = NULL;
    if (lst->head && lst->tail) {
        // Other items have been added
        tmp = lst->tail;
        lst->tail = node;
        node->next = NULL;
        node->prev = tmp;
        tmp->next = node;
    } else {
        // We are the first node to be added
        lst->head = node;
        lst->tail = node;
        node->next = NULL;
        node->prev = NULL;
    }
    return SYLKIE_SUCCESS;
}

enum sylkie_error sylkie_proto_list_rm_node(struct sylkie_proto_list* lst,
                                            struct sylkie_proto_node* node) {
    struct sylkie_proto_node* tmp = NULL;
    if (!lst || !node) {
        return SYLKIE_NULL_INPUT;
    } else {
        SYLKIE_HEADER_LIST_FOREACH(lst, node) {
            if (node == tmp) {
                rm_proto_node(lst, tmp);
                sylkie_proto_node_free(tmp);
                return SYLKIE_SUCCESS;
            }
        }
        return SYLKIE_NOT_FOUND;
    }
}

enum sylkie_error sylkie_proto_list_rm(struct sylkie_proto_list* lst,
                                       enum sylkie_proto_type type) {
    struct sylkie_proto_node* node;
    if (!lst || !lst->head || !lst->tail) {
        return SYLKIE_NULL_INPUT;
    } else {
        SYLKIE_HEADER_LIST_FOREACH(lst, node) {
            if (node->hdr.type == type) {
                rm_proto_node(lst, node);
                sylkie_proto_node_free(node);
                return SYLKIE_SUCCESS;
            }
        }
        return SYLKIE_NOT_FOUND;
    }
}

void sylkie_proto_list_free(struct sylkie_proto_list* lst) {
    struct sylkie_proto_node* node;
    struct sylkie_proto_node* tmp;
    if (lst) {
        if (lst->head && lst->tail) {
            node = lst->head;
            while (node != lst->tail) {
                tmp = node->next;
                sylkie_proto_node_free(node);
                node = tmp;
            }
            sylkie_proto_node_free(lst->tail);
        }
        free(lst);
        lst = NULL;
    }
}
