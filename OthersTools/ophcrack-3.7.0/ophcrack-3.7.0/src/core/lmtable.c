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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <openssl/des.h>

#include "lmtable.h"
#include "lmflash.h"
#include "hash.h"
#include "ophel.h"
/*-------------------------------------------------------------------------*/
int lmtable_setup(void *tbl_) {
  table_t *tbl = tbl_;

  switch (tbl->kind) {
  case lmalphanum10k:
    tbl->ncols  = 10000;
    tbl->offset = 10000;
    tbl->sizes  = lmalphanum10k_sizes;
    break;

  case lmalphanum5k:
    tbl->ncols  = 5000;
    tbl->offset = 10000;
    tbl->sizes  = lmalphanum5k_sizes;
    break;
 
  case lmextended:
    tbl->ncols  = 20479;
    tbl->offset = 71538;
    tbl->sizes  = lmextended_sizes;
    break;
   
  default:
    break;
  }

  tbl->find    = lmtable_find;
  tbl->check   = lmtable_check;
  tbl->isvalid = lmtable_isvalid;

  tbl->lookup_idx = lmtable_lookup_idx;
  tbl->lookup_end = lmtable_lookup_end;
  tbl->lookup_srt = lmtable_lookup_srt;

  return 1;
}
/*-------------------------------------------------------------------------*/
void lmtable_find(void *hsh_, void *tbl_, void *el_) {
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

  /* Get the charset. */

  char *charset = 0;
  int chars_size = 0;

  switch (tbl->kind) {
  case lmalphanum5k:
  case lmalphanum10k:
    charset    = (char*)alphanum_chars;
    chars_size = sizeof(alphanum_chars);
    break;

  case lmextended:
    charset    = (char*)extended_chars;
    chars_size = sizeof(extended_chars);
    break;

  default:
    break;
  }

  /* Compute the prefix from the first 4 bytes of the password. */

  int i;
  int len0 = strlen((char*)pwd);
  int len = len0 < 4 ? len0 : 4;

  uint32_t prefix = 0;
  uint64_t offset = chars_size * chars_size * chars_size * chars_size;

  char *p;

  for (i=0; i<4; ++i) {
    p = strchr(charset, pwd[i]);
    
    if (i < len)
      prefix = prefix * chars_size + (p - charset);
    else {
      prefix += offset;
      offset /= chars_size;
    }
  }

  el->prefix = prefix;

  /* Compute the postfix from the last 3 bytes of the password. */

  uint16_t postfix = 0;

  len    = (len0 > 4 ? len0-4 : 0);
  offset = chars_size * chars_size * chars_size;

  for (i=0; i<3; ++i) {
    p = strchr(charset, pwd[i+4]);
    
    if (i < len)
      postfix = postfix * chars_size + (p - charset);
    else {
      postfix += offset;
      offset  /= chars_size; 
    }
  }

  el->postfix = postfix;
}
/*-------------------------------------------------------------------------*/
int lmtable_lookup_idx(void *hsh_, void *tbl_, void *el_) {
  table_t *tbl = tbl_;
  ophel_t *el  = el_;
  
  uint32_t prefix = el->prefix;

  ophstat_t *stat = el->stat;
  ++stat->prefix;

  /* Sad table binarization implies sad bugs in tables... */

  if ((prefix+1)*4 == tbl->idxsize) prefix--;

  /* If the .index file has been preloaded, then we look into the memory. */

  if (tbl->idxmem) {
    uint32_t *mem = (uint32_t*)(tbl->idxmem + 4*prefix);

    el->low  = mem[0];
    el->high = mem[1];
  }

  /* Otherwise, we access the disk. */

  else {
    FILE *idxfile = tbl->idxfile;

    el->low = 0;
    el->high = 0;
    fseeko(idxfile, 4*prefix, SEEK_SET);
    fread(&el->low, 4, 1, idxfile);
    fread(&el->high, 4, 1, idxfile);
    ++stat->fseek_idx;
  }

  /* Sad table binarization implies sad bugs in tables... ENCORE */

  if ((prefix+1)*4 == tbl->idxsize) el->high = tbl->endsize / 2;
  if (prefix == 0) el->low = 0;
  if (el->high == 0 && el->low != 0) el->high = tbl->endsize / 2;

  /* Make sure that the bytes are in the correct order. */

  el->low  = ftohl(el->low);
  el->high = ftohl(el->high);

  assert(el->high >= el->low);

  return 1;
}
/*-------------------------------------------------------------------------*/
int lmtable_lookup_end(void *hsh_, void *tbl_, void *el_) {
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

  el->offset =  4 * (low+i);


  if (i < n) ++stat->postfix;

  return i == n ? 0 : 1;
}
/*-------------------------------------------------------------------------*/
int lmtable_lookup_srt(void *hsh_, void *tbl_, void *el_) {
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

  /* Convert it to ASCII. */

  int i;
  uchar_t *pwd = (uchar_t*)el->pwd;
  uchar_t *charset = 0;

  switch (tbl->kind) {
  case lmalphanum5k:
  case lmalphanum10k:
    charset = (uchar_t*)alphanum_chars;
    break;

  case lmextended:
    charset = (uchar_t*)extended_chars;
    break;

  default:
    break;
  }

  start &= 0x3FFFFFFF;

  pwd[6] = 0;
  pwd[7] = 0;

  /* WARNING. We must absolutely use 36 here instead of 'chars_size'
     because of a bug in the generation of the LM Extended tables. */

  for (i=5; i>=0; --i) {
    pwd[i] = charset[start % 36];
    start /= 36;
  }

  return 1;
}
/*-------------------------------------------------------------------------*/
int lmtable_check(void *hsh_, void *tbl_, void *el_) {
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
    lmtable_mkredux(tbl, tmp, pwd, n_redux);

    ++n_redux;
    ++stat->hredux;
    ++stat->falarm_hredux;
  }  

  /* Check if we got the same hash. */

  lmtable_mkhash(pwd, tmp);
  
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
int lmtable_isvalid(void *hsh_, void *tbl_) {
  hash_t *hsh  = hsh_;

  return (hsh->kind == lm1 || hsh->kind == lm2) ? 1 : 0;
}
/*-------------------------------------------------------------------------*/
void lmtable_mkredux(table_t *tbl, uchar_t *hash, uchar_t *pwd, 
		     int n_redux) {
  int i, n;
  uint32_t *max = 0;
  uchar_t *charset = 0;
  int chars_size = 0;

  switch (tbl->kind) {
  case lmalphanum5k:
  case lmalphanum10k:
    max = (uint32_t*)alphanum_max;
    charset    = (uchar_t*)alphanum_chars;
    chars_size = sizeof(alphanum_chars);
    break;
    
  case lmextended:
    max = (uint32_t*)extended_max;
    charset    = (uchar_t*)extended_chars;
    chars_size = sizeof(extended_chars);
    break;

  case lmflash:
    max = (uint32_t*)lmflash_max;
    charset    = (uchar_t*)extended_chars;
    chars_size = sizeof(extended_chars);
    break;

  default:
    break;
  }

  /* Convert the hash to two unsigned 32-bit integers. */

  uint32_t idx[2];
  uint32_t int_hash[2];

  int_hash[0] = ftohl(*(uint32_t*)hash);
  int_hash[1] = ftohl(*(uint32_t*)(hash+4));

  /* XOR the 1st integer with n_redux. */

  idx[0] = int_hash[0] ^ n_redux;
  idx[1] = int_hash[1];

  n = idx[1];

  /* Get 4 characters out of the upper word and 3 out of the lower. */

  for (i=0; i<4; i++) {
    pwd[i]  = charset[idx[0] % chars_size];
    idx[0] /= chars_size;
  }

  for (i=4; i<7; i++) {
    pwd[i]  = charset[idx[1] % chars_size]; 
    idx[1] /= chars_size;
  }

  /* Trucate the password. */

  if (n<max[7]) {
    pwd[6]=0; 
    if (n<max[6]) {
      pwd[5]=0; 
      if (n<max[5]) {
	pwd[4]=0; 
	if (n<max[4]) {
	  pwd[3]=0; 
	  if (n<max[3]) {
	    pwd[2]=0; 
	    if (n<max[2]) { 
	      pwd[1]=0;	
	    }
	  }
	}
      }
    }
  }
}
/*-------------------------------------------------------------------------*/
void lmtable_mkhash(uchar_t *pwd, uchar_t *hash) {
  uchar_t key[8];
  DES_key_schedule ks;
  DES_cblock *magic = (DES_cblock*)lmmagic;
  
  key[0] = pwd[0];
  key[1] = (pwd[0] << 7) | (pwd[1] >> 1);
  key[2] = (pwd[1] << 6) | (pwd[2] >> 2);
  key[3] = (pwd[2] << 5) | (pwd[3] >> 3);
  key[4] = (pwd[3] << 4) | (pwd[4] >> 4);
  key[5] = (pwd[4] << 3) | (pwd[5] >> 5);
  key[6] = (pwd[5] << 2) | (pwd[6] >> 6);
  key[7] = (pwd[6] << 1) ;

  DES_set_odd_parity(&key);
  DES_set_key(&key, &ks);
  DES_ecb_encrypt(magic, (DES_cblock*)hash, &ks, DES_ENCRYPT);
}
