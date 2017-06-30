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

#include "ntspecialxl.h"
#include "table.h"
#include "hash.h"
#include "ophel.h"
#include "original.h"
#include "lmtable.h"

static uint64_t ntspecialxl_sizes[10] = { 4, 269228524ULL, 8125947190ULL, 20314867975ULL, 8125917004ULL, 20314792510ULL, 8126010972ULL, 20315027430ULL, 8125860724ULL, 20314651810ULL};
/*-------------------------------------------------------------------------*/
int ntspecialxl_setup(void *tbl_) {
  table_t *tbl = tbl_;

  tbl->ncols  = 30000;
  tbl->offset = 100000;
  tbl->sizes  = ntspecialxl_sizes;

  tbl->find    = ntspecialxl_find;
  tbl->check   = ntspecialxl_check;
  tbl->isvalid = ntspecialxl_isvalid;

  tbl->lookup_idx = lmtable_lookup_idx;
  tbl->lookup_end = ntspecialxl_lookup_end;
  tbl->lookup_srt = ntspecialxl_lookup_srt;

  return 1;
}
/*-------------------------------------------------------------------------*/
void ntspecialxl_find(void *hsh_, void *tbl_, void *el_) {
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
  
  ntspecialxl_mkredux(tbl, hash, pwd, n_redux);

  for (c=1; c<ncols-col; ++c) {
    ++n_redux;

    ntspecialxl_mkhash(pwd, tmp);
    ntspecialxl_mkredux(tbl, tmp, pwd, n_redux);
    ++stat->hredux;
  }

  /* Compute the prefix and postfix. */
  
  uint64_t binary  = ntspecialxl_bin95(pwd, 7);
  uint32_t prefix  = (uint32_t)(binary >> 20);
  uint16_t postfix = (uint16_t)(binary & 0xffff);
  
  el->prefix  = prefix;
  el->postfix = postfix;
}
/*-------------------------------------------------------------------------*/
int ntspecialxl_lookup_end(void *hsh_, void *tbl_, void *el_) {
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
int ntspecialxl_lookup_srt(void *hsh_, void *tbl_, void *el_) {
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

  ntspecialxl_unbin95(start, pwd, 6);

  return 1;
}
/*-------------------------------------------------------------------------*/
int ntspecialxl_check(void *hsh_, void *tbl_, void *el_) {
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
    ntspecialxl_mkhash(pwd, tmp);
    ntspecialxl_mkredux(tbl, tmp, pwd, n_redux);

    ++n_redux;
    ++stat->hredux;
    ++stat->falarm_hredux;
  }  

  /* Check if we got the same hash. */

  ntspecialxl_mkhash(pwd, tmp);
  
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
int ntspecialxl_isvalid(void *hsh_, void *tbl_) {
  hash_t *hsh = hsh_;

  return hsh->kind == nt ? 1 : 0;
}
/*-------------------------------------------------------------------------*/
void ntspecialxl_mkredux(table_t *tbl, uchar_t *hash, uchar_t *pwd, 
			int n_redux) {

  /* Convert the hash to two unsigned 32-bit and one
     unsigned 64-bit integers. */
  
  uint32_t idx[4];
  uint32_t a,b;
  uint64_t limit;
  
  idx[0] = ftohl(*(uint32_t*)hash) ^ n_redux;
  idx[1] = ftohl(*(uint32_t*)(hash+4));
  idx[2] = ftohl(*(uint32_t*)(hash+8));
  idx[3] = ftohl(*(uint32_t*)(hash+12));
  limit = ((uint64_t) idx[2] << 32) | idx[3];

  /* Compute the password */
  if (limit < ntspecialxl_max[7]) {
    a = idx[0] / 9025;
    b = idx[1] / 9025;
    a %= 9025;
    b %= 9025;
    idx[0] %= 9025;
    idx[1] %= 9025;

    pwd[0] = ntspecialxl_ext95[idx[0]/95];
    pwd[1] = ntspecialxl_ext95[idx[0]%95];
    pwd[2] = ntspecialxl_ext95[a/95];
    pwd[3] = ntspecialxl_ext95[a%95];
    pwd[4] = ntspecialxl_ext95[idx[1]/95];
    pwd[5] = ntspecialxl_ext95[idx[1]%95];
    pwd[6] = ntspecialxl_ext95[b/95];
    pwd[7] = 0;
  } else if (limit < ntspecialxl_max[6]) {
    a = idx[0] / 9025;
    a %= 9025;
    idx[0] %= 9025;
    idx[1] %= 9025;

    pwd[0] = ntspecialxl_ext95[idx[0]/95];
    pwd[1] = ntspecialxl_ext95[idx[0]%95];
    pwd[2] = ntspecialxl_ext95[a/95];
    pwd[3] = ntspecialxl_ext95[a%95];
    pwd[4] = ntspecialxl_ext95[idx[1]/95];
    pwd[5] = ntspecialxl_ext95[idx[1]%95];
    pwd[6] = 0;
    pwd[7] = 0;
  } else   if (limit < ntspecialxl_max[5]) {
    a = idx[0] / 9025;
    a %= 9025;
    idx[0] %= 9025;
    idx[1] %= 9025;

    pwd[0] = ntspecialxl_ext95[idx[0]/95];
    pwd[1] = ntspecialxl_ext95[idx[0]%95];
    pwd[2] = ntspecialxl_ext95[a/95];
    pwd[3] = ntspecialxl_ext95[a%95];
    pwd[4] = ntspecialxl_ext95[idx[1]/95];
    pwd[5] = 0;
    pwd[6] = 0;
    pwd[7] = 0;
  } else   if (limit < ntspecialxl_max[4]) {
    a = idx[0] / 9025;
    a %= 9025;
    idx[0] %= 9025;

    pwd[0] = ntspecialxl_ext95[idx[0]/95];
    pwd[1] = ntspecialxl_ext95[idx[0]%95];
    pwd[2] = ntspecialxl_ext95[a/95];
    pwd[3] = ntspecialxl_ext95[a%95];
    pwd[4] = 0;
    pwd[5] = 0;
    pwd[6] = 0;
    pwd[7] = 0;
  } else   if (limit < ntspecialxl_max[3]) {
    a = idx[0] / 9025;
    a %= 9025;
    idx[0] %= 9025;

    pwd[0] = ntspecialxl_ext95[idx[0]/95];
    pwd[1] = ntspecialxl_ext95[idx[0]%95];
    pwd[2] = ntspecialxl_ext95[a/95];
    pwd[3] = 0;
    pwd[4] = 0;
    pwd[5] = 0;
    pwd[6] = 0;
    pwd[7] = 0;
  } else   if (limit < ntspecialxl_max[2]) {
    idx[0] %= 9025;

    pwd[0] = ntspecialxl_ext95[idx[0]/95];
    pwd[1] = ntspecialxl_ext95[idx[0]%95];
    pwd[2] = 0;
    pwd[3] = 0;
    pwd[4] = 0;
    pwd[5] = 0;
    pwd[6] = 0;
    pwd[7] = 0;
  } else {
    idx[0] %= 9025;

    pwd[0] = ntspecialxl_ext95[idx[0]/95];
    pwd[1] = 0;
    pwd[2] = 0;
    pwd[3] = 0;
    pwd[4] = 0;
    pwd[5] = 0;
    pwd[6] = 0;
    pwd[7] = 0;
  }
}
/*-------------------------------------------------------------------------*/
void ntspecialxl_mkhash(uchar_t *pwd, uchar_t *hash) {
  make_nthash((char*)pwd, (char*)hash);
}
/*-------------------------------------------------------------------------*/
uint64_t ntspecialxl_bin95(uchar_t *input, int length) {
  
  uint64_t sum=0;
  int i = 0;

  for (i=0; i<length && input[i]; i++)  {
    sum = sum*95 + (uint64_t) (strchr((char*)ntspecialxl_ext95,input[i]) - (char*)ntspecialxl_ext95); 
  }
  sum += ntspecialxl_offset[i-1];
  return sum;
}
/*-------------------------------------------------------------------------*/
int ntspecialxl_unbin95(uint64_t input, uchar_t *output, int length) {

  int i = 0;

  for (i=0; i<length; i++) {
    output[i]=ntspecialxl_ext95[input%95];
    input/=95;
  }
  return 1;
}
