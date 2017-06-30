#!/bin/bash

PKG_NAME=$1

set -e
set -x

if [[ -e deb/${PKG_NAME:?} ]]; then 
    rm -rf deb/$PKG_NAME
fi
mkdir -p deb/$PKG_NAME
cd deb/$PKG_NAME
apt-get source -b $PKG_NAME
DEBIAN_FRONTEND=noninteractive dpkg -i *.deb
