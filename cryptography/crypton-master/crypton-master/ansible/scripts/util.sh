#!/bin/bash

function generate_dev_secrets() {
    cd $BASE_DIR

    if [[ ! -e dev/secrets/cookie_secret.key ]]; then
        dd if=/dev/urandom of=dev/secrets/cookie_secret.key bs=2048 count=1
        chmod 400 dev/secrets/cookie_secret.key
    fi

    grep -sq 'END DH PARAMETERS' dev/secrets/cryptondev.local-wildcard.pem || {
        cd dev/secrets
        openssl genrsa -aes256 -passout pass:x \
            -out cryptondev.local-wildcard.pass.key 2048
        openssl rsa -passin pass:x \
            -in cryptondev.local-wildcard.pass.key \
            -out cryptondev.local-wildcard.key
        openssl req -new \
            -subj "/C=US/ST=Local/L=Local/O=IT/CN=*.cryptondev.local" \
            -key cryptondev.local-wildcard.key \
            -out cryptondev.local-wildcard.csr
        openssl x509 -req -days 3650 -in cryptondev.local-wildcard.csr \
            -signkey cryptondev.local-wildcard.key \
            -out cryptondev.local-wildcard.crt
        cat cryptondev.local-wildcard.crt \
            cryptondev.local-wildcard.key \
            > cryptondev.local-wildcard.pem
        openssl dhparam 1024 >> cryptondev.local-wildcard.pem
        cd $BASE_DIR
    }
}

function add_key_file () {
    node=$1

    keyfile_homedir=/var/lib/lxc/${node:?}/rootfs/home/ubuntu
    keyfile_dir=$keyfile_homedir/.ssh
    keyfile_path=$keyfile_dir/authorized_keys
    passwd_path=/var/lib/lxc/${node}/rootfs/etc/passwd 
    guest_uid=$(sudo grep ubuntu $passwd_path | awk -F: '{print $3}')

    # track down an appropriate source key file
    keyfile_src=$HOME/.ssh/authorized_keys 
    if [[ ! -e $keyfile_src ]]; then
        for check in id_rsa.pub id_dsa.pub ; do
            keyfile_src=$HOME/.ssh/$check
            if [[ -e $keyfile_src ]]; then
                break
            fi
        done
    fi
    if [[ ! -e $keyfile_src ]]; then
        echo "Could not find ssh key files to place on nodes."
        echo "Please generate ssh keys."
        echo "Use the ssh-keygen command."
        exit 1
    fi


    sudo bash <<EOF
    if [[ ! -d $keyfile_homedir ]]; then
        echo keyfile homedir $keyfile_homedir not found
        exit 1
    fi

    if [[ ! -d $keyfile_dir ]]; then
        mkdir -v $keyfile_dir
        chown -v $guest_uid:$guest_uid $keyfile_dir
        chmod -v 700 $keyfile_dir
    fi

    if [[ ! -s $keyfile_path ]]; then
        cat $keyfile_src | sudo bash -c "cat - >$keyfile_path"
    fi
    chown -v $guest_uid:$guest_uid $keyfile_path
    chmod -v 400 $keyfile_path
EOF
}

function start_nodes () {
    nodes="$1"
    for node in $nodes
    do
        sudo lxc-info -n $node | grep RUNNING || {
            echo starting $node
            sudo lxc-start -n $node \
                --daemon \
                /sbin/init
        }
        sudo lxc-wait -n $node -s RUNNING
    done

}

function destroy_nodes () {
    nodes="$1"
    for node in $nodes
    do
        sudo lxc-destroy -n $node
    done
}

function stop_nodes () {
    nodes="$1"

    # ask them nicely to shut down
    if [[ -x /usr/bin/lxc-stop ]]; then
        stop_cmd=/usr/bin/lxc-stop 
    elif [[ -x /usr/bin/lxc-stutdown ]]; then
        stop_cmd=/usr/bin/lxc-shutdown
    else
        echo "could not find lxc stop command"
        exit 1
    fi

    for node in $nodes
    do
        sudo lxc-info -n $node | grep RUNNING && sudo $stop_cmd -n $node
    done

    # wait in parallel for all nodes to shut down, and stop them if they
    # timeout
    for node in $nodes
    do
        ( sudo lxc-wait -t 30 -s STOPPED -n $node \
          || sudo lxc-stop -n $node ) &
    done
    echo "Waiting for nodes to shut down..."
    wait

    # be really sure they shut down
    for node in $nodes
    do
        sudo lxc-wait -t 30 -s STOPPED -n $node
    done
}

function create_nodes () {
    nodes="$1"
    for n in $nodes
    do
        distro_release=precise

        # create containers for nodes that don't have them
        if [[ ! -d /var/lib/lxc/$n ]]; then
            sudo lxc-create -n $n -t ubuntu -- -r $distro_release --trim
        fi

    done
}

function get_lxc_host_addr() {
    lxc_host_addr=$(ifconfig lxcbr0 | perl -ne 'print "$1" if /inet addr:([0-9.]+)/;')
    echo Host LXC address is ${lxc_host_addr:?}
    export lxc_host_addr
}

function prepare_container_host () {
    # host dependencies

    sudo apt-get -y install moreutils

    sudo apt-get -y install libvirt-bin
    sudo apt-get -y install python-virtualenv
    sudo apt-get -y install python-pip
    sudo apt-get -y install python-yaml
    sudo apt-get -y install python-dev
    sudo apt-get -y install lxc lxc-templates lxctl

    # virtual py env on host
    if [[ ! -d ${PY_ENV_DIR:?} ]]; then
        mkdir $PY_ENV_DIR
        virtualenv $PY_ENV_DIR
    fi
    . $PY_ENV_DIR/bin/activate
    # TODO make these use hash verification, and then a always-fail proxy to
    # prevent downloads. (although newer PIP does at least use SSL somewhat
    # correctly)
    python -c 'import jinja2' || pip install jinja2
    python -c 'import pretty' || pip install pretty
    python -c 'import ansible' || {
        if [[ ! -d ~/git ]]; then mkdir ~/git ; fi
        [ -d ~/git/ansible ] || \
            git clone \
                https://github.com/ansible/ansible.git \
                --branch release1.4.1 \
                ~/git/ansible
        pip install ~/git/ansible
    }

}

function update_ssh_known_hosts () {

    if [[ ! -e ~/.ssh/known_hosts ]]; then
        return 0
    fi

    nodes="$1"
    for n in $nodes 
    do
        ssh-keygen -R $n
        ssh-keygen -F $n \
            | grep -q found \
            || {
              ssh -O exit ubuntu@$n || true
              ssh -o StrictHostKeyChecking=no ubuntu@$n hostname
            }
    done
}

function allow_ubuntu_user_to_sudo () {
    nodes="$1"
    for n in $nodes
    do
        if [[ -e /var/lib/lxc/$n/rootfs/etc/sudoers.d/ubuntu_user_sudo ]]; 
        then
            continue
        fi

        echo 'ubuntu ALL = (ALL)NOPASSWD: ALL' \
            | sudo sponge /var/lib/lxc/$n/rootfs/etc/sudoers.d/ubuntu_user_sudo 
        sudo chmod 0440 /var/lib/lxc/$n/rootfs/etc/sudoers.d/ubuntu_user_sudo
        sudo chown 0:0 /var/lib/lxc/$n/rootfs/etc/sudoers.d/ubuntu_user_sudo 
    done
}
