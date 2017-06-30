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
#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#include "list.h"
#include "table.h"
#include "hash.h"
#include "ophtask.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef enum { msg_done = 0, 
	       msg_preload,
	       msg_unload,
	       msg_work,
	       msg_found,
	       msg_bforce,
	       msg_pause } msg_kind_t;

typedef struct msg_done_t_ {
  ophkind_t kind;
} msg_done_t;

typedef struct msg_load_t_ {
  table_t *tbl;
  table_preload_t preload;
  int done;
  uint64_t size;
} msg_load_t;

typedef struct msg_work_t_ {
  hash_t *hsh;
  table_t *tbl;
  ophkind_t kind;
  int cmin;
  int cmax;
} msg_work_t;

typedef struct msg_found_t_ {
  hash_t *hsh;
  table_t *tbl;
  int col;
} msg_found_t;

typedef struct msg_bforce_t_ {
  int done;
  uint64_t count;
} msg_bforce_t;

typedef struct message_t_ {
  msg_kind_t kind;
  uint32_t id;

  void *data;

  struct timeval time;
} message_t;

void message_init(void);
void message_free(message_t *msg);
void message_insert_first(list_t *messages);

message_t *message_get(void);
message_t *message_tryget(void);

void message_done(ophkind_t kind);
void message_preload(table_t *tbl, table_preload_t preload, int done, uint64_t size);
void message_unload(table_t *tbl, uint64_t size);
void message_work(hash_t *hsh, table_t *tbl, ophkind_t kind, int cmin, int cmax);
void message_found(hash_t *hsh, table_t *tbl, int col);
void message_bforce(int done, uint64_t count);
void message_pause(void);

#ifdef  __cplusplus
}
#endif
#endif
