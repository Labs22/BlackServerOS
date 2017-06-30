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
#ifndef OPHCRACK_H
#define OPHCRACK_H

#include "misc.h"
#include "scheduler.h"
#include "list.h"
#include "table.h"
#include "hash.h"
#include "ophtask.h"
#include "ophstat.h"
#include "arg.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct ophcrack_t_ {
  scheduler_t *sched;

  list_t *tables;
  list_t *hashes;
  list_t *active;
  list_t *enabled;
  list_t *remaining;

  list_nd_t *hnd;
  ophstat_t *stat;

  int maxtid;
  int maxhid;
  table_kind_t maxtkind;

  int npwds_total;
  int npwds_found;

  uint64_t (*find_freeram)(void);
  uint64_t freeram;

  arg_t *arg;
} ophcrack_t;

ophcrack_t *ophcrack_alloc(int nthreads, arg_t *arg);
void ophcrack_free(ophcrack_t *crack);
void ophcrack_start(ophcrack_t *crack);
void ophcrack_stop(ophcrack_t *crack);

int ophcrack_setup_table(table_t *tbl);
void ophcrack_add_table(ophcrack_t *crack, table_t *tbl);
void ophcrack_add_hash(ophcrack_t *crack, hash_t *hsh);
void ophcrack_associate(ophcrack_t *crack, hash_t *hsh, table_t *tbl);
void ophcrack_reset(ophcrack_t *crack, int kind_mask, int remove);

void *ophcrack_run(void *arg);
void ophcrack_preload_check(ophcrack_t *crack);

ophtask_t *ophcrack_find(ophcrack_t *crack, ophtask_t *task, int id);
ophtask_t *ophcrack_lookup_idx(ophcrack_t *crack, ophtask_t *task, int id);
ophtask_t *ophcrack_lookup_end(ophcrack_t *crack, ophtask_t *task, int id);
ophtask_t *ophcrack_lookup_srt(ophcrack_t *crack, ophtask_t *task, int id);
ophtask_t *ophcrack_check(ophcrack_t *crack, ophtask_t *task, int id);
ophtask_t *ophcrack_preload_all(ophcrack_t *crack, ophtask_t *task, int id);
ophtask_t *ophcrack_preload_one(ophcrack_t *crack, ophtask_t *task, int id);
ophtask_t *ophcrack_bforce_all(ophcrack_t *crack, ophtask_t *task, int id);
ophtask_t *ophcrack_resolve_nt(ophcrack_t *crack, ophtask_t *task, int id);
ophtask_t *ophcrack_bforce_nt(ophcrack_t *crack, ophtask_t *task, int id);
ophtask_t *ophcrack_bforce_lm(ophcrack_t *crack, ophtask_t *task, int id);
ophtask_t *ophcrack_unload(ophcrack_t *crack, ophtask_t *task, int id);

void ophcrack_update(ophcrack_t *crack);
void ophcrack_found(ophcrack_t *crack, hash_t *hsh, table_t *tbl, int col, char *pwd);
void ophcrack_notfound(ophcrack_t *crack, hash_t *hsh);

int ophcrack_has_next(ophcrack_t *crack);
ophtask_t *ophcrack_next(ophcrack_t *crack);
void ophcrack_save(ophcrack_t *crack, FILE *file, int nice, int status);
void ophcrack_export_csv(ophcrack_t *crack, FILE *file, int *fields, char separator, char quote);

int ophcrack_pwd_cmp(const void *a, const void *b);
int ophcrack_col_cmp(const void *a, const void *b);
char *lm_from_nt(const char *pwd, int n, const uchar_t *lmhash);

#ifdef __cplusplus
}
#endif
#endif
