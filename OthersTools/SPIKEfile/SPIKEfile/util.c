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

pid_t c_pid;

void
F_getdata (pid_t pid, long addr, size_t size, void *buff)
	{
	u_int8_t ix;
	u_int32_t word;
	if ( size % 4 )
		{
		fprintf (stderr, "F_getdata: size not mod 4. Fatal\n");
		exit (ERR_BAD);
		}

	for ( ix = 0; ix < size; ix += 4 )
		{
		word = ptrace (PTRACE_PEEKDATA, pid, addr + ix, 0);
		memcpy (buff + ix, &word, 4);
		}

	return;
	}

void
F_printregs (FILE *fp,struct user_regs_struct regs)
	{
	fprintf (fp, "%%eax 0x%.8lx\n", regs.eax);
	fprintf (fp, "%%ebx 0x%.8lx\n", regs.ebx);
	fprintf (fp, "%%ecx 0x%.8lx\n", regs.ecx);
	fprintf (fp, "%%edx 0x%.8lx\n", regs.edx);
	fprintf (fp, "%%esi 0x%.8lx\n", regs.esi);
	fprintf (fp, "%%edi 0x%.8lx\n\n", regs.edi);

	fprintf (fp, "%%eip 0x%.8lx\t%%esp 0x%.8lx\n\n", regs.eip, regs.esp);
	}

void
F_getregs (pid_t pid, struct user_regs_struct *regs)
	{

	if ( ptrace (PTRACE_GETREGS, pid, NULL, regs) == -1 )
		{
		perror ("ptrace(PTRACE_GETREGS)");
		abort();
	}

	return;
	}

void
F_libdis_print (pid_t pid, int instructions, long addr, FILE * fp)
	{
	size_t pos = 0;			  /* current position in buffer */
	u_int8_t isize;			  /* size of instruction */
	struct instr i;		  /* representation of the code instruction */
	u_int16_t icount = 0;

	u_int8_t *ibuff;

	if ( !(ibuff = malloc (MAX_INSTRUCTION_SIZE * instructions)) )
		{
		fprintf (stderr, "F_libdis_print malloc failure. fatal.");
		exit (-1);
		}

	F_getdata (pid, addr, MAX_INSTRUCTION_SIZE * instructions, ibuff);

	disassemble_init (0, ATT_SYNTAX);

	while ( pos < MAX_INSTRUCTION_SIZE * instructions  /* not needed */
			&& icount++ < instructions )
		{
		isize = disassemble_address (ibuff + pos, &i);
		if ( isize )
			{
			/* print address and mnemonic */
			fprintf (fp, "%08lx: %s", addr + pos, i.mnemonic);
			/* print operands */
			if ( i.destType )
				{
				fprintf (fp, "\t%s", i.dest);
				if ( i.srcType )
					{
					fprintf (fp, ", %s", i.src);
					if ( i.auxType )
						{
						fprintf (fp, ", %s", i.aux);
						}
					}
				}
			fprintf (fp, "\n");
			pos += isize;
			}
		else
			{
			/* invalid/unrecognized instruction */
			pos++;
			}
		}


	disassemble_cleanup ();
	free (ibuff);
	}

void
F_memdump (pid_t pid, long addr, size_t size, FILE * fp)
	{
	u_int8_t *buff;
	size_t ix;

	if ( !(buff = malloc (size)) )
		{
		fprintf (stderr, "F_memdump: malloc error. fatal\n");
		exit (ERR_BAD);
		}
	F_getdata (pid, addr, size, buff);

	fprintf (fp, "\n%.8lx: ", addr);
	for ( ix = 1; ix <= size; ix++ )
		{
		fprintf (fp, "%.2x ", *(buff + ix - 1));
		if ( ix && !(ix % 16) )
			{
			if ( ix != size )
				fprintf (fp, "\n%.8lx: ", addr + ix);
			}
		}
	fprintf (fp, "\n");
	free (buff);
	}

