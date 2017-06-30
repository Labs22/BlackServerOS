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


#ifndef DLRPC_H
#define DLRPC_H

#include "dlargs.h"

#define MAXLINESIZE 20000
#define MAXVARIABLESIZE 15000


#define PARSEERROR 4
#define ENDOFLINE 5
#define ENDOFFUNCTION 6
#define SUCCESS 7
#define SUCCESSSTRING 8
#define SUCCESSINT 9
#define NOTINT 10
#define ENDOFFILE 11
#define COMMENTLINE 12
#define NEWFUNCTION 13
#define INFUNCTION 14
#define OLDFUNCTION 15
#define INTTYPE 16
#define UCHARTYPE 17
#define INSIDEQUOTES 18
#define FAILURE 19
#define PUCHARTYPE 20
#define SUCCESSCHAR 21

int s_parse (char *filename);

/*I guess for now we just throw away the return value*/
/*for Helium, we'll have to fix this - should be void *, 
actually*/
void dlrpc_call (struct dlargs *mydlargs);

int s_parse_buffer (unsigned char *buffer);

#endif
