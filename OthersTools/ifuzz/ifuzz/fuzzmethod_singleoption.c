/* $Id */

#include "ifuzz.h"

#define FUZZES_PER_OPTION 5

extern pid_t child;

void
fuzzmethod_singleoption (char *fullpath,struct singleoption_args *singleoption_args)
{
  pid_t pid;
  int status;
  char option;
  char option_string[3];
  char *args[4];
  FILE *fp;


  for (option = 0x20; option < 0x7f; option++)
    {
      int thorough = 0;
      sprintf (option_string, "-%c", option);
      while (thorough++ < FUZZES_PER_OPTION)
	{
	  rfree ();

	  args[0] = fullpath;
	  args[1] = option_string;
	  args[2] = get_random_string ();
	  args[3] = NULL;

	  if ((pid = fork ()) != 0)
	    {
	      child = pid;
	      signal (SIGALRM, &handle_alarm);
	      alarm (TIME_TO_DIE);
	      waitpid (pid, &status, 0);
	      alarm (0);

	      if (WIFSIGNALED (status))
		{
		  switch (WTERMSIG (status))
		    {
		      /* 
		       ** since we are only logging the signals, we might as well catch anything
		       ** even remotely interesting
		       */
		    case SIGBUS:
		    case SIGILL:
		    case SIGSEGV:
		    case SIGTRAP:
		    case SIGFPE:
		    case SIGUSR1:
		    case SIGUSR2:
		      fprintf (stderr, "%s | CRASH SIGNAL #%d (%s)\n",
			       fullpath, WTERMSIG (status), option_string);
		      if (!
			  (fp =
			   open_c_file (fullpath, pid, WTERMSIG (status))))
			{
			  fprintf (stderr,
				   "have you ever heard of chmod?  no access to dump dir you douchebag.\n");
			  exit (-1);
			}

		      print_c_basic_header (fp);
		      print_c_comment_open (fp);
		      print_text (fp, asciitime ());
		      print_text (fp, "Single option: ");
		      print_text (fp, option_string);
		      print_c_comment_close (fp);
		      print_c_array_to_file (fp, args, "args");
		      print_c_array_to_file (fp, environ, "envp");
		      print_c_execve_call (fp, fullpath, "args", "envp");
		      print_c_basic_header_close (fp);
		      fclose (fp);
		      break;
		    default:
		      break;
		    }
		}

	    }
	  else
	    {
	      /* do the actual fuzz */
	      execle (fullpath, fullpath, option_string, get_random_string (),
		      NULL, environ);
	      perror ("execle");
	    }
	}
    }
  rfree ();
  return;			/* unreached */
}

/* should only be called once even when fuzzing multiple programs */
void
parse_singleoption (int argc, char *argv[], struct singleoption_args *singleoption_args)
{
  int c;
  opterr = 0;
  printf ("initializing single option options\n");
  while ((c = getopt (argc, argv, "s")) != -1)
    {
      switch (c)
        {
        case 's':
          singleoption_args->silent = 1;
          break;
        default:
          printf("Error in arguments\n");
          usage();
          break;
        }

    }
  return;

}

