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
#ifndef HASH_H
#define HASH_H

#include <stdio.h>

#include "misc.h"
#include "list.h"
#include "table.h"

#ifdef  __cplusplus
extern "C" {
#endif

static const uchar_t empty_lmhash[8]  = {0xaa, 0xd3, 0xb4, 0x35, 0xb5, 0x14, 0x04, 0xee};
static const uchar_t empty_nthash[16] = {0x31, 0xd6, 0xcf, 0xe0, 0xd1, 0x6a, 0xe9, 0x31,
					 0xb7, 0x3c, 0x59, 0xd7, 0xe0, 0xc0, 0x89, 0xc0 };
static const char pwdump_nopwd[33] = "NO PASSWORD*********************";
static const char samdump_nopwd[33] = "00000000000000000000000000000000";
static const char empty_pwd[8]     = "/EMPTY/";

typedef enum { lm1, lm2, nt } hash_kind_t;

typedef struct htbl_t_ {
  table_t *tbl;
  int col;
  int covered;
} htbl_t;

typedef struct hash_t_ {
  hash_kind_t kind;
  int id;

  int uid;
  int done;

  uchar_t *hash;
  char *pwd;
  char *str;
  char *status;
  char info[64];

  int length;
  int category;
  int time;
  char *table;

  list_t *tables;
  list_nd_t *tnd;
  
  struct hash_t_ *lmhsh1;
  struct hash_t_ *lmhsh2;
} hash_t;

uchar_t *hash_alloc_hash(hash_kind_t kind);
char *hash_alloc_pwd(hash_kind_t kind);

hash_t *hash_alloc(hash_kind_t kind, int hlen, int plen, int idx);
void hash_free(hash_t *hsh);

void hash_add_table(hash_t *hsh, table_t *tbl);
int hash_extract_lmnt(char *buff, list_t *hashes, int id);
int hash_load_pwdump(list_t *hashes, FILE *file, int id);
int hash_load_sam(list_t *hashes, const char *dir, int id);
int hash_dump_sam(list_t *hashes, int id);
void hash_print(hash_t *hsh, FILE *file, int nice, int status, int hide);

#ifdef  __cplusplus
}
#endif
#endif
