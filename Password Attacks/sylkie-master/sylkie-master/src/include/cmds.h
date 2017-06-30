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

#ifndef SYLKIE_SRC_INCLUDE_CMDS_H
#define SYLKIE_SRC_INCLUDE_CMDS_H

#include <sylkie_config.h>

#ifdef BUILD_JSON
#include <json-c/json.h>
#endif

// various subcommands to the sylkie command

// neighbor advert subcommand functions
int na_cmdline(int argc, const char** argv);
#ifdef BUILD_JSON
pid_t na_json(struct json_object* jobj);
#endif

// router advert subcommand functions
int ra_cmdline(int argc, const char** argv);
#ifdef BUILD_JSON
pid_t ra_json(struct json_object* jobj);
#endif

#endif
