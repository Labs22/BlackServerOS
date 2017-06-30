# capture the current OS name so we can do
# some basic conditionals for OS X vs. Linux.
UNAME := $(shell uname -s)

test: test-integration

test-integration: node_modules kill-server reset build-client run-server
	@echo "Running integration tests..."
	#@./node_modules/phantomjs/bin/phantomjs --ignore-ssl-errors=true ./node_modules/mocha-phantomjs/lib/mocha-phantomjs.coffee test/index.html; SUCCESS=$$?; \
	$(MAKE) kill-server; \
	exit $$SUCCESS

node_modules:
	@$(MAKE) -C ../server node_modules
	@echo "Installing node modules for integration tests..."
	@npm install

setup-test-environment:
	@echo "Setting up test database..."
	@./bin/setup_test_environment.sh

clean-test-db:
	@echo "Cleaning test database..."
ifeq ($(UNAME), Linux)
	-@sudo -u postgres dropdb crypton_test
endif
ifeq ($(UNAME), Darwin)
	# No postgres user on most OS X installs. So no sudo.
	-@dropdb crypton_test
endif

reset: clean-test-db setup-test-environment

build-client:
	@$(MAKE) -C ../client compile

run-server:
	@echo "Starting Crypton backend..."
	@NODE_ENV=test `which node` ../server/bin/cli.js run & # echo "$$!" > server.pid && cat server.pid

kill-server:
	@echo "Killing Crypton backend..."
	-@kill -9 $$(ps aux | grep "server/bin/cli.js" | grep -v "grep" | awk '{print $$2}') > /dev/null 2>&1 || echo "Could not find a Crypton server to kill, ignoring..."

loop:
	@echo "Looping integration tests until failure..."
	@until ! make; do :; done

.PHONY: test test-integration node_modules setup-test-environment clean-test-db reset build-client run-server kill-server loop
