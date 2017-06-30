#!/bin/sh
cd libdisasm/src/arch/i386/libdisasm/ && make && cd ../../../../../ && make
echo
echo
echo Now you need to set your LD_LIBRARY_PATH to include the path to libdisasm.so
echo
