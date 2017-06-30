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
#include <stdint.h>
#include <string.h>

#include "lmgerman.h"
#include "lmtable.h"
#include "hash.h"
#include "ophel.h"
#include "original.h"
#include "misc.h"

static uint64_t lmgerman_sizes[10] = { 4, 170911176, 599772172, 1199544344, 599747104, 1199494208, 599734104, 1199468208, 599782562, 1199565124};
/*-------------------------------------------------------------------------*/
int lmgerman_setup(void *tbl_) {
  table_t *tbl = tbl_;

  tbl->ncols  = 14000;
  tbl->offset = 14000;
  tbl->sizes  = lmgerman_sizes;

  tbl->find    = lmgerman_find;
  tbl->check   = lmgerman_check;
  tbl->isvalid = lmgerman_isvalid;

  tbl->lookup_idx = lmtable_lookup_idx;
  tbl->lookup_end = lmtable_lookup_end;
  tbl->lookup_srt = lmgerman_lookup_srt;

  return 1;
}
/*-------------------------------------------------------------------------*/
void lmgerman_find(void *hsh_, void *tbl_, void *el_) {
  hash_t  *hsh  = hsh_;
  table_t *tbl  = tbl_;
  ophel_t *el   = el_;
  uchar_t *hash = hsh->hash;;
  uchar_t *pwd  = (uchar_t*)el->pwd;
  ophstat_t *stat = el->stat;
  
  int c;
  int col      = el->col;
  int ncols    = tbl->ncols;
  int n_redux  = col + tbl->idx * tbl->offset;
  
  uchar_t tmp[8];
  
  /* Hash and redux until the last column. */
  
  lmgerman_mkredux(tbl, hash, pwd, n_redux);
  
  for (c=1; c<ncols-col; ++c) {
    ++n_redux;

    lmtable_mkhash(pwd, tmp);
    lmgerman_mkredux(tbl, tmp, pwd, n_redux);
    ++stat->hredux;
  }
  
  /* Compute the prefix and postfix. */

  uint64_t pw_bin  = lmgerman_bin73(pwd);
  uint32_t prefix  = (uint32_t)(pw_bin >> 18 & 0x00000000FFFFFFFF);
  uint16_t postfix = (uint16_t)(pw_bin & 0x000000000000FFFF);

  el->prefix  = prefix;
  el->postfix = postfix;
}
/*-------------------------------------------------------------------------*/
int lmgerman_lookup_srt(void *hsh_, void *tbl_, void *el_) {
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

  lmgerman_unbin36(start, pwd);

  return 1;
}
/*-------------------------------------------------------------------------*/
int lmgerman_check(void *hsh_, void *tbl_, void *el_) {
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

  uchar_t tmp[8];

  /* Hash and redux until the starting column. */
  
  stat->falarm_hredux = 0;

  for (c=ncols-col; c<ncols; ++c) {
    lmtable_mkhash(pwd, tmp);
    lmgerman_mkredux(tbl, tmp, pwd, n_redux);

    ++n_redux;
    ++stat->hredux;
    ++stat->falarm_hredux;
  }  

  /* Check if we got the same hash. */

  lmtable_mkhash(pwd, tmp);
  wincp1252_to_ascii(pwd);
  
  if (memcmp(tmp, hash, 8) == 0) {
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
int lmgerman_isvalid(void *hsh_, void *tbl_) {
  hash_t *hsh  = hsh_;

  return (hsh->kind == lm1 || hsh->kind == lm2) ? 1 : 0;
}
/*-------------------------------------------------------------------------*/
void lmgerman_mkredux(table_t *tbl, uchar_t *hash, uchar_t *pwd, 
		      int n_redux) {

  const uchar_t *charset = extended_chars;
  int chars_size = sizeof(extended_chars);
  int lmgerman_chars_size = 4;

  /* Convert the hash to two unsigned 32-bit integers. */

  uint32_t idx[2];
  uint32_t int_hash[2];

  int_hash[0] = ftohl(*(uint32_t*)hash);
  int_hash[1] = ftohl(*(uint32_t*)(hash+4)); 

  /* XOR the 1st integer with n_redux */

  idx[0] = int_hash[0] ^ n_redux;
  idx[1] = int_hash[1];

  /* Generate the password. */

  int n = idx[1];
  uchar_t len, k, x=0, rank, i, j=0;
  uchar_t chars1[8], chars2[8];

  if (n <= lmgerman_spec_max[0].max)
    x=0;
  else
    if (n <= lmgerman_spec_max[1].max)
      x=1;
    else
      if (n <= lmgerman_spec_max[2].max)
	x=2;
      else
	for (i=3; i<28; i++)
	  if (n <= lmgerman_spec_max[i].max) {
	    x=i;
	    break;
	  }

  len = lmgerman_spec_max[x].n; 
  k   = lmgerman_spec_max[x].k;
  
  for (i=0; i<4 || i<(len-k); i++) {
    chars1[i] = charset[idx[0] % chars_size]; 
    idx[0] /= chars_size;
  }
 
  for (i=4; i<(len-k); i++) {
    chars1[i] = charset[idx[1] % chars_size]; 
    idx[1] /= chars_size;
  } 
  
  rank = idx[1] % lmgerman_choose[len][k]; 

  if (tbl->kind == lmgermanv2)
    idx[1] /= lmgerman_choose[len][k];
  
  for (i=0; i<k; i++) {
    chars2[i] = lmgerman_chars[idx[1] % lmgerman_chars_size]; 
    idx[1] /= lmgerman_chars_size;
  };
  
  for (i=0,j=0,k=0; i<len; i++)
    if (my_getbit(lmgerman_ranks[x][rank],i)) {
      pwd[i] = chars2[k];
      k++;
    } else {
      pwd[i] = chars1[j];
      j++;
    }

  for (i=len; i<7; i++) pwd[i]=0;
}
/*-------------------------------------------------------------------------*/
uint64_t lmgerman_bin73(uchar_t *input) {
  int i = 0;
  uint64_t sum = 0;
  
  for (i=0; i<7&&input[i]; i++) {
    char *x1 = (char*)memchr(lmgerman_chars, input[i], 73);
    char *x2 = (char*)lmgerman_chars;

    sum = sum*73 + (uint64_t)(x1-x2);
  }

  return sum;
}
/*-------------------------------------------------------------------------*/
void lmgerman_unbin36(uint32_t input, uchar_t *output) {
  int i = 0;

  for (i=5; i>=0; i--) {
    output[i] = lmgerman_chars[input % 36 + 4];
    input /= 36;
  }
}
