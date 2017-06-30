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


int
F_getdata (pid_t pid, long addr, size_t size, void *buff)
	{
	u_int8_t ix;
	u_int32_t word;
	if ( size % 4 )
		{
		fprintf (stderr, "F_getdata: size not mod 4. Fatal\n");
		exit (-1);
		}

	for ( ix = 0; ix < size; ix += 4 )
		{
		errno=0;
		word = ptrace (PTRACE_PEEKDATA, pid, addr + ix, 0);
		if (word == -1 && errno == EIO)
		{
			return (ERR_UNMAPPED);
		}
		memcpy (buff + ix, &word, 4);
		}

	return (ERR_OK);
	}



void
F_printregs (FILE *fp,struct user_regs_struct regs)
{
	fprintf (fp,"\n%%eax 0x%.8lx\n", regs.eax);
	fprintf (fp,"%%ebx 0x%.8lx\n", regs.ebx);
	fprintf (fp,"%%ecx 0x%.8lx\n", regs.ecx);
	fprintf (fp,"%%edx 0x%.8lx\n", regs.edx);
	fprintf (fp,"%%esi 0x%.8lx\n", regs.esi);
	fprintf (fp,"%%edi 0x%.8lx\n\n", regs.edi);

	fprintf (fp,"%%eip 0x%.8lx\t%%esp 0x%.8lx\n\n", regs.eip, regs.esp);
}

void
F_getregs (pid_t pid, struct user_regs_struct *regs)
{
	if ( ptrace (PTRACE_GETREGS, pid, NULL, regs) == -1 )
		perror ("ptrace(PTRACE_GETREGS)");
	return;
}

