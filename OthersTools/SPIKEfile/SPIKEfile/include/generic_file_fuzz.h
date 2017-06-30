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


#ifndef _GENERIC_FILE_FUZZ_H
#define _GENERIC_FILE_FUZZ_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/user.h>           /* user_regs_struct */
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <signal.h>

#include <spike.h>
#include <hdebug.h>
#include <filestuff.h>
#include <dlrpc.h>

#include <libdis.h>

#include <getopt.h>

#include <util.h>

#include <md5.h>

struct spike *our_spike;
struct spike *current_spike;

extern int opterr;
extern char *optarg;
extern int optind;


#define ERR_UNMAPPED	4
#define ERR_EXECFAIL 	3
#define ERR_BAD			2
#define ERR_ENDOFBASE 	1
#define ERR_OK			0
#define ERR_CRASH 		220

int main(int argc,char *argv[]);
void usage (char **argv);


#endif
