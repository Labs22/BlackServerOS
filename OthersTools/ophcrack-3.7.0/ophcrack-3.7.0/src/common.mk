# Copyright (C) 2008, Bertrand Mesot <http://www.objectif-securite.ch>
#	 	2008, Cedric Tissieres <http://www.objectif-securite.ch>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-13, USA

# General variables
SHELL = /bin/sh

CC = i686-w64-mingw32.static-gcc
RANLIB = i686-w64-mingw32.static-ranlib

CFLAGS  = -g -O2 -DNDEBUG -Wall -std=gnu9x -pedantic -I.. -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -I.. -DWIN32 -I/usr/lib/mxe/usr/include
LDFLAGS =  -lm -lexpat -lpthread  -L/usr/lib/mxe/usr/lib -lssl -lcrypto -L/usr/lib/mxe/usr/i686-w64-mingw32.static/qt/lib  -lwsock32 -lregex -lntdll -lmman

HEADERS = $(wildcard *.h)
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:%.c=%.o)

all:

%.a:
	ar r $@ $^
	$(RANLIB) $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean_:
	rm -f *.o *~
	rm -f .depend

clean: clean_

distclean_: clean
	rm -f Makefile

distclean: distclean_

.depend: $(HEADERS) $(SOURCES)
	$(CC) $(CFLAGS) -MM $^ >.depend

-include .depend
