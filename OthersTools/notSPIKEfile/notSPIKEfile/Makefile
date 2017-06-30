CC=gcc
CFLAGS=
SRC=notSPIKEfile.c fuzz_utils.c signal_utils.c ptrace_utils.c
LIBDIR=libdisasm/src/arch/i386/libdisasm
INCLUDEDIR=include
OBJ=$(SRC:.c=.o)
HEADERS=ifuzz.h getopt.h
EXE=notSPIKEfile
LIBS=libdisasm/src/arch/i386/libdisasm/libdisasm.so

%.o: %.c $(INCLUDEDIR)/%.h
	$(CC) -c $(CFLAGS) $*.c -I $(INCLUDEDIR)
all: $(OBJ)
	$(CC) $(OBJ) -o $(EXE) $(LIBS)
clean:
	rm -rf $(OBJ) $(EXE) *~ core *.tar dist libdisasm/src/arch/i386/libdisasm/*.so
tar:
	mkdir dist
	cp -pr README LICENSE INSTALL libdisasm include *.c Makefile make.sh dist
	tar -cvf notSPIKEfile`date +%Y%M%d`.tar dist

