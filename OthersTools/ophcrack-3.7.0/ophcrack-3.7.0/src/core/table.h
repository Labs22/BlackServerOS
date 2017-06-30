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
#ifndef TABLE_H
#define TABLE_H

#include <stdio.h>
#include <stdint.h>

#include "misc.h"
#include "list.h"

#include "proba_info.h"

#ifdef WIN32
#define fstat _fstati64
#define stat _stati64
#define fseeko fseeko64
#endif

#ifdef  __cplusplus
extern "C" {
#endif

typedef enum {
  unknown = 0,
  lmalphanum10k,
  lmalphanum5k,
  lmextended,
  ntextended,
  lmgermanv1,
  lmgermanv2,
  ntdict,
  ntnine,
  nteight,
  ntnum,
  ntseven,
  lmflash,
  nteightxl,
  ntspecialxl,
  ntprobafree,
  ntproba10g,
  ntproba60g
} table_kind_t;

typedef enum {
  preload_none = 0x00,
  preload_init = 0x01,
  preload_idx  = 0x02,
  preload_end  = 0x04,
  preload_srt  = 0x08,
  preload_full = preload_init + preload_idx + preload_end + preload_srt
} table_preload_t;

typedef struct table_t_ {
  table_kind_t kind;
  uint32_t code;

  char *path;
  char *name;

  int id;
  int idx;
  int cmin;
  int cmax;
  int ncols;
  int offset;
  int enabled;
  int active;

  FILE *idxfile;
  FILE *endfile;
  FILE *srtfile;

  uint8_t shared_init;

  uint64_t inisize;
  uint64_t idxsize;
  uint64_t endsize;
  uint64_t srtsize;

  const uint64_t *sizes;

  void *param;
  char *idxmem;
  char *endmem;
  char *srtmem;

  void *(*init)(void*);
  void  (*cleanup)(void*);

  void (*find)(void*, void*, void*);
  int (*lookup_idx)(void*, void*, void*);
  int (*lookup_end)(void*, void*, void*);
  int (*lookup_srt)(void*, void*, void*);

  int (*check)(void*, void*, void*);
  int (*isvalid)(void*, void*);
} table_t;

table_t *table_alloc(uint32_t code, char *path, int idx);
void table_free(table_t *tbl);

int table_load(table_t *tbl);
void table_set_size(table_t *tbl);
int table_verify(table_t *tbl);
uint64_t table_preload(table_t *tbl, table_preload_t preload);
uint64_t table_unload(table_t *tbl, table_preload_t preload);

uint64_t table_size(table_t *tbl);
uint64_t table_preload_size(table_t *tbl, table_preload_t preload);
int table_open(list_t *tables, const char *dir, const char *tblstr);

table_preload_t table_preload_state(table_t *tbl);

table_kind_t table_kind(uint32_t code);
const char *table_string(table_kind_t kind);

#ifdef  __cplusplus
}
#endif
#endif
