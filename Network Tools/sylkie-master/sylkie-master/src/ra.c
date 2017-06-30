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

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <nd.h>
#include <sender.h>

#include <arpa/inet.h>
#include <unistd.h>

#include <sylkie_config.h>

#ifdef BUILD_JSON
#include <json-c/json.h>
#endif

#include <utils.h>

void ra_usage(FILE* fd) {
    fprintf(
        fd,
        "Usage: sylkie ra [OPTION]\n"
        "Send ICMPv6 Router Advertisement messages to the given address\n\n"
        "Available options:\n"
        " -h | --help        this\n"
        " -i | --interface   interface that will be used to send packets\n"
        " -s | --src-mac     source address in ethernet frame\n"
        " -d | --dst-mac     destination address in ethernet frame\n"
        " -S | --src-ip      source ipv6 address in IP header\n"
        " -D | --dst-ip      destination ipv6 address in IP header\n"
        " -t | --target-mac  link-layer address used as value for the target\n"
        "                    address option of the Neighbor Advertisement\n"
        " -r | --repeat      send the packet <num> times\n"
        " -z | --timeout     wait <seconds> before sending the packet again\n");
}

int inner_do_ra(const char* iface_name, u_int8_t* src_mac, u_int8_t* dst_mac,
                u_int8_t* tgt_mac, struct in6_addr* src_addr,
                struct in6_addr* dst_addr, struct in6_addr* tgt_addr,
                u_int8_t prefix, int repeat, int timeout) {
    int i, res = 0, retval = 0;
    struct in6_addr dst_addr_opt;
    u_int8_t dst_mac_opt[ETH_ALEN] = {0x33, 0x33, 0x00, 0x00, 0x00, 0x01};
    enum sylkie_error err = SYLKIE_INVALID_ERR;
    struct sylkie_sender* sender = NULL;
    struct sylkie_packet* pkt = NULL;
    if (!iface_name) {
        fprintf(stderr, "Must provide an interface to use.\n");
        ra_usage(stderr);
        _exit(-1);
    } else if ((!dst_mac && dst_addr) || (dst_mac && !dst_addr)) {
        fprintf(stderr, "Must provide a destination mac and ip"
                        " address or none at all.\n");
        ra_usage(stderr);
        _exit(-1);
    } else if (!src_addr && !tgt_addr) {
        fprintf(stderr, "Must provide a source or target ip address.\n");
        ra_usage(stderr);
        _exit(-1);
    }

    sender = sylkie_sender_init(iface_name, &err);

    if (err || !sender) {
        fprintf(stderr, "%s\n", sylkie_strerror(err));
        _exit(-1);
    }

    if (!dst_addr) {
        inet_pton(AF_INET6, "ff02::1", &dst_addr_opt);
        dst_addr = &dst_addr_opt;
        dst_mac = dst_mac_opt;
    }

    if (!tgt_addr) {
        tgt_addr = src_addr;
    }

    if (!src_addr) {
        src_addr = tgt_addr;
    }

    if (!src_mac) {
        src_mac = sender->addr.sll_addr;
    }

    if (!tgt_mac) {
        tgt_mac = src_mac;
    }

    pkt = sylkie_router_advert_create(src_mac, dst_mac, src_addr, dst_addr,
                                      tgt_addr, prefix, tgt_mac, &err);

    if (!pkt) {
        fprintf(stderr, "%s\n", "Could not allocate packet");
        _exit(-1);
    }

    if (repeat <= 0) {
        while (1) {
            res = sylkie_sender_send_packet(sender, pkt, 0, &err);
            if (err || res < 0) {
                fprintf(stderr, "SYLKIE Error:%s\nErrno: %s",
                        sylkie_strerror(err), strerror(errno));
                retval = -1;
                break;
            }

            if (timeout) {
                sleep(timeout);
            }
        }
    } else {
        for (i = 0; i < repeat; ++i) {
            res = sylkie_sender_send_packet(sender, pkt, 0, &err);
            if (err || res < 0) {
                fprintf(stderr, "SYLKIE Error:%s\nErrno: %s",
                        sylkie_strerror(err), strerror(errno));
                retval = -1;
                break;
            }

            if (timeout) {
                sleep(timeout);
            }
        }
    }

    // Cleanup

    sylkie_packet_free(pkt);
    sylkie_sender_free(sender);
    return retval;
}

