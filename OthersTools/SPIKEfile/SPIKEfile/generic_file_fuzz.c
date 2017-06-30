/*
    SpikeFile
    Copyright (C) 2005 Adam Greene <agreene@idefense.com>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the Free
    Software Foundation; either version 2 of the License, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
    more details.

    You should have received a copy of the GNU General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place, Suite 330, Boston, MA 02111-1307 USA
*/


#include <generic_file_fuzz.h>

int active_processes=0;	/* current # of children doing our bidding */
int crashes=0; /* # of "crashes" recorded for the session so far */
int killsignum=SIGTERM;	/* signal to send after timeout to kill a process */
u_int8_t quiet=1; /* quiet means just have a pretty status bar */

extern char **s_fuzzstring;

void sigchld_handler();
void sigint_handler();

void sigint_handler()
{
	signal(SIGCHLD,SIG_DFL);
	printf("\n [!] ABORTED                                            ");
	printf("\n [ ] Cleaning up...");fflush(NULL);
	while ( active_processes )
	{
		wait(NULL);
		active_processes--;
	}
	printf("\r [*] Cleaning up\n");
	printf("Done (received user abort)\n");
	printf("%d crashes occured, the files and their respective crash info has been recorded\n",crashes);
	exit(ERR_OK);
	return;
}

char stupid_progress(int ix);
char stupid_progress(int ix)
{
	char buff[] = "|/-\\";
	return(buff[ix%4]);
}

void sigchld_handler()
{
	int status;
	wait(&status);
	active_processes--;

	if ( (WIFEXITED(status)) && (WEXITSTATUS(status)==1) )
	{
		signal(SIGCHLD,SIG_DFL);
		while ( active_processes )
		{
			wait(NULL);
			active_processes--;
		}
		printf("\nDone (reached end of base file)\n");
		printf("%d crashes occured, the files and their respective crash info has been recorded\n",crashes);
		exit(ERR_OK);
	}
	else if ( WIFEXITED(status) && (WEXITSTATUS(status)==3) )
	{
		signal(SIGCHLD,SIG_DFL);

		while ( active_processes )
		{
			wait(NULL);
			active_processes--;
		}
		printf("Done (Specified executable does not exist or is not +x)\n");
		exit(ERR_OK);
	}

	if ( WIFEXITED(status) )
	{
		if ( WEXITSTATUS(status) == 255 )
		{
			crashes++;
		}
	}

	return;
}

