#! /bin/sh

#postgresql checks
if which psql > /dev/null; then
  echo "Found psql..."
else
  echo "Postgres is not installed, psql not located"
  exit 1;
fi

# sed regular expression usage varies by platform.
# do this to allow this script to work on both linux (-r, GNU sed)
# and BSD / Mac OS X (-E). (Or re-write to use something other than sed)
SED_EXTENDED_REGEXP_FLAG=-r
case $(uname)
in
  *BSD) SED_EXTENDED_REGEXP_FLAG=-E ;;
  Darwin) SED_EXTENDED_REGEXP_FLAG=-E ;;
esac

POSTGRES_VERSION="$(psql -V | sed $SED_EXTENDED_REGEXP_FLAG 's/.+9.[0-9]+.[0-9]+$/9.X.X/')"
if [ "$POSTGRES_VERSION" = "9.X.X" ]; then
  VERSION=`psql -V`
  echo "Found postgres $VERSION"
else
  echo "Postgres 9.x is required for Crypton"
  echo "On Linux please install the latest postgres server package"
  echo "On Mac OS X please install postgres via Homebrew or Postgresql.app"
  exit 1;
fi


# In most cases sudo is not needed on OS X since postgres runs
# as the user who installed postgres via Homebrew.
SUDO_POSTGRES='sudo -u postgres'
case $(uname)
in
  Darwin) SUDO_POSTGRES='' ;;
esac

echo "Checking for the 'postgres' user, may require user input..."
POSTGRES_USER_EXISTS="$($SUDO_POSTGRES psql --user postgres -c "\du" | grep -o postgres)"
if [ $POSTGRES_USER_EXISTS ]; then
  echo "Found postgres user..."
else
  echo "Configuration issue: the postgres user does not exist in the database, please create it:"
  echo "  createuser --superuser --createrole postgres"
  echo "is one possible way to create the postgres superuser"
  exit 1;
fi

# node checks
# Try node first as nodejs, for systems (like ubuntu) where 'node' is the
# amateur packet radio program:
if which nodejs > /dev/null; then
  NODECMD=nodejs
  echo "Found Node.js as $NODECMD..."
elif which node > /dev/null; then
  NODECMD=node
  echo "Found Node.js as $NODECMD..."
else
  echo "Node.js is not installed, \`which nodejs\` and \`which node\` failed"
  exit 1;
fi

NODE_VERSION_0="$($NODECMD --version | grep -o "0.12.")"
NODE_VERSION_4="$($NODECMD --version | grep -o "4.2.")"

if [ "$NODE_VERSION_4" = "4.2." ] || [ "$NODE_VERSION_0" = "0.12." ]; then
  echo "Found a supported Node.js version..."
else
  echo "Node.js >= 4.2.x is required for Crypton"
  exit 1;
fi

# redis-server checks
if which redis-server >/dev/null; then
  echo "Found redis-server..."
else
  echo "Redis is not installed, \`which redis-server\` failed"
  echo "Please install Redis 2.6.x, 2.8.x, or 3.0.x"
  exit 1;
fi

REDIS_VERSION=$(redis-server --version | grep -o -E "2\.6|2\.8|3\.0" | wc -c | awk {'print $1'})
if [ "$REDIS_VERSION" -ne "0"  ]; then
  echo "Found supported redis version..."
else
  echo "Redis 2.6.x, 2.8.x or 3.0.x are required for Crypton"
  echo "You have: `redis-server --version`"
  exit 1;
fi

echo "Crypton dependencies seem to be correctly installed!"
