all: test

test: check test-unit test-integration

test-unit: test-unit-server test-unit-client

test-unit-server:
	@$(MAKE) -C server -s test

test-unit-client:
	@$(MAKE) -C client -s test

test-integration:
	@$(MAKE) -C test -s test

full: nuke test

clean:
	@$(MAKE) -C client -s clean
	@$(MAKE) -C test -s clean-test-db

setup:
	@$(MAKE) -C test -s setup-test-environment

reset: clean setup

check:
	@echo "Checking dependencies..."
	@./check_dependencies.sh

nuke:
	@echo "Deleting all node modules..."
	-@rm -rf server/node_modules
	-@rm -rf client/node_modules
	-@rm -rf test/node_modules

loop: nuke
	@echo "Looping all tests until failure..."
	@until ! make; do :; done

.PHONY: test test-unit test-unit-server test-unit-client test-integration clean setup reset check nuke
