all: arpoison

arpoison: arpoison.c

	gcc -lnet -o arpoison `libnet-config --defines` -I/opt/local/include -L/opt/local/lib arpoison.c

clean:

	@rm -f arpoison
