# Makefile

SHELL=/bin/bash
CC=gcc
SRCS=dns-spoof.c
OBJS=dns-spoof.o
CFLAGS=-std=c99 -pedantic -Wall -ggdb -D_GNU_SOURCE
LFLAGS=-lpcap
PROGRAM=dns-spoof

.PHONY: depend clean

$(PROGRAM): $(SRCS) $(OBJS)
	$(CC) $(CFLAGS) $(LFLAGS) $(OBJS) -o $(PROGRAM)

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -f $(OBJS) $(PROGRAM) *.out

# DO NOT DELETE

dns-spoof.o: dns-spoof.c
