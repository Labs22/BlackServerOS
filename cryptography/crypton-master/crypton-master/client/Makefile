all: test

compile:
	@./compile.sh --once

test: test-unit

test-unit: node_modules clean compile
	@echo "Running client tests..."
	@./node_modules/karma/bin/karma start --browsers PhantomJS --single-run

node_modules:
	@echo "Installing node modules for client testing..."
	@npm install

clean:
	@echo "Removing client compiled code..."
	-@rm -rf dist

.PHONY: compile test test-unit node_modules clean
