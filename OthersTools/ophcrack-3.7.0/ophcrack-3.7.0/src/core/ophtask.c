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

#include "ophtask.h"
#include "original.h"
/*-------------------------------------------------------------------------*/
ophtask_t *ophtask_alloc(ophkind_t kind) {
  ophtask_t *task = (ophtask_t*)malloc(sizeof(ophtask_t));

  task->kind = kind;
  task->data = 0;
  task->stat = ophstat_alloc();

  switch (kind) {
  case preload_all: {
    ophload_t *load = (ophload_t*)malloc(sizeof(ophload_t));
    task->data = load;

    load->tables = list_alloc();
    load->tbl = 0;
    load->preload = preload_none;
  } break;

  case preload_one:
  case unload: {
    ophload_t *load = (ophload_t*)malloc(sizeof(ophload_t));
    task->data = load;
    
    load->tables = 0;
    load->tbl = 0;
    load->preload = preload_none;
  } break;
  
  case find:
  case lookup_idx:
  case lookup_end:
  case lookup_srt:
  case check: {
    ophwork_t *work = (ophwork_t*)malloc(sizeof(ophwork_t));
    task->data = work;

    work->hsh  = 0;
    work->htbl = 0;
    work->cmin = 0;
    work->cmax = 0;
    work->tosearch = 0;
  } break;

  case bforce_all: 
  case bforce_nt:
  case bforce_lm: 
  case resolve_nt: {
    ophbforce_t *force = (ophbforce_t*)malloc(sizeof(ophbforce_t));
    task->data = force;

    force->nhashes = 0;
    force->hashes = 0;
    force->hash = 0;
    force->found = 0;
    force->len = 1;
    force->count = 0;

    memset(force->idx, 0, sizeof(force->idx));
    memset(force->bloom, 0, sizeof(force->bloom));
  } break;

  case all:
    break;
  }

  return task;
}
/*-------------------------------------------------------------------------*/
void ophtask_free(ophtask_t *task) {
  int kind = task->kind;

  switch (kind) {
  case preload_all: {
    ophload_t *load = task->data;

    list_free(load->tables);
    free(load);
  } break;

  case preload_one: 
  case unload: {
    ophload_t *load = task->data;
    free(load);
  } break;

  case find:
  case lookup_idx:
  case lookup_end:
  case lookup_srt:
  case check: {
    ophwork_t *work = task->data;
    ophel_t **tosearch = work->tosearch;

    int cmax = work->cmax;
    int cmin = work->cmin;
    int size = cmax-cmin+1;
    int i;

    if (tosearch) {
      for (i=0; i<size; ++i)
	if (tosearch[i]) ophel_free(tosearch[i]); 
      
      free(tosearch);
    }
    
    free(work);
  } break;

  case bforce_all: 
  case bforce_nt:
  case bforce_lm:
  case resolve_nt: {
    ophbforce_t *force = task->data;

    hash_t **hashes = force->hashes;
    uchar_t **hash = force->hash;
    int *found = force->found;

    int nhashes = force->nhashes;
    int i;

    if (hash) {
      for (i=0; i<nhashes; ++i)
	if (hash[i]) free(hash[i]);

      free(hash);
    }

    if (found) free(found);
    if (hashes) free(hashes);

    free(force);
  } break;
 
  case all:
    break;
  }
  
  ophstat_free(task->stat);
  free(task);
}
/*-------------------------------------------------------------------------*/
void ophtask_convert(ophtask_t *task, ophkind_t from, ophkind_t to) {
  if (from == find) {
    ophwork_t *work = task->data;

    int cmin = work->cmin;
    int cmax = work->cmax;
    int size = cmax-cmin+1;
    int i;
    
    if (to == lookup_idx || to == lookup_end || to == lookup_srt) {
      task->kind = to;
      work->tosearch = (ophel_t**)malloc(size*sizeof(ophel_t));

      for (i=0; i<size; ++i)
        work->tosearch[i] = ophel_alloc();
    }
  }
}
