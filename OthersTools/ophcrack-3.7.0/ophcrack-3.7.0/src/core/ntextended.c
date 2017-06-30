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

#include "ntextended.h"
#include "table.h"
#include "hash.h"
#include "ophel.h"
#include "original.h"
#include "lmtable.h"

static uint64_t ntextended_sizes[10] = { 4, 108118236, 672425284, 1344850568, 672442590, 1344885180, 672411252, 1344822504, 672417770, 1344835540};
/*-------------------------------------------------------------------------*/
int ntextended_setup(void *tbl_) {
  table_t *tbl = tbl_;

  tbl->ncols  = 24320;
  tbl->offset = 50000;
  tbl->sizes  = ntextended_sizes;

  tbl->find    = ntextended_find;
  tbl->check   = ntextended_check;
  tbl->isvalid = ntextended_isvalid;

  tbl->lookup_idx = lmtable_lookup_idx;
  tbl->lookup_end = lmtable_lookup_end;
  tbl->lookup_srt = ntextended_lookup_srt;

  return 1;
}
/*-------------------------------------------------------------------------*/
void ntextended_find(void *hsh_, void *tbl_, void *el_) {
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
  
  ntextended_mkredux(tbl, hash, pwd, n_redux);

  for (c=1; c<ncols-col; ++c) {
    ++n_redux;

    ntextended_mkhash(pwd, tmp);
    ntextended_mkredux(tbl, tmp, pwd, n_redux);
    ++stat->hredux;
  }

  /* Compute the prefix and postfix. */
  
  uint64_t pw_bin  = ntextended_bin43(pwd);
  uint32_t prefix  = (uint32_t)(pw_bin >> 18 & 0x00000000FFFFFFFF);
  uint16_t postfix = (uint16_t)(pw_bin & 0x000000000000FFFF);
  
  el->prefix  = prefix;
  el->postfix = postfix;
}
/*-------------------------------------------------------------------------*/
int ntextended_lookup_srt(void *hsh_, void *tbl_, void *el_) {
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

  ntextended_unbin95(start, pwd);

  return 1;
}
/*-------------------------------------------------------------------------*/
int ntextended_check(void *hsh_, void *tbl_, void *el_) {
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
    ntextended_mkhash(pwd, tmp);
    ntextended_mkredux(tbl, tmp, pwd, n_redux);

    ++n_redux;
    ++stat->hredux;
    ++stat->falarm_hredux;
  }  

  /* Check if we got the same hash. */

  ntextended_mkhash(pwd, tmp);
  
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
int ntextended_isvalid(void *hsh_, void *tbl_) {
  hash_t *hsh = hsh_;

  return hsh->kind == nt ? 1 : 0;
}
/*-------------------------------------------------------------------------*/
void ntextended_mkredux(table_t *tbl, uchar_t *hash, uchar_t *pwd, 
			int n_redux) {

  /* Convert the hash to four unsigned 32-bit integers. */
  
  uint32_t idx[4];
  
  idx[0] = ftohl(*(uint32_t*)hash) ^ n_redux;
  idx[1] = ftohl(*(uint32_t*)(hash+4));
  idx[2] = ftohl(*(uint32_t*)(hash+8));
  idx[3] = ftohl(*(uint32_t*)(hash+12));
    
  int n = idx[3];
  int i;

  /* Compute the password */

  pwd[8] = 0;

  if (n > ntextended_max[8]) {  // 36 chars
    for (i=0; i<3; ++i) {
      pwd[i]  = ntextended_low36[idx[0] % 36];
      idx[0] /= 36;
    }

    for (i=3; i<6; ++i) {
      pwd[i]  = ntextended_low36[idx[1] % 36];
      idx[1] /= 36;
    }

    for (i=6; i<8; ++i) {
      pwd[i]  = ntextended_low36[idx[2] % 36];
      idx[2] /= 36;
    }
  } else if (n > ntextended_max[7]) {  // 62 chars
    for (i=0; i<3; ++i) {
      pwd[i]  = ntextended_alphanum62[idx[0] % 62];
      idx[0] /= 62;
    }

    for (i=3; i<7; ++i) {
      pwd[i]  = ntextended_alphanum62[idx[1] % 62];
      idx[1] /= 62;
    }

    pwd[7] = 0;
  } else {  // 95 chars
    for (i=0; i<3; ++i) {
      pwd[i]  = ntextended_ext95[idx[0] % 95];
      idx[0] /= 95;
    }

    for (i=3; i<6; ++i) {
      pwd[i]  = ntextended_ext95[idx[1] % 95];
      idx[1] /= 95;
    }

    /* Truncate the password. */

    pwd[7] = 0;
    pwd[6] = 0;

    if (n < ntextended_max[6]) {
      pwd[5]=0; 
      if (n < ntextended_max[5]) {
	pwd[4]=0; 
	if (n < ntextended_max[4]) {
	  pwd[3]=0; 
	  if (n < ntextended_max[3]) {
	    pwd[2]=0; 
	    if (n < ntextended_max[2]) { 
	      pwd[1]=0;	
	    }
	  }
	}
      }
    }
  }
}
/*-------------------------------------------------------------------------*/
void ntextended_mkhash(uchar_t *pwd, uchar_t *hash) {
  make_nthash((char*)pwd, (char*)hash);
}
/*-------------------------------------------------------------------------*/
uint64_t ntextended_bin43(uchar_t *input) {
  int i;
  int l = (int)strlen((char*)input);
  uint64_t sum = 0;

  if (l<7)
    for (i=0; i<l && i<6; i++) {
      char *x1 = strchr((char*)ntextended_ext95, input[i]);
      char *x2 = (char*)ntextended_ext95;

      sum = sum*95 + (uint64_t)(x1-x2);
    }  

  else if (l==7)
    for (i=0; i<7; i++) {
      char *x1 = strchr((char*)ntextended_alphanum62, input[i]);
      char *x2 = (char*)ntextended_alphanum62;

      sum = sum*62 + (uint64_t)(x1-x2);
    }  

  else if (l==8)
    for (i=0; i<8; i++) {
      char *x1 = strchr((char*)ntextended_low36, input[i]);
      char *x2 = (char*)ntextended_low36;

      sum = sum*36 + (uint64_t)(x1-x2);
    }  

  sum += ntextended_offset[l-1];
  
  return sum;
}
/*-------------------------------------------------------------------------*/
void ntextended_unbin95(uint32_t input, uchar_t *output) {
  int i = 0;

  for (i=4; i>=0; i--) {
    output[i] = ntextended_ext95[input % 95];
    input /= 95;
  }
}