int
main (int argc, char *argv[])
{
	char *pScript;
	char *pCommand_orig;
	char *pCommand;
	int skipfuzzstr = 0;
	int skipvariable = 0;
	int fuzzvarnum;
	int fuzzstrnum;
	int firstfuzz;
	char *pFilename;
	time_t timeout = 5;
	char **argp;
	int max_processes=7;
	char *pOutfileProcess=NULL;
	int c;
	pid_t master;

	master = getpid();
	struct sigaction act;
	ZERO(&act,sizeof(act));
	act.sa_handler = &sigchld_handler;
	act.sa_flags = SA_NODEFER;

	sigaction(SIGCHLD,&act,NULL);

	opterr = 0;

	pScript = pCommand_orig = pCommand = pFilename = NULL;

	if ( argc < 5 )
	{
		usage (argv);
	}



	while ( (c = getopt (argc, argv, "f:t:vkh")) != -1 )
	{
		switch ( c )
		{
			case 'f':
				pFilename = optarg;
				break;
			case 't':
				timeout = atoi (optarg);
				break;
			case 'v':
				quiet = 0;
				break;
			case 'k':
				timeout=0;
				break;
			case 'h':
				usage (argv);
				break;
		}

	}
	if ( ((argc - optind) != 4) )
	{
		usage (argv);
	}

	pScript = argv[optind++];
	skipvariable = atoi (argv[optind++]);
	skipfuzzstr = atoi (argv[optind++]);
	pCommand_orig = argv[optind++];



	current_spike = our_spike = new_spike ();

	if ( !our_spike )
	{
		fprintf (stderr, "Malloc failed trying to allocate a spike.\r\n");
		exit (ERR_BAD);
	}

	setspike (our_spike);

	s_init_fuzzing ();
	s_resetfuzzvariable ();

	fuzzvarnum = fuzzstrnum = 0;
	firstfuzz = 1;
	signal(SIGINT,&sigint_handler);
	while ( !s_didlastvariable () )
	{
		s_resetfuzzstring ();


		if ( firstfuzz )
		{
			firstfuzz = 0;
			if ( fuzzvarnum < skipvariable )
			{
				for ( fuzzvarnum = 0; fuzzvarnum < skipvariable; fuzzvarnum++ )
					s_incrementfuzzvariable ();
			}

			if ( fuzzstrnum < skipfuzzstr )
			{
				for ( fuzzstrnum = 0; fuzzstrnum < skipfuzzstr; fuzzstrnum++ )
					s_incrementfuzzstring ();
			}

		}
		else
		{
/* new variable, so reset fuzzstrnum */
			fuzzstrnum = 0;
		}



		while ( !s_didlastfuzzstring () )
		{
			int tmp;
			pid_t pid;
			spike_clear ();
			if ( getpid() == master )
			{
				if ( active_processes<max_processes )
				{
					active_processes++;

					switch ( pid = fork() )
					{
						case -1:
							perror("you suck so bad at forking.");exit(ERR_BAD);break;
						case 0:	/* child */
							signal(SIGCHLD,SIG_DFL);
							signal(SIGINT,SIG_IGN);
							break;
						default: /* parent */ 


							if ( s_fuzzstring[current_spike->fuzzstring + 1] == NULL )
							{
								current_spike->fuzzstring=0;
								current_spike->didlastfuzzstring=1;
							}


							break;
					}
				}
				else
				{

					pause();
				}
			}

			if ( getpid() == master )
			{

				printf("\r [%c] [VAR: %.5d STR: %.5d CRASHES: %.5d]",stupid_progress(current_spike->fuzzstring),current_spike->fuzzvariable,current_spike->fuzzstring,crashes);
				fflush(NULL);
/*
 this must be done because otherwise didlaststring wont get set in the parent, which is where we need it
*/
				spike_fileopen ("/dev/null");
				s_parse (pScript);
				spike_filewrite (current_spike->datasize, current_spike->databuf);
				spike_close_file ();

				s_incrementfuzzstring ();
				continue;
			}

			pCommand = strdup(pCommand_orig);

			if ( !quiet )
				printf ("Fuzzing Variable %d:%d\n", fuzzvarnum, fuzzstrnum);
			if ( !pFilename )
			{
				pFilename = "Default.Filename";
			}

			pOutfileProcess=malloc(strlen(pFilename)+10);
			snprintf(pOutfileProcess,strlen(pFilename)+9,"%.5d-%s",getpid(),pFilename);
			spike_fileopen (pOutfileProcess);
			s_parse (pScript);
			spike_filewrite (current_spike->datasize, current_spike->databuf);
			spike_close_file ();
			pCommand = F_dstring_replace (pCommand,"%FILENAME%", pOutfileProcess);
			argp = F_build_argv(NULL,pCommand);
			tmp = F_execmon(argp,NULL,timeout,pFilename,current_spike->fuzzstring,current_spike->fuzzvariable);
			if ( tmp != -1 )
			{

				unlink(pOutfileProcess);
			}
			free(pOutfileProcess);
			free(pCommand);

			_exit(tmp);
		}
		fuzzvarnum++;
		s_incrementfuzzvariable ();
	}

	return(ERR_OK);
}


void
usage (char **argv)
{
	printf ("fileSPIKE\n");
	printf ("\t%s [options] <spike_file.spk> <startvar> <startstr> <command>",
			argv[0]);
	printf ("\n\n");
	printf ("Options:\n\n");
	printf (" -t\tTimeout value (default=5)\n");
	printf (" -k\tDo not kill the process after timeout\n");
	printf (" -h\tPrint this message\n");
	printf (" -f\tName for created files (sometimes important, don't overlook this)\n");
	printf ("\n\n");
	printf ("Command:\n\n");
	printf
	("\tQuoted command to execute to process the generated file. Use %%FILENAME%% as a symbol to be replaced with the filename.\n\n");
	printf ("\n");
	printf ("Example:\n\n");
	printf
	(" %s -t 3 -f fuzz.gif gif89a.spk 0 0 \"/usr/X11R6/bin/display -debug %%FILENAME%%\"\n\n",
	 argv[0]);

	exit (ERR_OK);
}
