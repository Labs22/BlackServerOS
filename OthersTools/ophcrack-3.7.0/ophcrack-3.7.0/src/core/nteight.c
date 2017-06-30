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

#include "nteight.h"
#include "table.h"
#include "hash.h"
#include "ophel.h"
#include "original.h"
#include "lmtable.h"

static uint64_t nteight_sizes[18] = { 8, 67108864, 6000043318LLU, 12000086636LLU, 6000073034LLU, 12000146068LLU, 6000106106LLU, 12000212212LLU, 6000192456LLU, 12000384912LLU, 6000173996LLU, 12000347992LLU, 6000174818LLU, 12000349636LLU, 6000123556LLU, 12000247112LLU, 6000124550LLU, 12000249100LLU};
/*-------------------------------------------------------------------------*/
int nteight_setup(void *tbl_) {
  table_t *tbl = tbl_;

  tbl->ncols  = 55000;
  tbl->offset = 100000;
  tbl->sizes  = nteight_sizes;

  tbl->find    = nteight_find;
  tbl->check   = nteight_check;
  tbl->isvalid = nteight_isvalid;

  tbl->lookup_idx = lmtable_lookup_idx;
  tbl->lookup_end = lmtable_lookup_end;
  tbl->lookup_srt = nteight_lookup_srt;

  return 1;
}
/*-------------------------------------------------------------------------*/
void nteight_find(void *hsh_, void *tbl_, void *el_) {
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
  
  nteight_mkredux(tbl, hash, pwd, n_redux);

  for (c=1; c<ncols-col; ++c) {
    ++n_redux;

    nteight_mkhash(pwd, tmp);
    nteight_mkredux(tbl, tmp, pwd, n_redux);
    ++stat->hredux;
  }

  /* Compute the prefix and postfix. */
  
  uint32_t prefix  = nteight_bin64(pwd, 4);
  uint16_t postfix = (uint16_t)nteight_bin64(pwd+4, 4);
  
  el->prefix  = prefix;
  el->postfix = postfix;
}
/*-------------------------------------------------------------------------*/
int nteight_lookup_srt(void *hsh_, void *tbl_, void *el_) {
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

  pwd[6] = 0;
  pwd[7] = 0;
  pwd[8] = 0;
  pwd[9] = 0;

  nteight_unbin64(start, pwd, 6);

  return 1;
}
/*-------------------------------------------------------------------------*/
int nteight_check(void *hsh_, void *tbl_, void *el_) {
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
    nteight_mkhash(pwd, tmp);
    nteight_mkredux(tbl, tmp, pwd, n_redux);

    ++n_redux;
    ++stat->hredux;
    ++stat->falarm_hredux;
  }  

  /* Check if we got the same hash. */

  nteight_mkhash(pwd, tmp);
  
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
int nteight_isvalid(void *hsh_, void *tbl_) {
  hash_t *hsh = hsh_;

  return hsh->kind == nt ? 1 : 0;
}
/*-------------------------------------------------------------------------*/
void nteight_mkredux(table_t *tbl, uchar_t *hash, uchar_t *pwd, 
			int n_redux) {

  /* Convert the hash to four unsigned 32-bit integers. */
  
  uint32_t idx[8];
  uint32_t un_redux = (uint32_t) n_redux;
  
  idx[0] = ftohl(*(uint32_t*)hash) ^ un_redux;
  idx[2] = ftohl(*(uint32_t*)(hash+4)) ^ (un_redux >> 12);
  idx[4] = ftohl(*(uint32_t*)(hash+8)) ^ (un_redux >> 24);
  idx[6] = ftohl(*(uint32_t*)(hash+12));
    
  /* Compute the password */

  idx[1] = idx[0] & 0x3f;
  idx[0] = (idx[0] >> 6) & 0x3f;
  idx[3] = idx[2] & 0x3f;
  idx[2] = (idx[2] >> 6) & 0x3f;
  idx[5] = idx[4] & 0x3f;
  idx[4] = (idx[4] >> 6) & 0x3f;
  idx[7] = idx[6] & 0x3f;
  idx[6] = (idx[6] >> 6) & 0x3f;

  pwd[0] = nteight_alphanum64[idx[0]];
  pwd[1] = nteight_alphanum64[idx[1]];
  pwd[2] = nteight_alphanum64[idx[2]];
  pwd[3] = nteight_alphanum64[idx[3]];
  pwd[4] = nteight_alphanum64[idx[4]];
  pwd[5] = nteight_alphanum64[idx[5]];
  pwd[6] = nteight_alphanum64[idx[6]];
  pwd[7] = nteight_alphanum64[idx[7]];
  pwd[8] = 0;

}
/*-------------------------------------------------------------------------*/
void nteight_mkhash(uchar_t *pwd, uchar_t *hash) {
  make_nthash((char*)pwd, (char*)hash);
}
/*-------------------------------------------------------------------------*/
uint32_t nteight_bin64(uchar_t *input, int length) {
  
  uint32_t sum=0;
  int i = 0;

  for (i=0; i<length; i++)  {
    sum = sum*64 + (uint64_t) (strchr((char*)nteight_alphanum64,input[i]) - (char*)nteight_alphanum64); 
  }
  return sum;
}
/*-------------------------------------------------------------------------*/
int nteight_unbin64(uint32_t input, uchar_t *output, int length) {

  int i = 0;
  /* start binarized in inverse order ! */

  for (i=0; i<length; i++) {
    output[i]=nteight_alphanum64[input%64];
    input/=64;
  }
  return 1;
}
