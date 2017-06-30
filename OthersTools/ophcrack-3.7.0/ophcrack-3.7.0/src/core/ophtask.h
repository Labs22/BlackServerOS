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
#ifndef OPHTASK_H
#define OPHTASK_H

#include "hash.h"
#include "table.h"
#include "ophel.h"
#include "ophstat.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define BLOOM_SIZE 32768 /* 2^20 positions */

typedef enum { preload_all = 0x001,
	       preload_one = 0x002,
	       unload      = 0x004,
	       find        = 0x008,
	       lookup_idx  = 0x010,
	       lookup_end  = 0x020,
	       lookup_srt  = 0x040,
	       check       = 0x080,
	       bforce_all  = 0x100,
	       bforce_nt   = 0x200,
	       bforce_lm   = 0x400,
	       resolve_nt  = 0x800,
	       all         = 0xfff } ophkind_t;

typedef struct ophload_t_ {
  list_t *tables;
  table_t *tbl;
  table_preload_t preload;
} ophload_t;

typedef struct ophwork_t_ {
  hash_t *hsh;
  htbl_t *htbl;

  int cmin;
  int cmax;

  ophel_t **tosearch;
} ophwork_t;

typedef struct ophbforce_t_ {
  int nhashes;

  hash_t **hashes;
  uchar_t **hash;
  int *found;

  int idx[8];
  int len;

  int count;
  
  int bloom[BLOOM_SIZE];
} ophbforce_t;

typedef struct ophtask_t_ {
  ophkind_t kind;
  void *data;
  ophstat_t *stat;
} ophtask_t;

ophtask_t *ophtask_alloc(ophkind_t kind);
void ophtask_free(ophtask_t *task);
void ophtask_convert(ophtask_t *task, ophkind_t from, ophkind_t to);

#ifdef  __cplusplus
}
#endif
#endif
