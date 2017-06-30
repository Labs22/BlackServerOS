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

#include "ntdict.h"
#include "table.h"
#include "hash.h"
#include "ophel.h"
#include "original.h"
#include "lmtable.h"

#define my_islower(x) (((unsigned)(x) >= 'a') && ((unsigned)(x) <= 'z'))

static uchar_t worddict[65536][16];
static uchar_t worddict_len[65536];
static uchar_t postdict[4096][16];
static uchar_t postdict_len[4096];
static int dicinit = 1;
static uint64_t ntdict_sizes[10] = { 4, 16777216, 34481438, 68962876, 34473102, 68946204, 34475286, 68950572, 34479228, 68958456};
/*-------------------------------------------------------------------------*/
int ntdict_setup(void *tbl_) {
  table_t *tbl = tbl_;

  tbl->ncols  = 20000;
  tbl->offset = 64000;
  tbl->sizes  = ntdict_sizes;

  tbl->find    = ntdict_find;
  tbl->check   = ntdict_check;
  tbl->isvalid = ntdict_isvalid;

  tbl->lookup_idx = lmtable_lookup_idx;
  tbl->lookup_end = lmtable_lookup_end;
  tbl->lookup_srt = ntdict_lookup_srt;

  /* Load the dictionary files if they have not been loaded yet. */

  if (dicinit) {
    char fname[STR_BUFF_SIZE];
    int i;

    /* Load the woddict file. */

    FILE *worddict_file;
  
    snprintf(fname, sizeof(fname), "%s/worddict", tbl->path);
    if ((worddict_file = fopen(fname, "rb")) == 0) return 0;
  
    worddict[0][0]=0;
    worddict_len[0]=0;
    
    for (i=1; i<65536; i++) {
      fread(worddict[i], 16, 1, worddict_file);
      worddict_len[i]=(uchar_t)strlen((char*)worddict[i]);
    }
    
    fclose(worddict_file);

    /* Load the postdict file. */
    
    FILE *postdict_file;
    
    snprintf(fname, sizeof(fname), "%s/postdict", tbl->path);
    if ((postdict_file = fopen(fname, "r")) == 0) return 0;
    
    postdict[0][0]=0;
    postdict_len[0]=0;
    
    for (i=1; i<4096; i++) {
      fread(postdict[i], 16, 1, postdict_file);
      postdict_len[i]=(uchar_t)strlen((char*)postdict[i]);
    }
    
    fclose(postdict_file);

    dicinit = 0;
  }

  return 1;
}
/*-------------------------------------------------------------------------*/
void ntdict_find(void *hsh_, void *tbl_, void *el_) {
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

  memcpy(tmp, hash, 16);

  /* Hash and redux until the last column. */
  
  for (c=1; c<ncols-col; c++) {
    n_redux++;

    ntdict_mkredux(tbl, tmp, pwd, n_redux);
    ++stat->hredux;
    ntdict_mkhash(pwd, tmp);
  }

  /* Compute the prefix and postfix. */

  uint32_t prefix = tmp[2];
  prefix += tmp[3]<<8;
  prefix += (tmp[4]&0x3F)<<16;

  uint16_t postfix = tmp[0];
  postfix += tmp[1]<<8;
  
  el->prefix  = prefix;
  el->postfix = postfix;
}
/*-------------------------------------------------------------------------*/
int ntdict_lookup_srt(void *hsh_, void *tbl_, void *el_) {
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

  /* Convert it to a valid hash (passed through pwd anyway). */

  uchar_t *pwd = (uchar_t*) el->pwd;

  pwd[0] = start&0xFF;
  pwd[1] = (start&0xFF00)>>8;
  pwd[2] = (start&0xFF0000)>>16;
  pwd[3] = (start&0xFF000000)>>24;

  return 1;
}
/*-------------------------------------------------------------------------*/
int ntdict_check(void *hsh_, void *tbl_, void *el_) {
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

  memset(tmp, 0, 16);
  memcpy(tmp, el->pwd, 4);

  /* Hash and redux until the starting column. */

  stat->falarm_hredux = 0;
  
  for (c=ncols-col; c<=ncols; c++) {
    ntdict_mkredux(tbl, tmp, pwd, n_redux);
    ntdict_mkhash(pwd, tmp);


    ++n_redux;
    ++stat->hredux;
    ++stat->falarm_hredux;
  }  

  /* Check if we got the same hash. */

  if (memcmp(tmp, hash, 16) == 0) {
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
int ntdict_isvalid(void *hsh_, void *tbl_) {
  hash_t *hsh = hsh_;

  return hsh->kind == nt ? 1 : 0;
}
/*-------------------------------------------------------------------------*/
void ntdict_mkredux(table_t *tbl, uchar_t *hash, uchar_t *pwd, 
			int n_redux) {

  /* Convert the hash to four unsigned 32-bit integers */
  /* and xor the lower 4 bytes of hash with n_redux */
  
  uchar_t idx[5];

  int r = n_redux;
  
  idx[0] = hash[0]^(r & 0xFF);
  r>>=8;
  idx[1] = hash[1]^(r & 0xFF);
  r>>=8;
  idx[2] = hash[2]^(r & 0xFF);
  r>>=8;
  idx[3] = hash[3]^(r & 0xFF);
  idx[4] = hash[4];
    
  int i;

  /* Compute the password according to the lower 32 bits of the hash: */

  /* combinations: */

  /* prefix: one of 64        (6 bits)  from in[0]
     capitalize word:         ( 1 bit)  from in[0] 0x40
     capitalize 1st letter:   ( 1 bit)  from in[0] 0x80
     1337speak:               ( 1 bit)  from in[3] 0x01 
     one word of dict        (16 bits)  from in[1] and in[2]
     postfix: select         ( 1 bit)   from in[3] 0x02 
     postdict                (12 bits)  from in[3] and in[4]
     or
     two chars               (12 bits)  from in[3] and in[4]
     ---------
                              38 bits
  */

  int capitalized_1st=0;
  int capitalized=0;
  int leet = 0;
  uint32_t pos = 0;
  memset(pwd, 0, 32*sizeof(uchar_t));
	 
  /* Compute the prefix */

  pwd[pos] = ntdict63[idx[0]&PREFIX];

  if ((pwd[pos]) && my_islower(pwd[pos]) && (idx[0]&CAPITALIZE_1ST)) {
    pwd[pos]-=32;
    capitalized_1st=1;
  }

  if (pwd[pos]) pos++; /* increment only if not empty char */

  /* Compute the word */

  uint32_t word = idx[1]+(idx[2]<<8);

  for (i=0; i<worddict_len[word]; i++) {
    pwd[pos]=worddict[word][i];

    /* Leetspeak */
    if (idx[3]&L33TSP34K) 
      switch(pwd[pos]) {
      case 'o': pwd[pos]='0'; leet=1; break;
      case 'l': pwd[pos]='1'; leet=1; break;
      case 'i': pwd[pos]='1'; leet=1; break;
      case 'z': pwd[pos]='2'; leet=1; break;
      case 'e': pwd[pos]='3'; leet=1; break;
      case 'a': pwd[pos]='4'; leet=1; break;
      case 's': pwd[pos]='5'; leet=1; break;
      case 't': pwd[pos]='7'; leet=1; break;
      case 'b': pwd[pos]='8'; leet=1; break;
      }

    /* capitalize first */
    if ((pos==0) && my_islower(pwd[pos]) && (idx[0]&CAPITALIZE_1ST)) {
      pwd[pos]-=32;
      capitalized_1st=1;
    }

        /* capitalize */
    if ((pos!=0) && my_islower(pwd[pos]) && (idx[0]&CAPITALIZE)) {
      pwd[pos]-=32;
      capitalized=1;
    }
    pos++;
  }

  /* Exceptions */

  /* word was not capitalizable, do something else instead */
  if ((idx[0]&CAPITALIZE) && !capitalized) 
    pwd[pos++]='.';

  /* first letter was not capitalizable, do something else instead */
  if ((idx[0]&CAPITALIZE_1ST) && !capitalized_1st)
    pwd[pos++]=',';

  /* word was not leetable, do something else instead */
  if ((idx[3]&L33TSP34K) && !leet)
    pwd[pos++]='-';

  /* Compute the postifx */

  if (idx[3]&CHOSE_POST) { /* postdict */
    word = ((idx[3]&POST_1)>>2) + ((idx[4]&POST_2)<<6);
    for (i=0; i<postdict_len[word]; i++)
      pwd[pos++]=postdict[word][i];
  }
  else { /* two chars */
    pwd[pos++]=ntdict64[(idx[3]&POST_1)>>2];
    pwd[pos]=ntdict63[(idx[4]&POST_2)];
    if (pwd[pos]) pos++; /* increment only if not empty char */
  }
}
/*-------------------------------------------------------------------------*/
void ntdict_mkhash(uchar_t *pwd, uchar_t *hash) {
  make_nthash((char*)pwd, (char*)hash);
}
