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


#ifndef _notSPIKEfile_H
#define _notSPIKEfile_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <netinet/in.h>

#include <libdis.h>

extern int blobcount;
extern int stringcount;

typedef enum
{
	F_BLOB,
	F_STRING,
} fuzztype;

#define ERR_UNMAPPED	4
#define ERR_EXECFAIL 	3
#define ERR_BAD			2
#define ERR_ENDOFBASE 	1
#define ERR_OK			0
#define ERR_CRASH 		220

#include <signal_utils.h>
#include <fuzz_utils.h>
#include <ptrace_utils.h>
#include <values.h>

void wait_helper();

void usage ();

#define ZERO(_,__) memset(_,0x0,__);

#define MAX_INSTRUCTION_SIZE 20	/* maximum intel instruction size is 20 bytes */

#ifndef active_processes
extern int active_processes;
#endif

#ifndef crashes
extern int crashes;
#endif

#ifndef killsignum
extern int killsignum;
#endif

#ifndef c_pid
extern pid_t c_pid;
#endif

#ifndef quiet
extern u_int8_t quiet;
#endif

#endif