extern int quiet;
/* a do it all exec and deal with signals */
/* return 1 on a crash */
int
F_execmon (char **argv, char **envp, time_t timeout,char *pBasefile,int fuzz,int byte)
{
	pid_t pid;
	int status;
	struct user_regs_struct regs;
	FILE *fp;
	char *pDumpfile;

	if ( !(pid = fork ()) )
	{				/* child */
		ptrace (PTRACE_TRACEME, 0, NULL, NULL);
		/* not necessarily what we want to do, but thats what we do 
		** by default for now
		*/
		close(0);close(1);close(2);
		if ( envp && argv )
			execve (argv[0], argv, envp);
		else if ( argv )
			execv (argv[0], argv);
		else
		{
			fprintf (stderr, "F_execmon: argv is NULL. fatal.\n");
			exit (-1);
		}
	}
	else
	{				/* parent */
		c_pid = pid;
		signal (SIGALRM, F_alarm_killer);
		monitor:
		alarm (timeout);
		waitpid (pid, &status, 0);
		alarm (0);
		if ( WIFEXITED (status) )
		{
			if ( !quiet )
				printf ("Process %d exited with code %d\n", pid,
						WEXITSTATUS (status));
			return (ERR_OK);
		}
		else if ( WIFSIGNALED (status) )
		{
			if ( !quiet )
				printf ("Process %d terminated by unhandled signal %d\n", pid,
						WTERMSIG (status));
			return (ERR_OK);
		}
		else if ( WIFSTOPPED (status) )
		{
			if ( !quiet )
				fprintf (stderr, "Process %d stopped due to signal %d (%s) ",
						 pid,
						 WSTOPSIG (status), F_signum2ascii (WSTOPSIG (status)));
		}
		switch ( WSTOPSIG (status) )
		{
			case SIGILL:
			case SIGBUS:
			case SIGSEGV:
			case SIGSYS:
				/* its over */
/* do we want to let it try to handle bad signals? just kill for now */


				pDumpfile = malloc(strlen(pBasefile)+1+8+strlen("-dump.txt")+1);
				  F_getregs (pid, &regs);
				sprintf(pDumpfile,"%s-%.8x-dump.txt",pBasefile,(unsigned)regs.eip);
				if ( !(fp = fopen(pDumpfile,"w")) )
				{
					perror("fopen");
					abort();
				}
			  fprintf(fp,"FUZZSTRING: %d FUZZVARIABLE: %d\n",fuzz,byte);
				F_printregs (fp,regs);
				F_libdis_print (pid,9,regs.eip,fp);
				fprintf(fp,"STACK DUMP\n");
				F_memdump (pid, regs.esp, 128,fp);
	 

				ptrace (PTRACE_KILL, pid, NULL, NULL);
				ptrace(PTRACE_DETACH,pid,NULL,NULL);
				fflush(fp);
				fclose(fp);
				return (-1);

		}
/*
 otherwise, we want to deliver the signal and keep tracing
*/
		if ( !quiet )
			fprintf (stderr, "Continuing...\n");

		if ( (ptrace (PTRACE_CONT, pid, NULL,
					  (WSTOPSIG (status) == SIGTRAP) ? 0 : WSTOPSIG (status))) == -1 )
		{
			perror("ptrace");
		}
		goto monitor;    
	}
	return (ERR_OK);
}

char *
F_signum2ascii (int signal)
	{
	switch ( signal )
		{
		case 1:
			return("Hangup");
		case 2:
			return("Interrupt");
		case 3:
			return("Quit");
		case 4:
			return("Illegal instruction");
		case 5:
			return("Trace trap");
		case 6:
			return("Abort");
		case 7:
			return("BUS error");
		case 8:
			return("Floating-point exception");
		case 9:
			return("Kill");
		case 10:
			return("User-defined signal 1");
		case 11:
			return("Segmentation violation");
		case 12:
			return("User-defined signal 2");
		case 13:
			return("Broken pipe");
		case 14:
			return("Alarm clock");
		case 15:
			return("Termination");
		case 16:
			return("Stack fault");
		case 17:
			return("Child status has changed");
		case 18:
			return("Continue");
		case 19:
			return("Stop");
		case 20:
			return("Keyboard stop");
		case 21:
			return("Background read from tty");
		case 22:
			return("Background write to tty");
		case 23:
			return("Urgent condition on socket");
		case 24:
			return("CPU limit exceeded");
		case 25:
			return("File size limit exceeded");
		case 26:
			return("Virtual alarm clock");
		case 27:
			return("Profiling alarm clock");
		case 28:
			return("Window size change");
		case 29:
			return("I/O now possible");
		case 30:
			return("Power failure restart");
		case 31:
			return("Bad system call");
		case 32:
			return("Realtime Signal #0");
		default:
			return("UNKNOWN");


		}
	}


