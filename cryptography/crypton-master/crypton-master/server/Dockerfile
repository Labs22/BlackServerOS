from node:0.10.32
run mkdir -p /usr/src/app
workdir /usr/src/app
copy . /usr/src/app
run rm -rf /usr/src/app/node_modules
run npm install
expose 1025
entrypoint [ "node", "/usr/src/app/bin/cli.js" ]
cmd [ "run", "-d" ]
