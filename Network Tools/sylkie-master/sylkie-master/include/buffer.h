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

#ifndef SYLKIE_INCLUDE_BUFFER_H
#define SYLKIE_INCLUDE_BUFFER_H

#include <sys/types.h>

#define sylkie sylkie

struct sylkie_buffer {
    size_t len;
    size_t cap;
    u_int8_t* data;
};

struct sylkie_buffer* sylkie_buffer_init(size_t sz);

int sylkie_buffer_add(struct sylkie_buffer* buf, const void* data, size_t len);

int sylkie_buffer_add_value(struct sylkie_buffer* buf, const u_int8_t data,
                            size_t len);

struct sylkie_buffer* sylkie_buffer_clone(const struct sylkie_buffer* buf);

void sylkie_buffer_print(const struct sylkie_buffer* buf);

void sylkie_buffer_free(struct sylkie_buffer* buf);

#endif
