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
#include <stdio.h>

#define ZERO(_,__) memset(_,0x0,__);

#define MAX_INSTRUCTION_SIZE 20 /* maximum intel instruction size is 20 bytes */

void F_getregs (pid_t pid, struct user_regs_struct *regs);
void F_printregs (FILE *fp,struct user_regs_struct regs);
void F_memdump (pid_t pid, long addr, size_t size,FILE *fp);
void F_getdata (pid_t pid, long addr, size_t size, void *buff);

void F_execmon_orig (char **argv, char **envp,time_t timeout,u_int8_t *pLogfile);

int F_execmon (char **argv, char **envp, time_t timeout, char *pBasefile,int fuzz,int byte);

char *F_signum2ascii (int signal);
char **F_build_argv (u_int8_t *program, u_int8_t *buff);
char *
F_dstring_replace (char *buff, char *before, const char *after);
void F_libdis_print (pid_t pid, int instructions, long addr,FILE *fp);
void F_alarm_killer(int status);


