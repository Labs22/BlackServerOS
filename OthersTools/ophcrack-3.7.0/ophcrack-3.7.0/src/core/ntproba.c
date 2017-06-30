/*
 *   
 *   Ophcrack is a Lanmanager/NTLM hash cracker based on the faster time-memory
 *   trade-off using rainbow tables. 
 *   
 *   Created with the help of: Maxime Mueller, Luca Wullschleger, Claude
 *   Hochreutiner, Andreas Huber and Etienne Dysli.
 *   
 *   Copyright (c) 2013 Philippe Oechslin, Cedric Tissieres, 
 *                      Bertrand Mesot, Pierre Lestringant
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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "ntproba.h"
#include "proba_info.h"
#include "hashToPwd.h"
#include "ophel.h"
#include "hash.h"
#include "ioMarkov.h"
#include "ioPattern.h"

typedef struct {
  int refcount;
  proba_info *proba;
} ntproba_global_t;

static ntproba_global_t globalfree = { 0, NULL };
static ntproba_global_t global10g  = { 0, NULL };
static ntproba_global_t global60g  = { 0, NULL };
/*-------------------------------------------------------------------------*/
ntproba_global_t *ntproba_global(table_t *tbl) {
  switch (tbl->kind) {
  case ntprobafree: return &globalfree;
  case ntproba10g:  return &global10g;
  case ntproba60g:  return &global60g;
  default: assert(0);
  }

  return NULL;
}
/*-------------------------------------------------------------------------*/
int ntproba_setup(void *tbl_) {
  table_t *tbl = tbl_;

  tbl->ncols = 35000;
  tbl->offset = 35000;
  tbl->shared_init = 1;

  switch (tbl->kind) {
  case ntprobafree:
    tbl->sizes = ntprobafree_sizes;
    tbl->inisize = 152698328;
    break;

  case ntproba10g:
    tbl->sizes = ntproba10g_sizes;
    tbl->inisize = 154943200;
    break;

  case ntproba60g:
    tbl->sizes = ntproba60g_sizes;
    tbl->inisize = 151964072;
    break;

  default:
    assert(0);
  }

  tbl->init = ntproba_init;
  tbl->cleanup = ntproba_cleanup;

  tbl->find    = ntproba_find;
  tbl->check   = ntproba_check;
  tbl->isvalid = ntproba_isvalid;

  tbl->lookup_idx = ntproba_lookup_idx;
  tbl->lookup_end = ntproba_lookup_end;
  tbl->lookup_srt = ntproba_lookup_srt;

  return 1;
}
/*-------------------------------------------------------------------------*/
proba_info *ntproba_proba_info_alloc(table_t *tbl) {
  proba_info *proba = proba_info_alloc();

  switch (tbl->kind) {
  case ntprobafree:
    proba->mtype = MARK2;
    proba->power2 = 39;
    proba->nbByteM0 = 4;
    proba->nbByteM = 4;
    proba->nbByteBin = 3;
    proba->nbBitIndex = 15;
    break;

  case ntproba10g:
    proba->mtype = MARK2;
    proba->power2 = 43;
    proba->nbByteM0 = 4;
    proba->nbByteM = 4;
    proba->nbByteBin = 3;
    proba->nbBitIndex = 19;
    break;

  case ntproba60g:
    proba->mtype = MARK2;
    proba->power2 = 46;
    proba->nbByteM0 = 5;
    proba->nbByteM = 4;
    proba->nbByteBin = 3;
    proba->nbBitIndex = 22;
    break;

  default:
    assert(0);
  }

  if ((!readImprobaStructFromBinFile(&(proba->mis), tbl->path, "markovI.dat"))
      || (!initMarkovDynProgWrapper(proba->mdpw, proba->mis, proba->mtype))
      || (!readPatternArrayFromXMLFile(proba->array, tbl->path, "pattern.xml"))
      || (!checkPatternArrayCrackMarkov(proba->array, proba->mdpw, proba->mis))) {

    ntproba_cleanup(tbl);

    return NULL;
  }

  return proba;
}
/*-------------------------------------------------------------------------*/
void *ntproba_init(void *tbl_) {
  table_t *tbl = tbl_;
  ntproba_global_t *global = ntproba_global(tbl);

  if (!global->proba) {
    assert(global->refcount == 0);
    global->proba = ntproba_proba_info_alloc(tbl);
    global->refcount += global->proba ? 1 : 0;
  }
  else {
    assert(global->refcount > 0);
    global->refcount += 1;
  }

  return global->proba;
}
/*-------------------------------------------------------------------------*/
void ntproba_cleanup(void* tbl_) {
  table_t *tbl = tbl_;
  ntproba_global_t *global = ntproba_global(tbl);

  assert(global->refcount > 0);
  assert(global->proba == tbl->param);

  global->refcount -= 1;

  if (global->refcount == 0) {
    proba_info *proba = global->proba;

    deleteMarkovDynProgWrapper(proba->mdpw);
    deletePatternArray(proba->array);
    deleteImprobaStruct(proba->mis);
    proba_info_free(proba);

    global->proba = NULL;
  }
}
/*-------------------------------------------------------------------------*/
void ntproba_find(void *hsh_, void *tbl_, void *el_) {
  hash_t* 	hsh = hsh_;
  table_t* 	tbl = tbl_;
  ophel_t* 	el  = el_;
  proba_info *proba = tbl->param;

  uchar_t* 	hash = hsh->hash;
  ophstat_t* 	stat = el->stat;
  uint64_t	end;
  uint32_t	idx = tbl->idx;
  uint32_t 	nbColumn = tbl->ncols;
  uint32_t	startColumn = el->col;
  uint32_t 	power2 = proba->power2;

  end = hashToIndex(proba->array, proba->mdpw, proba->mis, (unsigned int*)hash, startColumn, nbColumn, power2, idx, nbColumn);
  stat->hredux += nbColumn-startColumn-1;

  el->prefix  = end >> (power2 - proba->nbBitIndex);
  el->postfix = end & ((uint64_t)0xffffffffffffffff >> (64 + proba->nbBitIndex - power2));
}
/*-------------------------------------------------------------------------*/
int ntproba_lookup_idx(void *hsh_, void *tbl_, void *el_) {
  table_t* 	tbl = tbl_;
  ophel_t*	el  = el_;
  uint32_t 	prefix = el->prefix;
  ophstat_t*	stat = el->stat;
  proba_info  *proba = tbl->param;
  uint32_t	nbByteM = proba->nbByteM;

  stat->prefix++;

  if (tbl->idxmem) {
    char* mem = tbl->idxmem + prefix*nbByteM;
    el->low = 0;
    el->high = 0;
    memcpy(&(el->low), mem, nbByteM);
    memcpy(&(el->high), mem+nbByteM, nbByteM);
  }
  else {
    FILE *idxfile = tbl->idxfile;
    fseeko(idxfile, prefix*nbByteM, SEEK_SET);
    el->low = 0;
    el->high = 0;
    fread(&(el->low), nbByteM, 1, idxfile);
    fread(&(el->high), nbByteM, 1, idxfile);
    stat->fseek_idx++;
  }

  return 1;
}
/*-------------------------------------------------------------------------*/
int ntproba_lookup_end(void *hsh_, void *tbl_, void *el_) {
  table_t* 	tbl = tbl_;
  ophel_t* 	el  = el_;
  ophstat_t* 	stat = el->stat;
  proba_info  *proba = tbl->param;
  uint64_t 	postfix = el->postfix;
  uint64_t 	range = el->high - el->low;
  uint64_t 	i;
  uint32_t	nbByteBin = proba->nbByteBin;
  uint64_t 	temp;
  char* 		mem;

  if (tbl->endmem) {
    mem = tbl->endmem + el->low*nbByteBin;
		
    for (i = 0; i < range; i++){
      temp = 0;
      memcpy(&temp, mem+i*nbByteBin, nbByteBin);
      if (temp == postfix){
	stat->postfix ++;
	break;
      }
    }
  }
  else {
    mem = (char*)malloc(range*nbByteBin);
    FILE* endfile = tbl->endfile;

    fseeko(endfile, el->low*nbByteBin, SEEK_SET);
    fread(mem, nbByteBin, range, endfile);
    stat->fseek_end++;

    for (i = 0; i < range; i++){
      temp = 0;
      memcpy(&temp, mem+i*nbByteBin, nbByteBin);
      if (temp == postfix){
	stat->postfix ++;
	break;
      }
    }
    free(mem);
  }

  el->offset = el->low+i;

  return  (i == range) ? 0 : 1;
}
/*-------------------------------------------------------------------------*/
int ntproba_lookup_srt(void *hsh_, void *tbl_, void *el_) {
  table_t* 	tbl = tbl_;
  ophel_t* 	el  = el_;
  ophstat_t* 	stat = el->stat;
  proba_info  *proba = tbl->param;
  uint32_t	nbByteM0 = proba->nbByteM0;
	
  stat->start ++;

  if (tbl->srtmem) {
    el->start = 0;
    memcpy(&(el->start), tbl->srtmem + el->offset*nbByteM0, nbByteM0);
  }
  else {
    FILE *srtfile = tbl->srtfile;

    fseeko(srtfile, el->offset*nbByteM0, SEEK_SET);
    el->start = 0;
    fread(&(el->start), nbByteM0, 1, srtfile);
    stat->fseek_srt ++;
  }

  return 1;
}
/*-------------------------------------------------------------------------*/
int ntproba_check(void *hsh_, void *tbl_, void *el_) {
  hash_t* 	hsh   = hsh_;
  table_t* 	tbl  = tbl_;
  ophel_t* 	el   = el_;
  proba_info  *proba = tbl->param;
  ophstat_t* 	stat = el->stat;
  uint32_t	taille;
  uint32_t	idx = tbl->idx;
  uint32_t 	nbColumn = tbl->ncols;
  uint32_t	stopColumn = el->col;
  uint32_t 	power2 = proba->power2;
  char		pwdUTF[PWD_UTF_SIZE] = {0};

  taille = indexToPwdUTF(proba->array, proba->mdpw, proba->mis, el->start, power2, stopColumn, idx, nbColumn, pwdUTF);
	
  stat->hredux += stopColumn;
  stat->falarm_hredux += stopColumn;
	
  if (checkFoundPwd(pwdUTF, taille, (unsigned int*)(hsh->hash), el->pwd)){
    stat->match_table++;
    stat->falarm_hredux = 0;
    return 1;
  }
  else{
    stat->falarm ++;
    return 0;
  }
}
/*-------------------------------------------------------------------------*/
int ntproba_isvalid(void *hsh_, void *tbl_) {
  hash_t *hsh = hsh_;

  return (hsh->kind == nt)? 1 : 0;
}
/*-------------------------------------------------------------------------*/
