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
#include <string.h>
#include <ctype.h>

#include "ntseven.h"
#include "table.h"
#include "hash.h"
#include "ophel.h"
#include "original.h"
#include "lmtable.h"
/*-------------------------------------------------------------------------*/
int ntseven_setup(void *tbl_) {
  table_t *tbl = tbl_;

  tbl->ncols  = 50000;
  tbl->offset = 100000;
  tbl->sizes  = ntseven_sizes;

  tbl->find    = ntseven_find;
  tbl->check   = ntseven_check;
  tbl->isvalid = ntseven_isvalid;

  tbl->lookup_idx = lmtable_lookup_idx;
  tbl->lookup_end = lmtable_lookup_end;
  tbl->lookup_srt = ntseven_lookup_srt;

  return 1;
}
/*-------------------------------------------------------------------------*/
void ntseven_find(void *hsh_, void *tbl_, void *el_) {
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
  
  ntseven_mkredux(tbl, hash, pwd, n_redux);

  for (c=1; c<ncols-col; ++c) {
    ++n_redux;

    ntseven_mkhash(pwd, tmp);
    ntseven_mkredux(tbl, tmp, pwd, n_redux);
    ++stat->hredux;
  }

  /* Compute the prefix and postfix. */
  
  uint64_t binary  = ntseven_bin95(pwd, 7);
  uint32_t prefix  = (uint32_t)(binary >> 23);
  uint16_t postfix = (uint16_t)(binary & 0xffff);
  
  el->prefix  = prefix;
  el->postfix = postfix;
}
/*-------------------------------------------------------------------------*/
int ntseven_lookup_srt(void *hsh_, void *tbl_, void *el_) {
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

  pwd[5] = 0;
  pwd[6] = 0;
  pwd[7] = 0;
  pwd[8] = 0;

  ntseven_unbin95(start, pwd, 5);

  return 1;
}
/*-------------------------------------------------------------------------*/
int ntseven_check(void *hsh_, void *tbl_, void *el_) {
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
    ntseven_mkhash(pwd, tmp);
    ntseven_mkredux(tbl, tmp, pwd, n_redux);

    ++n_redux;
    ++stat->hredux;
    ++stat->falarm_hredux;
  }  

  /* Check if we got the same hash. */

  ntseven_mkhash(pwd, tmp);
  
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
int ntseven_isvalid(void *hsh_, void *tbl_) {
  hash_t *hsh = hsh_;

  return hsh->kind == nt ? 1 : 0;
}
/*-------------------------------------------------------------------------*/
void ntseven_mkredux(table_t *tbl, uchar_t *hash, uchar_t *pwd, 
			int n_redux) {

  /* Convert the hash to four unsigned 32-bit integers. */
  
  uint32_t idx[4];
  
  idx[0] = ftohl(*(uint32_t*)hash) ^ n_redux;
  idx[1] = ftohl(*(uint32_t*)(hash+4));
  idx[2] = ftohl(*(uint32_t*)(hash+8));
  idx[3] = ftohl(*(uint32_t*)(hash+12));
    
  /* Compute the password */

  idx[0] %= power2_95;
  idx[1] %= power2_95;
  idx[2] %= power2_95;
  idx[3] %= power2_95;

  pwd[0] = ntseven_ext95[idx[0]/95];
  pwd[1] = ntseven_ext95[idx[0]%95];
  pwd[2] = ntseven_ext95[idx[1]/95];
  pwd[3] = ntseven_ext95[idx[1]%95];
  pwd[4] = ntseven_ext95[idx[2]/95];
  pwd[5] = ntseven_ext95[idx[2]%95];
  pwd[6] = ntseven_ext95[idx[3]/95];

  /* Finish the password. */

  pwd[7] = 0;

}
/*-------------------------------------------------------------------------*/
void ntseven_mkhash(uchar_t *pwd, uchar_t *hash) {
  make_nthash((char*)pwd, (char*)hash);
}
/*-------------------------------------------------------------------------*/
uint64_t ntseven_bin95(uchar_t *input, int length) {
  
  uint64_t sum=0;
  int i = 0;

  for (i=0; i<length; i++)  {
    sum = sum*95 + (uint64_t) (strchr((char*)ntseven_ext95,input[i]) - (char*)ntseven_ext95);
  }
  return sum;
}
/*-------------------------------------------------------------------------*/
int ntseven_unbin95(uint32_t input, uchar_t *output, int length) {

  int i = 0;

  for (i=length-1; i>=0; i--) {
    output[i]=ntseven_ext95[input%95];
    input/=95;
  }
  return 1;
}
