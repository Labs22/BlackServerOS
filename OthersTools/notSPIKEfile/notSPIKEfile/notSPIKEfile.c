/*
    notSPIKEfile
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


#include <notSPIKEfile.h>

int active_processes=0;	/* current # of children doing our bidding */
int crashes=0; /* # of "crashes" recorded for the session so far */
int killsignum=SIGTERM;	/* signal to send after timeout to kill a process */
u_int8_t quiet=1; /* quiet means just have a pretty status bar */

int
main (int argc, char *argv[])
{
	char *pCommand_orig;
	char *pCommand;
	char *pExtension;
	char *pBasefile;
	char *pOutfile;
	int max_processes=1;
	time_t timeout = 2;
	char **argp = NULL;
	size_t startbyte=0, endbyte=0;
	time_t delay=1;	/* delay between kill and execution */
	int c;
	int ix;
	int iy;
	pid_t master;
	int tmp;
	int limit;
	int startfuzz=0,stopfuzz=0;
	int keep=0;
	struct sigaction act;
	fuzztype fuzztype=F_STRING;

	ZERO(&act,sizeof(act));

	act.sa_handler = &sigchld_handler;
	master = getpid();

	pCommand_orig = pCommand = pExtension = pBasefile = pOutfile = NULL;

	startbyte = endbyte = opterr =  0;

	while ( (c = getopt (argc, argv, "t:vkho:r:BSd:m:s:f:K")) != -1 )
	{
		switch ( c )
		{
			case 'K': keep=1;
				break;
			case 'r':
				if ( *(optarg+strlen(optarg)-1) == '-' )
				{
					if ( (sscanf (optarg, "%d-", &startbyte) != 1) )
					{
						fprintf (stderr, "Bad byte range specified\n");
						usage ();
					}
					endbyte=MAXINT;

				}


				else
				{

					if ( (sscanf (optarg, "%d-%d", &startbyte, &endbyte) != 2) )
					{
						fprintf (stderr, "Bad byte range specified\n");
						usage ();
					}
				}
				break;
			case 'f':
				if ( (sscanf (optarg, "%d-%d", &startfuzz, &stopfuzz) != 2) )
				{
					fprintf (stderr, "Bad fuzz value range specified\n");
					usage ();
				}

				break;
			case 'm': max_processes = atoi(optarg);
				break;
			case 'd':
				delay = atoi(optarg);
				break;
			case 't':
				timeout = atoi (optarg);
				break;
			case 'v':
				quiet=0;
				break;
			case 'k':
				timeout = 0;
				break;
			case 'h':
				usage ();
				break;
			case 's':
				if ( (killsignum = get_signum(optarg)) == ERR_BAD )
				{
					fprintf(stderr,"Bad signal '%s'\n",optarg);
					exit(ERR_BAD);
				}

				break;
			case 'o':
				pOutfile = strdup(optarg);
				break;
			case 'B': fuzztype = F_BLOB; break;
			case 'S': fuzztype = F_STRING; break;
		}
	}


	if ( (argc - optind) != 2 )
	{
		printf("Missing arguments.\n");
		usage ();
	}

	pBasefile = argv[optind++];
	pCommand = strdup (argv[optind]);

	if ( !pOutfile )
	{
		printf("Need outfile\n");exit(ERR_BAD);
	}
	init_fuzz();
	sigaction(SIGCHLD,&act,NULL);
	signal(SIGINT,&sigint_handler);
	if ( fuzztype == F_STRING )
	{
		limit = stringcount;
	}
	else if ( fuzztype == F_BLOB )
	{
		limit = blobcount;
	}

	if ( !startbyte && !endbyte )
	{
		endbyte = get_filesize(pBasefile);
	}
	else if ( assert_range(pBasefile,endbyte) )
	{
		endbyte = get_filesize(pBasefile);
		fprintf(stderr,"Adjusting range to end at byte %d\n",endbyte);
	}

	/* change to allow the range 0-0 */
	if ( stopfuzz > limit || !stopfuzz )
	{
		stopfuzz = limit;
	}
	for ( iy=startfuzz;iy<=stopfuzz;iy++ )
	{
		for ( ix=startbyte;ix<=endbyte;ix++ )
		{
			pid_t pid;
			char *pOutfileProcess;
			if ( getpid() == master )
			{
				if ( active_processes<max_processes )
				{
					active_processes++;
					switch ( pid = fork() )
					{
						case -1:
							fprintf(stderr,"Can't fork\n");
							abort();
							exit(ERR_BAD);
							break;
						case 0:	/* child */
							signal(SIGCHLD,SIG_DFL);
							signal(SIGINT,SIG_IGN);
							break;
						default: /* parent */ 
							break;
					}
				}
				else
				{
					ix--;
					pause();
				}
			}
			if ( getpid() == master )
			{
				printf(" [%c] [FUZZ %-5d BYTE %-5d CRASHES %-5d]\r",stupid_progress(ix),iy,ix,crashes);
				fflush(NULL);
				continue;
			}

			pOutfileProcess=malloc(strlen(pBasefile)+10);
			snprintf(pOutfileProcess,strlen(pOutfile)+9,"%.5d-%s",getpid(),pOutfile);
			pCommand = F_dstring_replace (pCommand, "%FILENAME%", pOutfileProcess);
			if (!(strstr(pCommand,pOutfileProcess)))
			{
				fprintf(stderr,"Appending filename automatically\n");
				F_build_argv(pCommand,pOutfileProcess);

			}
			else
			{
			
			argp = F_build_argv (pCommand,NULL);
			}
		

			if ( access(argp[0],X_OK) )
			{
				return(ERR_EXECFAIL);
			}

			if ( F_mmap_replace (ix, pBasefile, pOutfileProcess,fuzztype,iy) == -1 )
			{
				fprintf(stderr,"F_mmap_replace() failed. This is bad.\n");
				unlink(pOutfileProcess);

				return(ERR_BAD);
			}

			tmp = F_execmon (argp, NULL, timeout,pOutfileProcess,pBasefile,fuzztype,iy,ix);
			if ( !keep )
			{

				unlink(pOutfileProcess);


			}

			_exit(tmp);
		}
	}

	printf(" [*] [FUZZ %-5d BYTE %-5d CRASHES %-5d]\n\r",iy,ix,crashes);


	signal(SIGCHLD,SIG_DFL);
	while ( active_processes )
	{
		printf("\r [%c] Waiting on [%.3d] processes",stupid_progress(max_processes-active_processes),active_processes);
		fflush(NULL);
		wait(NULL);	/* should probably change this to a pause so crashes still get caught */
		active_processes--;
	}
	printf("\r         \r\tDone (reached end of specified byte range)\n");
	printf("\t%d crashes occured%s\n",crashes,(crashes)?"each files respective crash info has been recorded into .txt files":"");
	return(ERR_OK);
}

