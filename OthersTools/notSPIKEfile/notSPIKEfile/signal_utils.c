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

pid_t c_pid;

struct signal_description
{
	int sig;
	char *name;
	char *abbr;
} ;


struct signal_description signal_description[] = 
{
	{SIGHUP,"Hangup","SIGHUP"},
	{SIGINT,"Interrupt","SIGINT"},
	{SIGQUIT,
		"Quit","SIGQUIT"},
	{SIGILL,
		"Illegal instruction","SIGILL"},
	{SIGTRAP,
		"Trace trap","SIGTRAP"},
	{SIGABRT,
		"Abort","SIGABRT"},
	{SIGBUS,
		"BUS error","SIGBUS"},
	{SIGFPE,
		"Floating-point exception","SIGFPE"},
	{SIGKILL,
		"Kill","SIGKILL"},
	{SIGUSR1,
		"User-defined signal 1","SIGUSR1"},
	{SIGSEGV,
		"Segmentation violation","SIGSEGV"},
	{SIGUSR2,
		"User-defined signal 2","SIGUSR2"},
	{SIGPIPE,
		"Broken pipe","SIGPIPE"},
	{SIGALRM,
		"Alarm clock","SIGALRM"},
	{SIGTERM,
		"Termination","SIGTERM"},
	{SIGSTKFLT,
		"Stack fault","SIGSTKFLT"},
	{SIGCHLD,
		"Child status has changed","SIGCHLD"},
	{SIGCONT,
		"Continue","SIGCONT"},
	{SIGSTOP,
		"Stop","SIGSTOP"},
	{SIGTSTP,
		"Keyboard stop","SIGSTP"},
	{SIGTTIN,
		"Background read from tty","SIGTTIN"},
	{SIGTTOU,
		"Background write to tty","SIGTTOU"},
	{SIGURG,
		"Urgent condition on socket","SIGURG"},
	{SIGXCPU,
		"CPU limit exceeded","SIGXCPU"},
	{SIGXFSZ,
		"File size limit exceeded","SIGXFSZ"},
	{SIGVTALRM,
		"Virtual alarm clock","SIGVTALRM"},
	{SIGPROF,
		"Profiling alarm clock","SIGPROF"},
	{SIGWINCH,
		"Window size change","SIGWINCH"},
	{SIGIO,
		"I/O now possible","SIGIO"},
	{SIGPWR,
		"Power failure restart","SIGPWR"},
	{SIGSYS,"Bad system call","SIGSYS"},
	{32, "Realtime Signal #0",NULL},
	{-1,NULL}

};


void sigchld_handler()
{
	int status;
	wait(&status);
	active_processes--;

	if ( (WIFEXITED(status)) && (WEXITSTATUS(status)==ERR_ENDOFBASE) )
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
	else if ( WIFEXITED(status) && (WEXITSTATUS(status)==ERR_EXECFAIL) )
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
		if ( WEXITSTATUS(status) == ERR_CRASH )
		{
			
			crashes++;
		}
	}

	return;
}

int F_ascii2signum(char *sig)
{
	int ix=0;
	while ( signal_description[ix].abbr && (signal_description[ix].sig != -1) )
	{
		if ( !strcmp(sig,signal_description[ix].abbr) )
		{
			return(signal_description[ix].sig);
		}
		ix++;
	}
	return(ERR_BAD);
}

char *
F_signum2ascii(int sig)
{
	int ix=0;
	while ( signal_description[ix].sig != -1 )
	{
		if ( signal_description[ix].sig == sig )
		{
			return(signal_description[ix].name);
		}
		ix++;
	}
	return("Unknown signal");
}


void
F_alarm_killer (int status)
{
	kill(c_pid,killsignum);
	return;
}

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
