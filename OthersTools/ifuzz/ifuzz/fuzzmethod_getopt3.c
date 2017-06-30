/* $Id */

#include "ifuzz.h"

extern pid_t child;

/* 
** fullpath: path binaries reside in
*/

/* commandline:
**
** -s		Silent mode, /dev/null for stderr and stdout
** -e x 	Use x extra unbound strings
** -f x		Use string x as argv[1] ALWAYS
** -l x		Use string x as the last argv[] ALWAYS
** -o x		Use string x as optstring
*/

void
fuzzmethod_getopt3 (char *fullpath, struct getopt_args *getopt_args)
{
  pid_t pid;
  int status;

  char *flags = NULL;
  char *options = NULL;

  int option_count;
  int flag_count;

  int ix, iy;
  static char *args[255];	/* if you pass more than 255 options to a program you are not cool */

  parse_getopt (getopt_args->optstring, &options, &flags);
  option_count = p_strlen (options);
  flag_count = p_strlen (flags);


  /* feel free to bound this loop differently */
  for (ix = 0; ix < (factorial (option_count + 1) * (2 * flag_count + 1)); ix++)	/* nice arbitrary length boundary for now */
    {
      for (iy = 0; iy <= option_count; iy++)
	{
	  int tmp;
	  int args_counter = 0;
	  int randint;
	  int extracnt = 0;
	  rfree ();

	  p_strfry (options);	/* next order */
	  p_strfry (flags);
	  if (getopt_args->first)
	    args[1] = getopt_args->first;
	  for (tmp = 0; tmp < iy; tmp++)	/* setting up args[] for execve */
	    {
	      if (getopt_args->first)
		{
		  args[tmp * 2 + 2] = get_short_option (options[tmp]);
		  args[tmp * 2 + 3] = get_random_string ();
		}
	      else
		{
		  args[tmp * 2 + 1] = get_short_option (options[tmp]);
		  args[tmp * 2 + 2] = get_random_string ();

		}

	      args_counter += 2;
	    }
	  args_counter++;

	  randint = get_random_int (0, flag_count);
	  for (tmp = args_counter; tmp < args_counter + randint; tmp++)
	    args[tmp] = get_short_option (flags[tmp - args_counter]);

	  while (extracnt++ < getopt_args->extra)
	    args[tmp++] = get_random_string ();

	  if (getopt_args->last)
	    args[tmp++] = getopt_args->last;
	  args[tmp] = NULL;	/* terminating args[] */
	  args[0] = fullpath;

	  if ((pid = fork ()) != 0)	/* parent, wait for crash or just chill */
	    {
	      FILE *fp;
	      child = pid;
	      signal (SIGALRM, &handle_alarm);
	      alarm (TIME_TO_DIE);
	      waitpid (pid, &status, 0);
	      alarm (0);

	      if (WIFSIGNALED (status))	/* program exited due to signal */
		{
		  int args_ix = 0;
		  switch (WTERMSIG (status))	/* find out which signal */
		    {
		      /* 
		       ** since we are only logging the signals, we might as well catch anything
		       ** even remotely interesting
		       */
		    case SIGBUS:	/* bus error */
		    case SIGILL:	/* illegal instruction */
		    case SIGSEGV:	/* segmentation violation */
		    case SIGTRAP:	/* breakpoint/trap */
		    case SIGFPE:	/* floating point exception */
		    case SIGUSR1:	/* user defined */
		    case SIGUSR2:	/* user defined */
		      fprintf (stderr, "CRASH\n");
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
		      print_text (fp, "CrashCausedBy: ");
		      while (args[args_ix])
			{
			  if (strlen (args[args_ix]) > 50)
			    {
			      print_text (fp, "[FUZZ] ");
			    }
			  else
			    {
			      print_text (fp, args[args_ix]);
			      print_text (fp, " ");
			    }
			  args_ix++;
			}

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
	      if (getopt_args->silent)	/* silent mode */
		{
		  int fd;
		  fd = open ("/dev/null", O_WRONLY);
		  dup2 (fd, STDOUT_FILENO);
		  dup2 (fd, STDERR_FILENO);
		}
	      execve (fullpath, args, environ);
	      perror ("execle");
	    }
	}
    }
  free (options);
  free (flags);
  return;
}


/* should only be called once even when fuzzing multiple programs */
void
parse_getopt3 (int argc, char *argv[], struct getopt_args *getopt_args)
{
  int c;
  opterr = 0;

  while ((c = getopt (argc, argv, "o:e:f:l:s")) != -1)
    {
      switch (c)
	{
	case 's':
	  getopt_args->silent = 1;
	  break;
	case 'o':
	  if (!getopt_args->optstring)
	    {
	      getopt_args->optstring = strdup (optarg);
	    }
	  else
	    {
	      printf ("ignoring multiple -o arguments\n");
	    }
	  break;

	case 'e':
	  if (!getopt_args->extra)
	    {
	      getopt_args->extra = atoi (optarg);
	    }
	  else
	    {
	      printf ("ignoring multiple -e arguments\n");
	    }
	  break;

	case 'f':
	  if (!getopt_args->first)
	    {
	      getopt_args->first = strdup (optarg);
	    }
	  else
	    {
	      printf ("ignoring multiple -f arguments\n");
	    }
	  break;

	case 'l':
	  if (!getopt_args->last)
	    {
	      getopt_args->last = strdup (optarg);
	    }
	  else
	    {
	      printf ("ignoring multiple -l arguments\n");
	    }


	  break;
	default:
	  fuzzmethod_getopt3_usage ();
	  break;
	}

    }
  return;

}

void
fuzzmethod_getopt3_usage ()
{
  printf ("::fuzzmethod_getopt3_usage::\n");
  printf
    ("\tifuzz 3 directory/ <-o optstring> [-e extra-args] [-f first_arg] [-l last_arg] [-s]\n");
  exit (0);
}
