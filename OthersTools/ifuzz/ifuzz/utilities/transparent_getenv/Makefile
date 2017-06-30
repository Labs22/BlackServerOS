CC=gcc
LD=ld
GNULD=ld

all: transparent_getenv

transparent_getenv: transparent_getenv.c
	$(CC) -c -fPIC transparent_getenv.c 
	$(GNULD) -G -z text -o transparent_getenv.so transparent_getenv.o -ldl

irixmips2:
	$(CC) -c -fPIC  -mips2 transparent_getenv.c -mabi=32
	$(GNULD) -m elf32bsmip  -G -z text -o transparent_getenv.so transparent_getenv.o

irix: shared
solaris: shared
hpux: shared

clean:
	rm -f *.o *.so core