#ifdef BUILD_JSON
pid_t ra_json(struct json_object* jobj) {
    pid_t pid = -1;
    int res = 0, repeat = 1, timeout = 0;
    u_int8_t prefix = 64;
    const char* tmp;
    const char* iface_name = NULL;
    u_int8_t* src_mac = NULL;
    u_int8_t* dst_mac = NULL;
    u_int8_t* tgt_mac = NULL;
    u_int8_t src_mac_opt[8], dst_mac_opt[8], tgt_mac_opt[8];
    struct in6_addr src_addr_opt, dst_addr_opt, tgt_addr_opt;
    struct in6_addr* src_addr = NULL;
    struct in6_addr* dst_addr = NULL;
    struct in6_addr* tgt_addr = NULL;
    enum json_type type;
    json_object_object_foreach(jobj, key, val) {
        type = json_object_get_type(val);
        if (strcmp(key, "interface") == 0) {
            if (type == json_type_string) {
                iface_name = json_object_get_string(val);
            } else {
                fprintf(stderr, "Invalid type for interface\n");
                return -1;
            }
        } else if (strcmp(key, "src-mac") == 0) {
            if (type == json_type_string) {
                tmp = json_object_get_string(val);
                res = parse_hwaddr(tmp, src_mac_opt);
                if (res < 0) {
                    fprintf(stderr, "Could not parse mac: %s\n", tmp);
                    return -1;
                }
                src_mac = src_mac_opt;
            } else {
                fprintf(stderr, "Invalid type for src-mac\n");
                return -1;
            }
        } else if (strcmp(key, "dst-mac") == 0) {
            if (type == json_type_string) {
                tmp = json_object_get_string(val);
                res = parse_hwaddr(tmp, dst_mac_opt);
                if (res < 0) {
                    fprintf(stderr, "Could not parse mac: %s\n", tmp);
                    return -1;
                }
                dst_mac = dst_mac_opt;
            } else {
                fprintf(stderr, "Invalid type for dst-mac\n");
                return -1;
            }
        } else if (strcmp(key, "target-mac") == 0) {
            if (type == json_type_string) {
                tmp = json_object_get_string(val);
                res = parse_hwaddr(tmp, tgt_mac_opt);
                if (res < 0) {
                    fprintf(stderr, "Could not parse mac: %s\n", tmp);
                    return -1;
                }
                tgt_mac = tgt_mac_opt;
            } else {
                fprintf(stderr, "Invalid type for target-mac\n");
                return -1;
            }
        } else if (strcmp(key, "src-ip") == 0) {
            if (type == json_type_string) {
                tmp = json_object_get_string(val);
                res = inet_pton(AF_INET6, tmp, &src_addr_opt);
                if (res <= 0) {
                    fprintf(stderr, "Could not parse %s - %s\n", tmp,
                            strerror(errno));
                    ra_usage(stderr);
                    _exit(-1);
                }
                src_addr = &src_addr_opt;
            } else {
                fprintf(stderr, "Invalid type for src-ip\n");
                return -1;
            }
        } else if (strcmp(key, "dst-ip") == 0) {
            if (type == json_type_string) {
                tmp = json_object_get_string(val);
                res = inet_pton(AF_INET6, tmp, &dst_addr_opt);
                if (res <= 0) {
                    fprintf(stderr, "Could not parse %s - %s\n", tmp,
                            strerror(errno));
                    ra_usage(stderr);
                    _exit(-1);
                }
                dst_addr = &dst_addr_opt;
            } else {
                fprintf(stderr, "Invalid type for dst-ip\n");
                return -1;
            }
        } else if (strcmp(key, "router-ip") == 0) {
            if (type == json_type_string) {
                tmp = json_object_get_string(val);
                res = inet_pton(AF_INET6, tmp, &tgt_addr_opt);
                if (res <= 0) {
                    fprintf(stderr, "Could not parse %s - %s\n", tmp,
                            strerror(errno));
                    ra_usage(stderr);
                    _exit(-1);
                }
                tgt_addr = &tgt_addr_opt;
            } else {
                fprintf(stderr, "Invalid type for dst-ip\n");
                return -1;
            }
        } else if (strcmp(key, "prefix") == 0) {
            if (type == json_type_int) {
                res = json_object_get_int(val);
                if (res <= 128) {
                    prefix = (u_int8_t)res;
                } else {
                    fprintf(stderr, "%d is too large for an ipv6 prefix\n",
                            res);
                    return -1;
                }
            }
        } else if (strcmp(key, "repeat") == 0) {
            if (type == json_type_int) {
                repeat = json_object_get_int(val);
            } else {
                fprintf(stderr, "Invalid type for repeat\n");
                return -1;
            }
        } else if (strcmp(key, "timeout") == 0) {
            if (type == json_type_int) {
                timeout = json_object_get_int(val);
            } else {
                fprintf(stderr, "Invalid type for timeout\n");
                return -1;
            }
        } else {
            fprintf(stderr, "Unknown key: %s\n", key);
            return -1;
        }
    }

    if ((pid = fork()) < 0) {
        return -1;
    } else if (pid == 0) {
        if (lockdown()) {
            fprintf(stderr, "Could not lock down the process\n");
            _exit(-1);
        }
        res = inner_do_ra(iface_name, src_mac, dst_mac, tgt_mac, src_addr,
                          dst_addr, tgt_addr, prefix, repeat, timeout);
        exit(res);
    } else {
        return pid;
    }
}
#endif

