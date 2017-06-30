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


/*dlargs.h, where we define the dlargs structure (a linked list)
  and the functions used to play with it*/

#ifndef DLARGS_H
#define DLARGS_H

/*the first "data" is the function name, and the first datatype is
  the return dataType*/

struct dlargs
{
  struct dlargs *next;
  unsigned char *data;
  int dataType;
};


void dlargs_destroy (struct dlargs *mydlargs);

int dlargs_add (int variabletype, struct dlargs *mydlargs, char *newbuffer);

/*constructor for our linked list*/
struct dlargs *dlargs_new ();

/*length of the linked list*/
int dlargs_getlength (struct dlargs *mydlargs);


#define VOIDRETURNTYPE 1
#define INTRETURNTYPE 2


#endif
