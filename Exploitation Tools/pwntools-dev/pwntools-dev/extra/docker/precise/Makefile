all: Dockerfile

Dockerfile: ../xenial/Dockerfile FORCE
	sed "s|ubuntu:xenial|ubuntu:precise|" < "$<" > "$@"

FORCE:

.PHONY: all