int ra_cmdline(int argc, const char** argv) {
    int i = 1, res = -1, repeat = 1, timeout = 0;
    u_int8_t prefix = 64;
    u_int8_t* src_mac = NULL;
    u_int8_t* dst_mac = NULL;
    u_int8_t* tgt_mac = NULL;
    u_int8_t src_mac_opt[8], dst_mac_opt[8], tgt_mac_opt[8];
    const char* iface_name = NULL;
    struct in6_addr src_addr_opt, dst_addr_opt, tgt_addr_opt;
    struct in6_addr* src_addr = NULL;
    struct in6_addr* dst_addr = NULL;
    struct in6_addr* tgt_addr = NULL;

    if (argc < 2) {
        fprintf(stderr, "Too few arguments\n");
        ra_usage(stderr);
        _exit(-1);
    }

    while (i < argc) {
        if (ARG_CMP(argv[i], "-h", "--help")) {
            ra_usage(stdout);
            _exit(0);
        } else if (!(i + 1 <= argc)) {
            fprintf(stderr, "%s requires an argument\n", argv[i]);
            ra_usage(stderr);
            _exit(-1);
        } else if (ARG_CMP(argv[i], "-i", "--interface")) {
            ++i;
            iface_name = argv[i];
        } else if (ARG_CMP(argv[i], "-s", "--src-mac")) {
            ++i;
            res = parse_hwaddr(argv[i], src_mac_opt);
            if (res < 0) {
                fprintf(stderr, "Could not parse mac: %s\n", argv[i]);
                _exit(-1);
            }
            src_mac = src_mac_opt;
        } else if (ARG_CMP(argv[i], "-d", "--dst-mac")) {
            ++i;
            res = parse_hwaddr(argv[i], dst_mac_opt);
            if (res < 0) {
                fprintf(stderr, "Could not parse mac: %s\n", argv[i]);
                _exit(-1);
            }
            dst_mac = dst_mac_opt;
        } else if (ARG_CMP(argv[i], "-t", "--target-mac")) {
            ++i;
            res = parse_hwaddr(argv[i], tgt_mac_opt);
            if (res < 0) {
                fprintf(stderr, "Could not parse mac: %s\n", argv[i]);
                _exit(-1);
            }
            tgt_mac = tgt_mac_opt;
        } else if (ARG_CMP(argv[i], "-S", "--src-ip")) {
            ++i;
            res = inet_pton(AF_INET6, argv[i], &src_addr_opt);
            if (res <= 0) {
                fprintf(stderr, "Could not parse %s - %s\n", argv[i],
                        strerror(errno));
                ra_usage(stderr);
                _exit(-1);
            }
            src_addr = &src_addr_opt;
        } else if (ARG_CMP(argv[i], "-D", "--dst-ip")) {
            ++i;
            res = inet_pton(AF_INET6, argv[i], &dst_addr_opt);
            if (res <= 0) {
                fprintf(stderr, "Could not parse %s - %s\n", argv[i],
                        strerror(errno));
                ra_usage(stderr);
                _exit(-1);
            }
            dst_addr = &dst_addr_opt;
        } else if (ARG_CMP(argv[i], "-R", "--router-ip")) {
            ++i;
            res = inet_pton(AF_INET6, argv[i], &tgt_addr_opt);
            if (res <= 0) {
                fprintf(stderr, "Could not parse %s - %s\n", argv[i],
                        strerror(errno));
                ra_usage(stderr);
                _exit(-1);
            }
            tgt_addr = &tgt_addr_opt;
        } else if (ARG_CMP(argv[i], "-p", "--prefix")) {
            ++i;
            res = atoi(argv[i]);
            if (res <= 0 || res >= 128) {
                fprintf(stderr, "%s is not a valid prefix value", argv[i]);
                ra_usage(stderr);
                _exit(-1);
            }
            prefix = (u_int8_t)res;
        } else if (ARG_CMP(argv[i], "-r", "--repeat")) {
            ++i;
            repeat = atoi(argv[i]);
            if (repeat <= 0) {
                if (strcmp(argv[i], "-1") == 0) {
                    repeat = -1;
                } else {
                    fprintf(stderr, "%s is not a valid repeat value", argv[i]);
                    ra_usage(stderr);
                    _exit(-1);
                }
            }
        } else if (ARG_CMP(argv[i], "-z", "--timeout")) {
            ++i;
            timeout = atoi(argv[i]);
            if (timeout < 0) {
                fprintf(stderr, "%s is not a valid timout value", argv[i]);
                ra_usage(stderr);
                _exit(-1);
            }
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            ra_usage(stderr);
            _exit(-1);
        }
        ++i;
    }
    return inner_do_ra(iface_name, src_mac, dst_mac, tgt_mac, src_addr,
                       dst_addr, tgt_addr, prefix, repeat, timeout);
}
