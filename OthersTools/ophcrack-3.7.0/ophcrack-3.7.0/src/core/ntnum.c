/*
 *   
 *   Ophcrack is a Lanmanager/NTLM hash cracker based on the faster time-memory
 *   trade-off using rainbow tables. 
 *   
 *   Created with the help of: Maxime Mueller, Luca Wullschleger, Claude
 *   Hochreutiner, Andreas Huber and Etienne Dysli.
 *   
 *   Copyright (c) 2009 Philippe Oechslin, Cedric Tissieres, Bertrand Mesot
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
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "ntnum.h"
#include "table.h"
#include "hash.h"
#include "ophel.h"
#include "original.h"
#include "lmtable.h"

static uint64_t ntnum_sizes[10] = { 4, 7086244, 259203456, 518406912, 259204606, 518409212, 259212464, 518424928, 259218600, 518437200};
/*-------------------------------------------------------------------------*/
int ntnum_setup(void *tbl_) {
  table_t *tbl = tbl_;

  tbl->ncols  = 15000;
  tbl->offset = 100000;
  tbl->sizes  = ntnum_sizes;

  tbl->find    = ntnum_find;
  tbl->check   = ntnum_check;
  tbl->isvalid = ntnum_isvalid;

  tbl->lookup_idx = lmtable_lookup_idx;
  tbl->lookup_end = lmtable_lookup_end;
  tbl->lookup_srt = ntnum_lookup_srt;

  return 1;
}
/*-------------------------------------------------------------------------*/
void ntnum_find(void *hsh_, void *tbl_, void *el_) {
  hash_t  *hsh  = hsh_;
  table_t *tbl  = tbl_;
  ophel_t *el   = el_;
  uchar_t *hash = hsh->hash;
  uchar_t *pwd  = (uchar_t*)el->pwd;
  ophstat_t *stat = el->stat;

  int c;
  int col     = el->col;
  int ncols   = tbl->ncols;
  int n_redux = col + tbl->idx * tbl->offset;

  uchar_t tmp[16];

  /* Hash and redux until the last column. */
  
  ntnum_mkredux(tbl, hash, pwd, n_redux);

  for (c=1; c<ncols-col; ++c) {
    ++n_redux;

    ntnum_mkhash(pwd, tmp);
    ntnum_mkredux(tbl, tmp, pwd, n_redux);
    ++stat->hredux;
  }

  /* Compute the prefix and postfix. */
  uint32_t prefix  = ntnum_bin11(pwd, 6);
  uint16_t postfix = (uint16_t) ntnum_bin11(pwd+6,6);
  
  el->prefix  = prefix;
  el->postfix = postfix;
}
/*-------------------------------------------------------------------------*/
int ntnum_lookup_srt(void *hsh_, void *tbl_, void *el_) {
  table_t *tbl = tbl_;
  ophel_t *el  = el_;

  uint64_t offset = el->offset;
  uint32_t start;

  ophstat_t *stat = el->stat;
  ++stat->start;

  /* If the .start file has been preloaded, then we look into the memory. */

  if (tbl->srtmem) {
    uint32_t *mem = (uint32_t*)(tbl->srtmem + offset);
    start = mem[0];
  }

  /* Otherwise, we access the disk. */

  else {
    FILE *srtfile = tbl->srtfile;

    fseeko(srtfile, offset, SEEK_SET);
    fread(&start, sizeof(start), 1, srtfile);
    ++stat->fseek_srt;
  }

  /* Make sure that start is in the correct byte order. */

  start = ftohl(start);

  /* Convert it to a valid password. */

  uchar_t *pwd = (uchar_t*)el->pwd;

  uint32_t len = (start >= 1000000000) ? 10 : (start >= 100000000) ? 9 : 
    (start >= 10000000) ? 8 : (start >= 1000000) ? 7 : (start >= 100000) ? 6 : 
    (start >= 10000) ? 5 : (start >= 1000) ? 4 : (start >= 100) ? 3 : 
    (start >= 10) ? 2 : 1;

  memset(pwd, 0, 13);
  memset(pwd, '0', 10-len);
  sprintf((char*)pwd+10-len, "%u", start);

  return 1;
}
/*-------------------------------------------------------------------------*/
int ntnum_check(void *hsh_, void *tbl_, void *el_) {
  hash_t *hsh   = hsh_;
  table_t *tbl  = tbl_;
  ophel_t *el   = el_;
  uchar_t *hash = hsh->hash;
  uchar_t *pwd  = (uchar_t*)el->pwd;
  ophstat_t *stat = el->stat;

  int c;
  int col     = el->col;
  int ncols   = tbl->ncols;
  int n_redux = tbl->idx * tbl->offset;

  uchar_t tmp[16];

  /* Hash and redux until the starting column. */

  stat->falarm_hredux = 0;
  
  for (c=ncols-col; c<ncols; ++c) {
    ntnum_mkhash(pwd, tmp);
    ntnum_mkredux(tbl, tmp, pwd, n_redux);

    ++n_redux;
    ++stat->hredux;
    ++stat->falarm_hredux;
  }  

  /* Check if we got the same hash. */

  ntnum_mkhash(pwd, tmp);
  
  if (memcmp(tmp, hash, sizeof(tmp)) == 0) {
    ++stat->match_table;
    stat->falarm_hredux = 0;
    return 1;
  } 

  else {
    ++stat->falarm;
    return 0;
  }
}
/*-------------------------------------------------------------------------*/
int ntnum_isvalid(void *hsh_, void *tbl_) {
  hash_t *hsh = hsh_;

  return hsh->kind == nt ? 1 : 0;
}
/*-------------------------------------------------------------------------*/
void ntnum_mkredux(table_t *tbl, uchar_t *hash, uchar_t *pwd, 
			int n_redux) {

  /* Convert the hash to four unsigned 32-bit integers. */
  
  uint32_t idx[4];
  
  idx[0] = ftohl(*(uint32_t*)hash) ^ (uint32_t) n_redux;
  idx[1] = ftohl(*(uint32_t*)(hash+4));
  idx[2] = ftohl(*(uint32_t*)(hash+8));
  idx[3] = ftohl(*(uint32_t*)(hash+12));
    
  /* Compute the password */
  uint64_t q = (uint64_t) idx[1] << 32 | (uint64_t) idx[0];
  uint64_t l = (uint64_t) idx[3] << 32 | (uint64_t) idx[2];
  l = l % 1000000000000LLU;
  
  uint32_t len = (l >= 100000000000LLU) ? 12 : (l >= 10000000000LLU) ? 11 : 
    (l >= 1000000000) ? 10 : (l >= 100000000) ? 9 : (l >= 10000000) ? 8 : 
    (l >= 1000000) ? 7 : (l >= 100000) ? 6 : (l >= 10000) ? 5 : 
    (l >= 1000) ? 4 : (l >= 100) ? 3 : (l >= 10) ? 2 : 1;
  
  q = (len == 12) ? q % 1000000000000LLU : (len == 11) ? q % 100000000000LLU :
    (len == 10) ? q % 10000000000LLU : (len == 9) ? q % 1000000000 :
    (len == 8) ? q % 100000000 : (len == 7) ? q % 10000000 :
    (len == 6) ? q % 1000000 : (len == 5) ? q % 100000 :
    (len == 4) ? q % 10000 : (len == 3) ? q % 1000 :
    (len == 2) ? q % 100 : q % 10;
  
  uint32_t len_q = (q >= 100000000000LLU) ? 12 : (q >= 10000000000LLU) ? 11 : 
    (q >= 1000000000) ? 10 : (q >= 100000000) ? 9 : (q >= 10000000) ? 8 : 
    (q >= 1000000) ? 7 : (q >= 100000) ? 6 : (q >= 10000) ? 5 : 
    (q >= 1000) ? 4 : (q >= 100) ? 3 : (q >= 10) ? 2 : 1;
  
  memset(pwd, 0, 13);
  memset(pwd, '0', len-len_q);
  snprintf((char*)pwd+len-len_q, len_q+1, "%llu", (long long unsigned int)q);
  //ulltoa2(str+len-len_q, q);

}
/*-------------------------------------------------------------------------*/
void ntnum_mkhash(uchar_t *pwd, uchar_t *hash) {
  make_nthash((char*)pwd, (char*)hash);
}
/*-------------------------------------------------------------------------*/
uint32_t ntnum_bin11(uchar_t *input, int length) {

  uint32_t sum=0;
  int i = 0;

  for (i=0; i<length; i++) 
    sum = sum*11 + (input[i] ? input[i]-0x2f : 0);
  return sum;
}