/* buff is a commandline string like '-o blah -z blah --file out' */
/* program is full path to a program like "/usr/bin/blah" */
/* return value is an array of pointers suitable for use in execv */
/* i could have used strtok i guess, but its gay */
char **
F_build_argv (u_int8_t *program, u_int8_t *buff)
	{
	u_int8_t *ptr;
	char **argp;
	u_int8_t *string;
	u_int16_t ix = 1;
	u_int16_t elements;
	string = strdup (buff);
	ptr = string;


	while ( (ptr = strstr (ptr, " ")) )
		{

		ix++;
		while ( *(ptr++) == 0x20 )
			{
			if ( !(*ptr) )	/* so multiple spaces is ok, even at the end */
				{
				ix--;
				}
			}
		}
	elements = ++ix;

	if ( !(argp = malloc (sizeof (char *) * (elements + 1))) )
		{

		fprintf (stderr, "F_build_argv: malloc failed. fatal\n");
		exit (ERR_BAD);
		}

	memset (argp, 0x0, sizeof (char *) * (elements + 1));

	for ( ix = 0, ptr = string; ix < elements - 1; ix++ )
		{
		char *end;
		while ( *ptr && (*ptr == 0x20) )
			ptr++;
		end = ptr;
		while ( *end && ((*end) != 0x20) )
			end++;
		*end = 0x0;

		argp[ix] = strdup (ptr);
		ptr = end + 1;

		}
	argp[ix] = NULL;


	return(argp);
	}





void
F_alarm_killer (int status)
	{
	if (!quiet) fprintf (stderr, "Killing %d\n", c_pid);
	kill (c_pid, 9);
	return;
	}

/* buff must be a malloc,realloc,calloc or strdup string */
char *
F_dstring_replace (char *buff, char *before, const char *after)
	{
	u_int8_t *ptr = buff;
	u_int16_t replacements = 0;


	while ( (ptr = strstr (ptr, before)) )
		{
		replacements++;
		ptr++;
		}

	if ( strlen (after) == strlen (before) )
		{
		ptr = buff;
		while ( (ptr = strstr (ptr, before)) )
			{
			memcpy (ptr, after, strlen (after));
			ptr++;
			}
		}
	else
		{
		size_t size;

		if ( strlen (after) > strlen (before) )
			{
			int start,stop;

			size =
			strlen(buff)
			-strlen(before)*replacements
			+strlen(after)*replacements
			+1;




			start = strlen(buff)+1;
			stop = size-start;
			buff = realloc (buff, size);
			memset(buff+start,0x0,stop);



			ptr = buff;

			while ( (ptr = strstr (ptr, before)) )
				{
				char *dst,*src;
				size_t len;

				dst = ptr+strlen(after);
				src = ptr+strlen(before);
				len = strlen(src)+1;


				memmove ((void *) (dst),
						 (void *) (src),
						 (size_t) len);


				memcpy (ptr, after, strlen (after));
				ptr += strlen (after);
				}


			}
		else
			{
			ptr = buff;
			size = strlen (buff) + 1;
			size -= replacements * (strlen (before) - strlen (after));

			while ( (ptr = strstr (ptr, before)) )
				{
				memcpy (ptr, after, strlen (after));
				memmove ((void *) (ptr + strlen (after)),
						 (void *) (ptr + strlen (before) + 1),
						 (size_t) ((buff + size) - (int) ptr));


				}
			*(buff + size - 1) = 0x0;
			}


		}
	return(buff);
	}
