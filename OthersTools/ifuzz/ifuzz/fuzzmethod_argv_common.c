#include "ifuzz.h"

/* should only be called once even when fuzzing multiple programs */
void
parse_argv (int argc, char *argv[], struct argv_args *argv_args)
{
  int c;
  opterr = 0;
  printf ("initializing argv options\n");
  while (argv && (c = getopt (argc, argv, "o:s")) != -1)
    {
      switch (c)
	{
	case 's':
	  argv_args->silent = 1;
	  break;
	default:
	  printf("Error in arguments\n");
	  usage();
	  break;
	}

    }
  return;

}

