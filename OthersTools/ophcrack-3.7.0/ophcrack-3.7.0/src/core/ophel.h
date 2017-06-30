/*
 *   
 *   Ophcrack is a Lanmanager/NTLM hash cracker based on the faster time-memory
 *   trade-off using rainbow tables. 
 *   
 *   Created with the help of: Maxime Mueller, Luca Wullschleger, Claude
 *   Hochreutiner, Andreas Huber and Etienne Dysli.
 *   
 *   Copyright (c) 2008 Philippe Oechslin, Cedric Tissieres, Bertrand Mesot
 *   
 *   Ophcrack is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *   
 *   Ophcrack is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *   
 *   You should have received a copy of the GNU General Public License
 *   along with Ophcrack; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *   
 *   This program is released under the GPL with the additional exemption 
 *   that compiling, linking, and/or using OpenSSL is allowed.
 *   
 *   
 *   
 *   
*/
#ifndef OPHEL_H
#define OPHEL_H

#include <stdint.h>
#include "misc.h"
#include "ophstat.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct ophel_t_ {
  int col;
  char pwd[MAX_PWD_LEN+1];
  uint64_t prefix;
  uint64_t postfix;

  uint64_t start;

  uint64_t low;
  uint64_t high;
  uint64_t offset;

  ophstat_t *stat;
} ophel_t;

ophel_t *ophel_alloc(void);
void ophel_free(ophel_t *el);

#ifdef  __cplusplus
}
#endif
#endif
