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

#include "lmflash.h"
#include "lmtable.h"
#include "hash.h"
#include "ophel.h"
#include "original.h"
#include "misc.h"

static uint64_t lmflash_sizes[14] = { 6, 268435456, 5279834004ULL, 10559668008ULL, 5279862016ULL, 10559724032ULL, 5279757448ULL, 10559514896ULL, 5279801098ULL, 10559602196ULL, 5279747172ULL, 10559494344ULL, 5279855682ULL, 10559711364ULL};
/*-------------------------------------------------------------------------*/
int lmflash_setup(void *tbl_) {
  table_t *tbl = tbl_;

  tbl->ncols  = 2200;
  tbl->offset = 100000;
  tbl->sizes  = lmflash_sizes;

  tbl->find    = lmflash_find;
  tbl->check   = lmtable_check;
  tbl->isvalid = lmtable_isvalid;

  tbl->lookup_idx = lmtable_lookup_idx;
  tbl->lookup_end = lmtable_lookup_end;
  tbl->lookup_srt = lmflash_lookup_srt;

  return 1;
}
/*-------------------------------------------------------------------------*/
void lmflash_find(void *hsh_, void *tbl_, void *el_) {
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
  
  lmtable_mkredux(tbl, hash, pwd, n_redux);
  
  for (c=1; c<ncols-col; ++c) {
    ++n_redux;

    lmtable_mkhash(pwd, tmp);
    lmtable_mkredux(tbl, tmp, pwd, n_redux);
    ++stat->hredux;
  }
  
  /* Compute the prefix and postfix. */

  uint64_t pw_bin  = lmflash_bin69(pwd);
  uint32_t prefix  = (uint32_t)(pw_bin >> 17 & 0x00000000FFFFFFFF);
  uint16_t postfix = (uint16_t)(pw_bin & 0x000000000000FFFF);

  el->prefix  = prefix;
  el->postfix = postfix;
}
/*-------------------------------------------------------------------------*/
int lmflash_lookup_srt(void *hsh_, void *tbl_, void *el_) {
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

  memset(pwd, 0, 8);

  lmflash_unbin69(start, pwd);

  return 1;
}
/*-------------------------------------------------------------------------*/
uint64_t lmflash_bin69(uchar_t *input) {
  int i = 0;
  uint64_t sum = 0;
  uint64_t offset = 7446353252589ULL;
  
  for (i=0; i<7; i++) {
    if (input[i]) {
      char *x1 = (char*)memchr(extended_chars, input[i], 73);
      char *x2 = (char*)extended_chars;
      
      sum = sum*69 + (uint64_t)(x1-x2);
    } else {
      sum += offset;
      offset /= 69;
    }
  }

  return sum;
}
/*-------------------------------------------------------------------------*/
void lmflash_unbin69(uint32_t input, uchar_t *output) {
  int i = 0;

  for (i=5; i>=0; i--) {
    output[i] = extended_chars[input % 69];
    input /= 69;
  }
}