void
usage ()
{
	printf ("notSPIKEfile\n");
	printf ("\tnotSPIKEfile [options] <base file> <command>\n");
	printf ("\n\n");
	printf ("Required Options:\n\n");
	printf (" -o\tOutput file name base for fuzzed files\n\n");
	printf ("Additional Options:\n\n");
	printf (" -t\tTimeout value (default=2)\n");\
	printf (" -k\tDo not kill the process after timeout\n");
	printf (" -s\tSend the specified signal to kill the process. Default is SIGTERM, some apps need SIGKILL\n");
	printf (" -h\tPrint this message\n");
	printf (" -m\tMaximum concurrent processes (default=1)\n");
	printf (" -r\tFuzz this range of bytes in stead of trying the whole file (format low-high)\n");
	printf (" -f\tFuzz this range of fuzz values in stead of using all known fuzz values (format low-high)\n");
	printf (" -B\tBlob mode (replace with blobs)\n");
	printf (" -S\tString mode (replace with fancy strings)\n");
	printf(" -d\tDelay between kill and re-exec (default=1)\n");
	printf ("\nCommand:\n\n");
	printf ("\tQuoted command to execute to process the generated file.\n");
	printf ("\tUse %%FILENAME%% as a symbol to be replaced with the filename.\n");
	printf ("\tIf %%FILENAME%% is absent in the command string, filename will be appended automatically\n");
	printf ("\n\n");
	printf ("Example:\n\n");
	printf
	("notSPIKEfile -t 3 -d 1 -m 6 -r 30- -s SIGKILL -o FUZZY.gif test.gif \"/usr/bin/display -debug %%FILENAME%%\"\n\n");
	exit (ERR_OK);
}
