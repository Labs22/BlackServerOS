all:
	gcc -Wall -c -fPIC getopt_shared.c
	ld -G -z text getopt_shared.o -o getopt_shared.so
clean:
	rm -f *.so *.o
