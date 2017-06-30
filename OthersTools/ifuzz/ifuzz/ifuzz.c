#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>

#include "ifuzz.h"

pid_t child;

int
main (int argc, char *argv[])
{
  if (argc < 3)
    {
      usage ();
      exit (0);
    }

  check_dumpdir_existance ();

  /* could also call do_fuzz, begin_fuzz is just a wrapper to
   ** pass files to do_fuzz so you don't have to specify them 
   ** manually 
   */

  if (argc == 3)
    begin_fuzz (argv[2], atoi (argv[1]), 0, 0);
  else
    begin_fuzz (argv[2], atoi (argv[1]), argc, argv);

  remove_file ();
  return 0;
}


int
begin_fuzz (const char *path, int fuzztype, int argc, char **argv)
{
  DIR *dirp;
  struct dirent *dp;
  char *buff = NULL;
  struct stat statbuf;
  int ix = 0;

  if ((dirp = opendir (path)) == NULL)
    {
      fprintf (stderr, "Error opening specified directory [%s]\n", path);
      perror ("opendir");
      exit (-1);
    }


  while ((dp = readdir (dirp)))
    {
      buff = realloc (buff, p_strlen (path) + 1 + strlen (dp->d_name) + 1);
      sprintf (buff, "%s/%s", path, dp->d_name);
      if (stat (buff, &statbuf))
	{
	  perror ("stat");
	  exit (-1);
	}
      if (!(strcmp (dp->d_name, "ifuzz")) ||
	  !(strcmp (dp->d_name, ".")) ||
	  !(strcmp (dp->d_name, "..")) ||
	  !(statbuf.st_mode & S_IEXEC) || !(S_ISREG (statbuf.st_mode)))
	continue;
      printf ("Executable: %s\n", dp->d_name);
      ix++;
      do_fuzz (buff, dp->d_name, fuzztype, argc, argv);
    }
  free (buff);
  closedir (dirp);
  printf ("Fuzzed %d files in directory %s\n", ix, path);

  return 0;
}

/* 
 fullpath is full binary path for exec
 filename is argv[0] 
*/
void
do_fuzz (char *fullpath, char *filename, int fuzztype, int argc, char **argv)
{
  static struct getopt_args getopt_args;
  static struct argv_args argv_args;
  static struct singleoption_args singleoption_args;
  switch (fuzztype)
    {
    case FUZZTYPE_ARGV0:
      printf ("Doing argv[0] fuzz\n");
      if (!argv_args.initialized)	/* uninitialized */
	parse_argv (argc, argv, &argv_args);
      fuzzmethod_argvzero (fullpath, &argv_args);
      break;
    case FUZZTYPE_ARGV1:
      printf ("Doing argv[1] fuzz\n");
      if (!argv_args.initialized)	/* uninitialized */
	parse_argv (argc, argv, &argv_args);

      fuzzmethod_argvone (fullpath, &argv_args);
      break;
    case FUZZTYPE_SINGLE:
      printf ("Doing unintelligent singleoption fuzz\n");
      parse_singleoption(argc,argv,&singleoption_args);
      fuzzmethod_singleoption (fullpath,&singleoption_args);
      break;
    case FUZZTYPE_GETOPT:
      if (!getopt_args.optstring)	/* uninitialized */
	parse_getopt3 (argc, argv, &getopt_args);
      if (!getopt_args.optstring)
	{
	  fuzzmethod_getopt3_usage ();
	}
      printf ("Doing getopt optstring fuzz [%s]\n", getopt_args.optstring);
      fuzzmethod_getopt3 (fullpath, &getopt_args);
      break;
    default:
      printf ("Fuzz type not implemented yet\n");
      break;
    }


  return;
}


/* 
** to handle those programs that either expect 
** user interaction or ones that just lock up.
*/

void
handle_alarm (int signum)
{
  printf ("Killing %d\n", child);
  kill (child, SIGTERM);
  return;
}

void
usage ()
{
  printf
    ("Usage: ifuzz <fuzztype> <binary directory> [fuzz specific options]\n");
  printf ("Fuzztypes: \t0 - argv[0] fuzzing\n\t\t");
  printf ("1 - argv[1] fuzzing\n\t\t");
  printf ("2 - incremental single option fuzzing\n\t\t");
  printf ("3 - incremental multiple option fuzzing\n");
 printf   ("\tifuzz 3 directory/ <-o optstring> [-e extra-args] [-f first_arg] [-l last_arg] [-s]\n");
  printf("\tifuzz 1 directory/ [-s]\n");
  printf("\tifuzz 0 directory/ [-s]\n");

}

char *
asciitime ()
{
  time_t t;
  time (&t);
  return ctime (&t);
}


void
check_dumpdir_existance ()
{
  DIR *dir;

  if (!(dir = opendir (CODE_DUMP_PATH)))
    {
      perror ("Fatal error opening code dump directory");
      fprintf (stderr, "create/check permissions on %s\n", CODE_DUMP_PATH);
      exit (-1);
    }
  closedir (dir);
  return;
}
