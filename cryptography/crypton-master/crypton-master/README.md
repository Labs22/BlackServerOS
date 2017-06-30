![Crypton logo](https://i.imgur.com/2Q5Qpek.png)


[![NPM version](https://img.shields.io/npm/v/crypton-server.svg?style=flat)](https://npmjs.org/package/crypton-server)
[![Build Status](https://img.shields.io/travis/SpiderOak/crypton.svg?style=flat)](https://travis-ci.org/SpiderOak/crypton)


Crypton is a framework for creating zero-knowledge applications with JavaScript.

Zero-Knowledge applications offer meaningful privacy assurance to end users
because the servers running the application cannot read the data created and
stored by the application.

To learn more, check out the [Crypton website](https://spideroak.com/solutions/crypton-framework).

# Install dependencies

## Linux

### Install [Redis](http://redis.io)

"The suggested way of installing Redis is compiling it from sources as Redis has no dependencies other than a working GCC compiler and libc. Installing it using the package manager of your Linux distribution is somewhat discouraged as usually the available version is not the latest." - [Redis documentation](http://redis.io/topics/quickstart#installing-redis)

```
wget http://download.redis.io/redis-stable.tar.gz
tar xvzf redis-stable.tar.gz
cd redis-stable
make
make test
sudo make install
```

Configuring and setting up redis to run is an environment specific problem
and left as an exercise for the user. You may now be able to just simply
do `redis-server &`

### Install PostgreSQL Database

[PostgreSQL](http://www.postgresql.org/download/linux/) supports a variety of
Linux versions. One of these install commands may work for you:

- [Red Hat](http://www.postgresql.org/download/linux/redhat/): `yum install postgresql-server`

- [Debian](http://www.postgresql.org/download/linux/debian/): `apt-get install postgresql-9.4`

- [Ubuntu](http://www.postgresql.org/download/linux/ubuntu/): `sudo apt-get install postgresql postgresql-contrib`

- [SuSE](http://www.postgresql.org/download/linux/suse/): See the [official instructions](http://www.postgresql.org/download/linux/suse/) for more detail

## OS X

These installation instructions were run on a system running OS X `El Capitan` (version 10.11) and all dependencies were installed using the latest software versions available from Homebrew at the time this was written (10/2015). These instructions assume you have `git` and `homebrew` already installed.

### Install [Redis](http://redis.io) using [Homebrew](http://brew.sh)

````
brew install redis
````

After installation follow the instructions to copy the `launchd` plist file and setup Redis to launch at boot time.


### Install PostgreSQL Database using [Homebrew](http://brew.sh)

````
brew install postgresql
````

After installation follow the instructions to copy the `launchd` plist file and launch PostgreSQL at boot time.

#### Create a new `postgres` database user using the `createuser` command.

`createuser` is installed as part of the `postgresql` install. This command will
create a new user called `postgres` that has superuser role permissions and
can create roles.

````
createuser --superuser --createrole postgres
````

## OS Common

### Install the [Node.js Version Manager](https://github.com/creationix/nvm) and Node.js

See the [nvm](https://github.com/creationix/nvm) website for detailed installation instructions. Crypton currently expects Node.js version `4.2.2` to be installed which is the most current LTS release.

````
curl -o- https://raw.githubusercontent.com/creationix/nvm/v0.30.2/install.sh | bash

nvm install 4.2.2

# optional - set this as your default node install
nvm alias default v4.2.2
````

Verify your `node` installation:

````
$ which node
~/.nvm/versions/node/v4.2.2/bin/node

$ which npm
~/.nvm/versions/node/v4.2.2/bin/npm
````

# Build the client and server

## Clone a local copy of Crypton

````
git clone https://github.com/SpiderOak/crypton.git
cd crypton
nvm use
````

***Important*** : If you are using `nvm` enable the use of the correct Node.js version after you enter the `crypton` directory by typing `nvm use` which will reference the `.nvmrc` file which is already present in the root of the repository. This will setup the correct version of `node` and `npm`.

You need to do this ***every*** time you `cd` into the `crypton` dir in every terminal shell you open.

## Install `npm` global packages used for development:

````
npm install -g karma-cli
````

## Configure your master postgres database user:

You need to configure your environment specific config files for crypton to use
the postgres database user that has superuser permissions. Edit the following
files:

```
server/config/config.development.json
server/config/config.test.json

# create as needed from a copy of the other config files:
server/config/config.production.json
```

In those files you need to change the `postgres_user` and `postgres_user_password`
keys as appropriate for your environment. The user you configure must have
postgres superuser permissions.

e.g.
```
"database": {
   "type": "postgres",
   "postgres_user": "postgres",        <-- CHANGE AS NEEDED
   "postgres_user_password": "",       <-- CHANGE AS NEEDED
   "user": "crypton_development_user",
   "password": "crypton_development_user_password",
   "database": "crypton_development",
   "host": "localhost",
   "port": 5432
 },
 ...
```

The default is username `postgres` with an empty password.

NOTE: If you are on Ubuntu Linux please note that you must have a username
and password set for a postgres superuser and you need to ensure you can connect to the DB with `psql -U username`. You need to be able to do this
without using `sudo` to first shell out to the `postgres` user that is created by
the default installation scripts on Ubuntu. [This page might be of interest](https://stackoverflow.com/questions/7695962/postgresql-password-authentication-failed-for-user-postgres).

## Test that all Crypton dependencies are installed

````
./check_dependencies.sh
````

If the output is OK then continue.

## Build the Crypton Javascript client

````
cd crypton/client
npm install
./compile.sh --once
````

`compile.sh` will place a `crypton.js` file in the `crypton/client/dist/` directory, this is the JavaScript file you'll use in your client applications.


## Build the Crypton Node.js Server and setup the DB

The `db:init` command creates a `crypton_ENVIRONMENT_user` DB user, `crypton_ENVIRONMENT` database, and the appropriate DB schema in PostgreSQL based on your `NODE_ENV`.

````
cd crypton/server
npm install
npm link
NODE_ENV=development crypton-server db:init
NODE_ENV=test crypton-server db:init
````

# Test the client and server

## Test the Crypton client

Mocha + PhantomJS Tests

````
cd crypton/client
make test
````

Mocha + Browser Tests

You may first want to edit `crypton/client/karma.conf.js` to specify which (or all) of Chrome, Firefox, and Safari to test.  When you run `karma` it will launch all of the selected browsers and run the tests in each.

````
cd crypton/client
karma start --single-run
````

## Test the Crypton server

Mocha Tests

````
cd crypton/server
make test
````

# Running the Crypton server

## In the foreground

The default server configurations can be found in `crypton/server/config/config.test.json`.

````
NODE_ENV=development crypton-server run
````

Use `Control-c` to exit.

## In the background

The default server configurations can be found in `crypton/server/config/config.test.json`.

````
export NODE_ENV=development
crypton-server start
crypton-server status
crypton-server tail
crypton-server stop
````

The full set of commands can be seen with `crypton-server -h`

````
$ crypton-server -h

  Usage: crypton-server [options] [command]

  Commands:

    run                    Run the server
    db:init                Initialize the database
    db:drop                Drop the database
    status                 Print the status of the daemonized server
    start                  Daemonize the server
    stop                   Stop the daemonized server
    restart                Restart the daemonized server
    logs                   Print the latest server logs
    tail                   Tail the server logs to STDOUT
    cleanlogs              Remove the server logs

  Options:

    -h, --help           output usage information
    -V, --version        output the version number
    -c, --config [file]  Specify a custom configuration file [default config]
    -d, --docker         Enable Docker mode to use environment variables instead of a config file [default false]

````

## Test the server is running

Verify the server is running with `curl` or a browser. You may need to use `--insecure` mode which skips self-signed TLS certificate checks. Crypton ships with a self-signed certificate for testing purposes only. You will definitely want to use your own real TLS certificates!

````
curl --insecure -i https://localhost:1025
````

## Test a Crypton example application

````
cd client/examples/items
NODE_ENV=development crypton-server run
````

Open your browser to the URL [https://localhost:1025/](https://localhost:1025/) and register a new account via the form. Since the server uses a self-signed certificate by default you may need to instruct your browser to accept and trust this `localhost` only certificate. You can find the instructions for Safari [here](http://blog.marcon.me/post/24874118286/secure-websockets-safari).

Once you are registered and logged in, open the JS console in your browser. See the original version of these instructions [here](https://github.com/SpiderOak/crypton/wiki/Quick-Start-&-Items-API:-%5BBETA-docs%5D).  The following is just a quick smoke-test that things are working as expected. You should continue to the Crypton docs for more info and examples.

This creates an empty Item called 'myitem'

````
app.session.getOrCreateItem('myitem', function callback(err, item) {console.log(arguments)});
````

Add values to this Item. This will save the item immediately, no callback required.

````
app.session.items.myitem.value = {foo: 1, bar: 2};
````
