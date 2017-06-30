/* $Id */

#include "ifuzz.h"

extern pid_t child;


/*
** fullpath: path binaries reside in
*/

void
fuzzmethod_argvone (char *fullpath, struct argv_args *argv_args)
{
  pid_t pid;
  int status;
  FILE *fp;

  char *args[3];
  args[0] = fullpath;
  args[1] = get_random_string ();
  args[2] = NULL;

  if ((pid = fork ()) != 0)
    {
      child = pid;
      printf ("set child to %d\n", child);
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
	      fprintf (stderr, "%s | CRASH SIGNAL #%d (argv[1])\n", fullpath,
		       WTERMSIG (status));
	      if (!(fp = open_c_file (fullpath, pid, WTERMSIG (status))))
		{
		  fprintf (stderr,
			   "have you ever heard of chmod?  no access to dump dir you douchebag.\n");
		  exit (-1);
		}

	      print_c_basic_header (fp);
	      print_c_comment_open (fp);
	      print_text (fp, asciitime ());

	      print_text (fp, "Standard argv[1] crash");
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

      if (argv_args->silent)	/* silent mode */
	{
	  int fd;
	  fd = open ("/dev/null", O_WRONLY);
	  dup2 (fd, STDOUT_FILENO);
	  dup2 (fd, STDERR_FILENO);
	}

      /* XXX should also run with no args to try to get an error message XXX */
      execve (fullpath, args, environ);
      perror ("execle");

    }
  rfree ();
  return;
}
