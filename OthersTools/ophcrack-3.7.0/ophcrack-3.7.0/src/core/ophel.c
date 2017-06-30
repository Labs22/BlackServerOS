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
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ophel.h"
/*-------------------------------------------------------------------------*/
ophel_t *ophel_alloc(void) {
  ophel_t *el = (ophel_t*)malloc(sizeof(ophel_t));
  el->col = 0;
  memset(el->pwd, 0, MAX_PWD_LEN+1);
  el->prefix = 0;
  el->postfix = 0;
  el->start = 0;
  el->low = 0;
  el->high = 0;
  el->offset = 0;
  el->stat = ophstat_alloc();

  assert(el->stat != 0);

  return el;
}
/*-------------------------------------------------------------------------*/
void ophel_free(ophel_t *el) {
  ophstat_free(el->stat);
  free(el);
}
