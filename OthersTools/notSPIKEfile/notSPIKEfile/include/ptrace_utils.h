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


#ifndef _PTRACE_UTILS_H
#define _PTRACE_UTILS_H

int F_getdata (pid_t pid, long addr, size_t size, void *buff);
void F_printregs (FILE *fp,struct user_regs_struct regs);
void F_getregs (pid_t pid, struct user_regs_struct *regs);
void F_libdis_print (FILE *fp,pid_t pid, int instructions, long addr);
void F_memdump (FILE *fp,pid_t pid, long addr, size_t size,char *symbol);
int F_execmon (char **argv, char **envp, time_t timeout,char *pOutfileProcess,char *pBasefile,fuzztype type,int fuzz,int byte);

#endif
