#!/bin/bash

ALL_NODES="crypton1"

. ${BASE_DIR:=$(cd $(dirname $0) ; cd .. ; readlink -f $(pwd))}/scripts/util.sh
PY_ENV_DIR=$BASE_DIR/pyenv

set -e
set -x

prepare_container_host
sudo chmod +rx /var/lib/lxc
get_lxc_host_addr

create_nodes "$ALL_NODES"
allow_ubuntu_user_to_sudo "$ALL_NODES"

# make it possible for us to ssh into the node
for node in $ALL_NODES ; do
    add_key_file $node
done
start_nodes "$ALL_NODES"

# determine the DHCP address the node got, add it to /etc/hosts
for node in $ALL_NODES ; do
    grep -q $node /etc/hosts || {
        logfile=/var/lib/lxc/$node/rootfs/var/log/syslog
        while : ; do
            grep -sq "bound to" $logfile && break
            sleep 1
        done
        container_ip=$(perl -ne '/dhclient: bound to (\S+)/; print "$1\n";' $logfile | tail -n 1)
        echo "${container_ip:?} $node # lxc container" | sudo tee -a /etc/hosts
    }
done

update_ssh_known_hosts "$ALL_NODES"

generate_dev_secrets

cd $BASE_DIR
ansible-playbook \
    -i hosts.dev \
    site.yml "$@"

set +x
echo "If everything went well, you should be able to point your browser"
echo "to use the HTTP proxy inside the container on "
echo "$container_ip port 3128.  Use for all"
echo "protocols.  With that proxy, try the demo apps at: "
echo "  https://server.cryptondev.local/examples/diary"
echo "  https://server.cryptondev.local/examples/chat3"
