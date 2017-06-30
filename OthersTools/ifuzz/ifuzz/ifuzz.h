/* $Id */

#ifndef _IFUZZ_H
#define _IFUZZ_H

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>


/* weird solaris10/x86 hack (broken c libraries?) */
/* don't use -DHACK unless you really know what youre doing */
#ifdef HACK
#include <sys/iso/signal_iso.h>
#define WTERMSIG(stat)          ((int)((stat)&0x7F))
#define WIFSIGNALED(stat)       ((int)((stat)&0xFF) > 0 && \
                                    (int)((stat)&0xFF00) == 0)
#endif


#define TIME_TO_DIE 3		/* seconds to let a process run before killing it and continuing */

#define CODE_DUMP_PATH "dumps/"

#define FUZZTYPE_ARGV0 0
#define FUZZTYPE_ARGV1 1
#define FUZZTYPE_SINGLE 2
#define FUZZTYPE_GETOPT 3

#define MAX_FUZZ_LENGTH 15000

#define MAX_HEAP_STRINGS 2048

#define APPEND_BYTE_TO_STRING(array,byte) *(array+p_strlen(array)) = byte;
#define ZERO(array,size) memset(array,0x0,size);

#define VALID_FILE "/tmp/IFUZZ_EMPTY_FILE"

extern char **environ;

extern char *optarg;
extern int opterr;

struct getopt_args
{
  char *optstring;		/* option string */
  int extra;			/* how many extra args that arent associated with options to add at most */
  char *first;			/* if set, always use this as argv[1] */
  char *last;			/* if set, always use this as the last argv value */
  int silent;			/* if set, dup2 stderr and stdout to /dev/null */
};

struct argv_args
{
  int initialized;
  int silent;			/* if set, dup2 stderr and stdout to /dev/null */
};

struct singleoption_args
{
int initialized;
int silent;
};

void handle_alarm (int signum);
void do_fuzz (char *fullpath, char *filename, int fuzztype, int argc,
	      char **argv);
int begin_fuzz (const char *path, int fuzztype, int argc, char **argv);
void usage ();

void fuzzmethod_argvzero (char *fullpath, struct argv_args *argv_args);
void fuzzmethod_singleoption (char *fullpath,struct singleoption_args *singleoption_args);
void fuzzmethod_argvone (char *fullpath, struct argv_args *argv_args);
void fuzzmethod_getopt3 (char *fullpath, struct getopt_args *getopt_args);
void parse_getopt3 (int argc, char *argv[], struct getopt_args *getopt_args);
void fuzzmethod_getopt3_usage ();
int count_flags (char *optstring);
int count_options (char *optstring);
void parse_argv (int argc, char *argv[], struct argv_args *argv_args);
void parse_singleoption (int argc, char *argv[], struct singleoption_args *singleoption_args);


int factorial (int num);

char *p_strfry (char *string);
size_t p_strlen (const char *s);
char *make_string (int size, char *startswith, char *endswith, char *hasone);
char *get_random_string ();

void print_c_basic_header (FILE * fp);
void print_c_execve_call (FILE * fp, char *path, char *args, char *envp);
void print_c_basic_header_close (FILE * fp);
void print_c_array_to_file (FILE * fp, char *array[], char *arrayname);
FILE *open_c_file (char *binary, int pid, int signal);
void print_c_comment (FILE * fp, char *comment);
void print_c_comment_close (FILE * fp);
void print_c_comment_open (FILE * fp);
void print_text (FILE * fp, char *text);

char *asciitime ();



void *rmalloc (size_t size);
void rfree ();

int get_random_int (int min, int max);

void parse_getopt (char *optstring, char **options, char **flags);

char *get_short_option (char opt);
void check_dumpdir_existance ();

void verify_file ();
void remove_file ();
#endif