void
F_libdis_print (FILE *fp,pid_t pid, int instructions, long addr)
{
	int pos = 0;		  /* current position in buffer */
	int isize;			  /* size of instruction */
	struct instr i;		  /* representation of the code instruction */
	int icount = 0;
	char *ibuff;

	if ( !(ibuff = malloc (MAX_INSTRUCTION_SIZE * instructions)) )
	{
		fprintf (stderr, "F_libdis_print malloc failure. fatal.");
		exit (-9);
	}

	if ( F_getdata (pid, addr, MAX_INSTRUCTION_SIZE * instructions, ibuff) == ERR_UNMAPPED)
	{
		fprintf(fp,"%s+%.4x:%08lx <unmapped>\n\n","%eip",0,addr);
		free(ibuff);
		return; 
	}

	/* copied (verabatim?) from the libdisasm example code */
	disassemble_init (0, ATT_SYNTAX);
	while ( pos < MAX_INSTRUCTION_SIZE * instructions  /* not needed */
			&& icount++ < instructions )
	{
		isize = disassemble_address (ibuff + pos, &i);
		if ( isize )
		{
			/* print address and mnemonic */
			fprintf (fp,"%s+%.4x:%08lx %s", "%eip",pos,addr + pos, i.mnemonic);
			/* print operands */
			if ( i.destType )
			{
				fprintf (fp,"\t%s", i.dest);
				if ( i.srcType )
				{
					fprintf (fp,", %s", i.src);
					if ( i.auxType )
					{
						fprintf (fp,", %s", i.aux);
					}
				}
			}
			fprintf (fp,"\n");
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

/* i think i pasted this from somewhere also, just added the symbol stuff */
void
F_memdump (FILE *fp,pid_t pid, long addr, size_t size,char *symbol)
{
	unsigned char *buff;
	int ix=0;

	if (!symbol)
	{
		symbol = "";
	}

	if ( !(buff = malloc (size)) )
	{
		fprintf (stderr, "F_memdump: malloc error. fatal\n");
		abort();    
		exit (-9); /* XXX */
	}
	F_getdata (pid, addr, size, buff);
	fprintf (fp,"\n%s+%.4x:%.8lx  ",symbol,ix, addr);
	for ( ix = 1; ix <= size; ix++ )
	{
		fprintf (fp,"%.2x ", *(buff + ix - 1));
		if ( ix && !(ix % 16) )
		{
			if ( ix != size )
				fprintf (fp,"\n%s+%.4x:%.8lx  ",symbol,ix, addr + ix);
		}
	}
	fprintf (fp,"\n");
	free (buff);
}

/* execute and deal with the signals appropriately
**	return: ERR_CRASH on a crash 
** 			ERR_OK on successful execute and terminate
*/
int
F_execmon (char **argv, char **envp, time_t timeout, char *pOutfileProcess,char *pBasefile,fuzztype type,int fuzz,int byte)
{
	pid_t pid;
	int status;
	struct user_regs_struct regs;
	FILE *fp;
	char *pDumpfile;

	if ( !(pid = fork ()) )
	{ /* child */
		ptrace (PTRACE_TRACEME, 0, NULL, NULL);
/* XXX don't always want to close stdin/out/err XXX */
		close(fileno(stdin));
		close(fileno(stdout));
		close(fileno(stderr));
		if ( envp && argv )
			execve (argv[0], argv, envp);
		else if ( argv )
			execv (argv[0], argv);
		else
		{
			fprintf (stderr, "F_execmon: argv is NULL. fatal.\n");
			exit (4); /* XXX */
		}
	}
	else
	{ /* parent */
		c_pid = pid;
		signal (SIGALRM, F_alarm_killer);
monitor:
		alarm (timeout);
		waitpid (pid, &status, 0);
		alarm (0);
		if ( WIFEXITED (status) )
		{ /* program exited */
			if ( !quiet )
				printf ("Process %d exited with code %d\n", pid,
						WEXITSTATUS (status));
			return(ERR_OK);
		}
		else if ( WIFSIGNALED (status) )
		{ /* program ended because of a signal */
			if ( !quiet )
				printf ("Process %d terminated by unhandled signal %d\n", pid,
						WTERMSIG (status));
			return(ERR_OK);
		}
		else if ( WIFSTOPPED (status) )
		{ /* program stopped because of a signal */
			if ( !quiet )
				fprintf (stderr, "Process %d stopped due to signal %d (%s) ",
						 pid,
						 WSTOPSIG (status), F_signum2ascii (WSTOPSIG (status)));
		}
		switch ( WSTOPSIG (status) )
		{ /* the following signals are usually all we care about */
			case SIGILL:
			case SIGBUS:
			case SIGSEGV:
			case SIGSYS:
/* XXX this whole block can be cleaned up a great deal XXX */
				pDumpfile = malloc(strlen(pOutfileProcess)+1+8+strlen("-dump.txt")+1);
/* XXX check errors on F_getregs XXX */
				F_getregs (pid, &regs);
				sprintf(pDumpfile,"%s-%.8x-dump.txt",pBasefile,(unsigned)regs.eip);
				if ( !(fp = fopen(pDumpfile,"w")) )
				{
					perror("fopen");
					abort();
				}
				fprintf(fp,"TYPE %d: FUZZ %d: BYTE %d\n",type,fuzz,byte);
				F_printregs (fp,regs);
				F_libdis_print (fp,pid, 9, regs.eip);
/* XXX aesthetics: consider removing "STACK DUMP" label XXX */
				F_memdump (fp,pid, regs.esp, 128,"%esp");
				if ( (ptrace (PTRACE_CONT, pid, NULL,
							  (WSTOPSIG (status) == SIGTRAP) ? 0 : WSTOPSIG (status))) == -1 )
				{
					perror("ptrace");
				}
/* XXX check error (?) XXX */
				ptrace(PTRACE_DETACH,pid,NULL,NULL);
				fclose(fp);
				return(ERR_CRASH);
		}
/* deliver the signal and keep tracing */
		if ( !quiet )
			fprintf (stderr, "Continuing...\n");
		if ( (ptrace (PTRACE_CONT, pid, NULL,
					  (WSTOPSIG (status) == SIGTRAP) ? 0 : WSTOPSIG (status))) == -1 )
		{
			perror("ptrace");
		}
		goto monitor;    
	}
	return(ERR_OK);
}
