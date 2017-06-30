LIBDISASM=libdisasm/src/arch/i386/libdisasm/
SRC=dlargs.c dlrpc.c filestuff.c listener.c generic_file_fuzz.c spike.c crc32.c getopt.c
OBJ=$(SRC:.c=.o)
LIBS=-ldl -ldisasm
CFLAGS=-L $(LIBDISASM) -L.
EXE=SPIKEfile
INCLUDES=-I include/ -I $(LIBDISASM)
CC=gcc
LD=ld
SO=libdlrpc.so

%.o: %.c
	$(CC) -c $(CFLAGS) $*.c $(INCLUDES)

all: $(OBJ) util.o
	$(LD) -shared -soname libdlrpc.so -o libdlrpc.so $(OBJ)
	$(CC) $(OBJ) -o $(EXE) $(LIBS) $(CFLAGS) $(SO) util.o
clean:
	rm -f *.o $(EXE) $(SO) *~ core *.tar $(LIBDISASM)/*.so
tar:
	mkdir dist
	cp -pr LICENSE README INSTALL libdisasm include *.c Makefile make.sh dist
	tar -cvf SPIKEfile`date +%Y%M%d`.tar dist

