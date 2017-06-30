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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <buffer.h>

struct sylkie_buffer* sylkie_buffer_init(size_t sz) {
    struct sylkie_buffer* buf = malloc(sizeof(struct sylkie_buffer));
    if (buf == NULL) {
        return NULL;
    } else if (sz) {
        buf->len = 0;
        buf->cap = sz;
        buf->data = malloc(sz);
        if (buf->data == NULL) {
            free(buf);
            return NULL;
        } else {
            return buf;
        }
    } else {
        memset(buf, 0, sizeof(struct sylkie_buffer));
        return buf;
    }
}

static int maybe_realloc(struct sylkie_buffer* buf, size_t len) {
    size_t newcap = 0;
    u_int8_t* tmp = NULL;

    // Do not continue for zero length buffers
    if (!len || !buf) {
        return -1;
    }

    newcap = buf->len + len;
    if (newcap > buf->cap) {
        if (buf->data) {
            tmp = realloc(buf->data, newcap);
        } else {
            tmp = malloc(newcap);
        }
        if (tmp) {
            buf->cap = newcap;
            buf->data = tmp;
            return 0;
        } else {
            return -1;
        }
    } else {
        return 0;
    }
}

int sylkie_buffer_add(struct sylkie_buffer* buf, const void* data, size_t len) {
    if (maybe_realloc(buf, len)) {
        return -1;
    } else {
        memcpy(buf->data + buf->len, data, len);
        buf->len += len;
        return 0;
    }
}

int sylkie_buffer_add_value(struct sylkie_buffer* buf, const u_int8_t data,
                            size_t len) {
    if (maybe_realloc(buf, len)) {
        return -1;
    } else {
        memset(buf->data + buf->len, data, len);
        buf->len += len;
        return 0;
    }
}

struct sylkie_buffer* sylkie_buffer_clone(const struct sylkie_buffer* buf) {
    struct sylkie_buffer* other = sylkie_buffer_init(buf->cap);
    if (other) {
        other->len = buf->len;
        memcpy(other->data, buf->data, buf->len);
        return other;
    }
    return NULL;
}

void sylkie_buffer_print(const struct sylkie_buffer* buf) {
    if (buf && buf->data) {
        int i;
        for (i = 0; i < (buf->len - 1); ++i) {
            if ((i + 1) % 16 == 0 && i > 0) {
                printf("%02x\n", (int)((u_int8_t)buf->data[i]));
            } else {
                printf("%02x ", (int)((u_int8_t)buf->data[i]));
            }
        }
        printf("%02x\n", (int)((u_int8_t)buf->data[i]));
    }
    printf("\n");
}

void sylkie_buffer_free(struct sylkie_buffer* buf) {
    if (buf) {
        if (buf->data) {
            free(buf->data);
        }
        free(buf);
        buf = NULL;
    }
}
