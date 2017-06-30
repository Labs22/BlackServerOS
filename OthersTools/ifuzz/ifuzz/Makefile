CC=gcc

CFLAGS=-Wall

SRC=fuzz_strings.c fuzzmethod_argvone.c fuzzmethod_argvzero.c ifuzz.c \
fuzzmethod_singleoption.c fuzz_generate_code.c \
fuzzmethod_getopt3.c fuzz_misc.c getopt.c fuzz_memory.c \
fuzz_fileops.c fuzzmethod_argv_common.c

DIRS=utilities

OBJ=$(SRC:.c=.o)

HEADERS=ifuzz.h getopt.h

EXE=ifuzz

%.o: %.c
	$(CC) -c $(CFLAGS) $*.c
all: $(OBJ)
	$(CC) $(OBJ) -o $(EXE)
clean:
	rm -f $(OBJ) $(EXE) *~ core *.tar
indent:
	indent $(SRC) $(HEADERS)
tar:
	mkdir -p dist/dumps
	cp -r $(DIRS) $(SRC) $(HEADERS) Makefile doc dist/
	tar -cvf iFUZZ-`date +%G-%m-%d.tar` dist
	rm -rf dist
	ls -alF iFUZZ-`date +%G-%m-%d.tar`

bak:
	mkdir iFUZZ-`date +%G-%m-%d.bak`
	cp -r $(SRC) $(HEADERS) Makefile doc iFUZZ-`date +%G-%m-%d.bak`
