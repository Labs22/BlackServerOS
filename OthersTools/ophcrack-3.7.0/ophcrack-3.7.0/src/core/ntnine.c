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

#include "ntnine.h"
#include "table.h"
#include "hash.h"
#include "ophel.h"
#include "original.h"
#include "lmtable.h"

static uint64_t ntnine_sizes[14] = { 6, 241864704, 2963172506, 5926345012LLU, 2963141176, 5926282352LLU, 2963198436, 5926396872LLU, 2963189836, 5926379672LLU, 2963140476, 5926280952LLU, 2963169516, 5926339032LLU};
/*-------------------------------------------------------------------------*/
int ntnine_setup(void *tbl_) {
  table_t *tbl = tbl_;

  tbl->ncols  = 45000;
  tbl->offset = 50000;
  tbl->sizes  = ntnine_sizes;

  tbl->find    = ntnine_find;
  tbl->check   = ntnine_check;
  tbl->isvalid = ntnine_isvalid;

  tbl->lookup_idx = lmtable_lookup_idx;
  tbl->lookup_end = lmtable_lookup_end;
  tbl->lookup_srt = ntnine_lookup_srt;

  return 1;
}
/*-------------------------------------------------------------------------*/
void ntnine_find(void *hsh_, void *tbl_, void *el_) {
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
  
  ntnine_mkredux(tbl, hash, pwd, n_redux);

  for (c=1; c<ncols-col; ++c) {
    ++n_redux;

    ntnine_mkhash(pwd, tmp);
    ntnine_mkredux(tbl, tmp, pwd, n_redux);
    ++stat->hredux;
  }

  /* Compute the prefix and postfix. */
  
  uint32_t prefix  = ntnine_bin36(pwd, 5);
  uint16_t postfix = (uint16_t)ntnine_bin37(pwd+5, 4);
  
  el->prefix  = prefix;
  el->postfix = postfix;
}
/*-------------------------------------------------------------------------*/
int ntnine_lookup_srt(void *hsh_, void *tbl_, void *el_) {
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

  ntnine_unbin36(start, pwd, 6);

  return 1;
}
/*-------------------------------------------------------------------------*/
int ntnine_check(void *hsh_, void *tbl_, void *el_) {
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
    ntnine_mkhash(pwd, tmp);
    ntnine_mkredux(tbl, tmp, pwd, n_redux);

    ++n_redux;
    ++stat->hredux;
    ++stat->falarm_hredux;
  }  

  /* Check if we got the same hash. */

  ntnine_mkhash(pwd, tmp);
  
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
int ntnine_isvalid(void *hsh_, void *tbl_) {
  hash_t *hsh = hsh_;

  return hsh->kind == nt ? 1 : 0;
}
/*-------------------------------------------------------------------------*/
void ntnine_mkredux(table_t *tbl, uchar_t *hash, uchar_t *pwd, 
			int n_redux) {

  /* Convert the hash to four unsigned 32-bit integers. */
  
  uint32_t idx[6];
  
  idx[0] = ftohl(*(uint32_t*)hash) ^ n_redux;
  idx[2] = ftohl(*(uint32_t*)(hash+4));
  idx[4] = ftohl(*(uint32_t*)(hash+8));
  idx[5] = ftohl(*(uint32_t*)(hash+12));
    
  int i;

  /* Compute the password */

  idx[1] = idx[0] % power2_36;
  idx[0] /= power2_36;
  idx[0] %= power2_36;

  idx[3] = idx[2] % power2_36;
  idx[2] /= power2_36;
  idx[2] %= power2_36;

  pwd[0] = ntnine_low36[idx[0]/36];
  pwd[1] = ntnine_low36[idx[0]%36];
  pwd[2] = ntnine_low36[idx[1]/36];
  pwd[3] = ntnine_low36[idx[1]%36];
  pwd[4] = ntnine_low36[idx[2]/36];
  pwd[5] = ntnine_low36[idx[2]%36];
  pwd[6] = ntnine_low36[idx[3]/36];
  pwd[7] = ntnine_low36[idx[3]%36];

  /* Finish the password. */


  if (idx[5] > max8) {
    idx[4] %= power2_36;
    pwd[8] = ntnine_low36[idx[4]/36];
    pwd[9] = 0;
  }  else {
    pwd[8] = 0;
    for (i=0; i<8; i++)
      if (pwd[i] > 0x60) {
	pwd[i] -= 0x20;
	break;
      }
  }
}
/*-------------------------------------------------------------------------*/
void ntnine_mkhash(uchar_t *pwd, uchar_t *hash) {
  make_nthash((char*)pwd, (char*)hash);
}
/*-------------------------------------------------------------------------*/
uint32_t ntnine_bin36(uchar_t *input, int length) {
  
  uint32_t sum=0;
  int i = 0;

  for (i=0; i<length; i++)  {
    sum = sum*36 + (uint64_t) (strchr((char*)ntnine_low36,tolower(input[i])) - (char*)ntnine_low36); 
  }
  return sum;
}
/*-------------------------------------------------------------------------*/
uint32_t ntnine_bin37(uchar_t *input, int length) {
  
  uint32_t sum=0;
  int i = 0;

  for (i=0; i<length; i++) {
    if (input[i]==0)
      sum = sum*37;
    else
      sum = sum*37+ 1 + (uint64_t) (strchr((char*)ntnine_low36,input[i]) - (char*)ntnine_low36);
  }
  return sum;
}
/*-------------------------------------------------------------------------*/
int ntnine_unbin36(uint32_t input, uchar_t *output, int length) {

  int i = 0;

  for (i=length-1; i>=0; i--) {
    output[i]=ntnine_low36[input%36];
    input/=36;
  }
  return 1;
}
