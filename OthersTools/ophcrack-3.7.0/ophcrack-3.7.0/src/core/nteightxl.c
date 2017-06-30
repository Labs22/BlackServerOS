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
#include <assert.h>

#include "nteightxl.h"
#include "table.h"
#include "hash.h"
#include "ophel.h"
#include "original.h"
#include "lmtable.h"
/*-------------------------------------------------------------------------*/
int nteightxl_setup(void *tbl_) {
  table_t *tbl = tbl_;

  tbl->ncols  = 100000;
  tbl->offset = 1000000;
  tbl->sizes  = nteightxl_sizes;

  tbl->find    = nteightxl_find;
  tbl->check   = nteightxl_check;
  tbl->isvalid = nteightxl_isvalid;

  tbl->lookup_idx = nteightxl_lookup_idx;
  tbl->lookup_end = nteightxl_lookup_end;
  tbl->lookup_srt = nteightxl_lookup_srt;

  return 1;
}
/*-------------------------------------------------------------------------*/
void nteightxl_find(void *hsh_, void *tbl_, void *el_) {
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
  
  nteightxl_mkredux(tbl, hash, pwd, n_redux);

  for (c=1; c<ncols-col; ++c) {
    ++n_redux;

    nteightxl_mkhash(pwd, tmp);
    nteightxl_mkredux(tbl, tmp, pwd, n_redux);
    ++stat->hredux;
  }

  /* Compute the prefix and postfix. */
  
  uint64_t binary  = nteightxl_bin95(pwd, 8);
  uint32_t prefix  = (uint32_t)(binary >> 23);
  uint16_t postfix = (uint16_t)(binary & 0xffff);
  
  el->prefix  = prefix;
  el->postfix = postfix;
}
/*-------------------------------------------------------------------------*/
int nteightxl_lookup_idx(void *hsh_, void *tbl_, void *el_) {
  table_t *tbl = tbl_;
  ophel_t *el  = el_;
  
  uint32_t prefix = el->prefix;

  ophstat_t *stat = el->stat;
  ++stat->prefix;

  el->low  = 0;
  el->high = 0;
    
  /* If the .index file has been preloaded, then we look into the memory. */

  if (tbl->idxmem) {
    uchar_t *mem = (uchar_t*)(tbl->idxmem + 5*prefix);

    memcpy(&(el->low), mem, 5);
    memcpy(&(el->high), mem + 5, 5);
  }

  /* Otherwise, we access the disk. */

  else {
    FILE *idxfile = tbl->idxfile;

    fseeko(idxfile, 5*prefix, SEEK_SET);
    fread(&el->low, 5, 1, idxfile);
    fread(&el->high, 5, 1, idxfile);
    ++stat->fseek_idx;
  }

  /* Make sure that the bytes are in the correct order. */

  el->low  = ftohl64(el->low);
  el->high = ftohl64(el->high);

  assert(el->high >= el->low);

  return 1;
}
/*-------------------------------------------------------------------------*/
int nteightxl_lookup_end(void *hsh_, void *tbl_, void *el_) {
  table_t *tbl = tbl_;
  ophel_t *el  = el_;
  ophstat_t *stat = el->stat;

  uint16_t postfix = el->postfix;
  uint64_t low  = el->low;
  uint64_t high = el->high;
  uint64_t n    = high - low;
  uint64_t i;
  uint64_t offset = 2*low;

  /* If the .bin file has been preloaded, then we look into the memory. */

  if (tbl->endmem) {
    uint16_t *mem = (uint16_t*)(tbl->endmem + offset);

    for (i=0; i<n; ++i)
      if (ftohs(mem[i]) == postfix) break;

  }

  /* Otherwise, we access the disk. */

  else {
    uint16_t pfix[512];
    FILE *endfile = tbl->endfile;

    fseeko(endfile, offset, SEEK_SET);
    fread(pfix, sizeof(uint16_t), n, endfile);
    ++stat->fseek_end;

    for (i=0; i<n; ++i)
      if (ftohs(pfix[i]) == postfix) break;

  }

  el->offset =  5 * (low+i);


  if (i < n) ++stat->postfix;

  return i == n ? 0 : 1;
}
/*-------------------------------------------------------------------------*/
int nteightxl_lookup_srt(void *hsh_, void *tbl_, void *el_) {
  table_t *tbl = tbl_;
  ophel_t *el  = el_;

  uint64_t offset = el->offset;
  uint64_t start;

  ophstat_t *stat = el->stat;
  ++stat->start;

  start = 0;

  /* If the .start file has been preloaded, then we look into the memory. */

  if (tbl->srtmem) {
    uchar_t *mem = (uchar_t*)(tbl->srtmem + offset);

    memcpy(&start, mem, 5);
  }

  /* Otherwise, we access the disk. */

  else {
    FILE *srtfile = tbl->srtfile;

    fseeko(srtfile, offset, SEEK_SET);
    fread(&start, 5, 1, srtfile);
    ++stat->fseek_srt;
  }

  /* Make sure that start is in the correct byte order. */

  start = ftohl64(start);

  /* Convert it to a valid password. */

  uchar_t *pwd = (uchar_t*)el->pwd;

  pwd[6] = 0;
  pwd[7] = 0;
  pwd[8] = 0;
  pwd[9] = 0;

  nteightxl_unbin95(start, pwd, 6);

  return 1;
}
/*-------------------------------------------------------------------------*/
int nteightxl_check(void *hsh_, void *tbl_, void *el_) {
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
    nteightxl_mkhash(pwd, tmp);
    nteightxl_mkredux(tbl, tmp, pwd, n_redux);

    ++n_redux;
    ++stat->hredux;
    ++stat->falarm_hredux;
  }  

  /* Check if we got the same hash. */

  nteightxl_mkhash(pwd, tmp);
  
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
int nteightxl_isvalid(void *hsh_, void *tbl_) {
  hash_t *hsh = hsh_;

  return hsh->kind == nt ? 1 : 0;
}
/*-------------------------------------------------------------------------*/
void nteightxl_mkredux(table_t *tbl, uchar_t *hash, uchar_t *pwd, 
			int n_redux) {

  /* Convert the hash to four unsigned 32-bit integers. */
  
  uint32_t idx[4];
  
  idx[0] = ftohl(*(uint32_t*)hash) ^ n_redux;
  idx[1] = ftohl(*(uint32_t*)(hash+4));
  idx[2] = ftohl(*(uint32_t*)(hash+8));
  idx[3] = ftohl(*(uint32_t*)(hash+12));
    
  /* Compute the password */

  idx[0] %= 9025;
  idx[1] %= 9025;
  idx[2] %= 9025;
  idx[3] %= 9025;

  pwd[0] = nteightxl_ext95[idx[0]/95];
  pwd[1] = nteightxl_ext95[idx[0]%95];
  pwd[2] = nteightxl_ext95[idx[1]/95];
  pwd[3] = nteightxl_ext95[idx[1]%95];
  pwd[4] = nteightxl_ext95[idx[2]/95];
  pwd[5] = nteightxl_ext95[idx[2]%95];
  pwd[6] = nteightxl_ext95[idx[3]/95];
  pwd[7] = nteightxl_ext95[idx[3]%95];
  pwd[8] = 0;

}
/*-------------------------------------------------------------------------*/
void nteightxl_mkhash(uchar_t *pwd, uchar_t *hash) {
  make_nthash((char*)pwd, (char*)hash);
}
/*-------------------------------------------------------------------------*/
uint64_t nteightxl_bin95(uchar_t *input, int length) {
  
  uint64_t sum=0;
  int i = 0;

  for (i=0; i<length; i++)  {
    sum = sum*95 + (uint64_t) (strchr((char*)nteightxl_ext95,input[i]) - (char*)nteightxl_ext95); 
  }
  return sum;
}
/*-------------------------------------------------------------------------*/
int nteightxl_unbin95(uint64_t input, uchar_t *output, int length) {

  int i = 0;

  for (i=0; i<length; i++) {
    output[i]=nteightxl_ext95[input%95];
    input/=95;
  }
  return 1;
}
