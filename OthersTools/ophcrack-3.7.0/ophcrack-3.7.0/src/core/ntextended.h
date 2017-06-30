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
#ifndef NTEXTENDED_H
#define NTEXTENDED_H

#include <stdint.h>

#include "misc.h"
#include "table.h"

#ifdef  __cplusplus
extern "C" {
#endif

static const uint32_t ntextended_max[9] = { 0, 0, 1, 6, 525, 49896, 4740179, 450317032, 2584948056U };
static const uchar_t ntextended_low36[36]="0123456789abcdefghijklmnopqrstuvwxyz";
static const uchar_t ntextended_alphanum62[62]="0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const uchar_t ntextended_ext95[95]="0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
static const uint64_t ntextended_offset[8] = {0ULL, 95ULL, 9120ULL, 866495ULL, 82317120ULL, 7820126495ULL, 742912017120ULL, 4264526623328ULL};

int ntextended_setup(void *tbl_);
void ntextended_find(void *hsh_, void *tbl_, void *el_);
int ntextended_lookup_srt(void *hsh_, void *tbl_, void *el_);
int ntextended_check(void *hsh_, void *tbl_, void *el_);
int ntextended_isvalid(void *hsh_, void *tbl_);

void ntextended_mkredux(table_t *tbl, uchar_t *hash, uchar_t *pwd, int n_redux);
void ntextended_mkhash(uchar_t *pwd, uchar_t *hash);

uint64_t ntextended_bin43(uchar_t *input);
void ntextended_unbin95(uint32_t input, uchar_t *output);

#ifdef  __cplusplus
}
#endif
#endif
