#!/bin/bash

set -e

CLIENT_SRC_DIR="../client"
SERVER_SRC_DIR="../server"

# In most cases sudo is not needed on OS X since postgres runs
# as the user who installed postgres via Homebrew.
SUDO_POSTGRES='sudo -u postgres'
case $(uname)
in
  Darwin) SUDO_POSTGRES='' ;;
esac

# create crypton test database user if it doesn't exist
echo "select * from pg_user where usename = 'crypton_test_user';" \
  | $SUDO_POSTGRES psql template1 | grep -q crypton_test_user || {
    echo "Creating Crypton test user..."
    $SUDO_POSTGRES psql template1 <<EOF
create user crypton_test_user with encrypted password 'crypton_test_user_password';
EOF
}

# create a .pgpass file if not present
[ -e ~/.pgpass ] || {
  echo "Creating ~/.pgpass file..."
  touch ~/.pgpass
  chmod 600 ~/.pgpass
}

# add entry to pgpass for test user to make it easy to use psql, etc.
grep -q crypton_test_user ~/.pgpass || {
  echo "Adding  Crypton database information to ~/.pgpass file..."
  cat >> ~/.pgpass <<EOF
localhost:*:crypton_test:crypton_test_user:crypton_test_user_password
EOF
}

# create crypton_test db if it doesn't exist
$SUDO_POSTGRES psql -l | grep -q crypton_test || {
  echo "Creating Crypton test database..."
  $SUDO_POSTGRES createdb -O crypton_test_user crypton_test
}

# add the schema if nothing is there
# options from recommendation http://petereisentraut.blogspot.com/2010/03/running-sql-scripts-with-psql.html
echo "select * from pg_tables" \
  | psql -h localhost -U crypton_test_user crypton_test \
  | grep -q account || {
    echo "Adding schema to Crypton test database..."
    PGOPTIONS='--client-min-messages=warning' \
      psql -X -q -v ON_ERROR_STOP=1 --pset pager=off \
      -h localhost -U crypton_test_user crypton_test \
      -f $SERVER_SRC_DIR/lib/stores/postgres/sql/setup.sql
  }

# add crypton-dev.local to /etc/hosts
grep -q crypton-dev.local /etc/hosts || {
  echo "Adding crypton-dev.local to /etc/hosts..."
  sudo bash -c 'echo -e "127.0.0.1\tcrypton-dev.local\n" >> /etc/hosts'
}
