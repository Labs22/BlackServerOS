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
#ifndef LMTABLE_H
#define LMTABLE_H

#include "misc.h"
#include "table.h"

#ifdef  __cplusplus
extern "C" {
#endif

static const uchar_t alphanum_chars[36] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const uchar_t extended_chars[69] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
static const uint32_t alphanum_max[8] = { 0, 0, 2, 71, 2557, 92056, 3314018, 119304647 };
static const uint32_t extended_max[8] = { 0, 0, 1, 3, 190, 13076, 902235, 62254232 };
static const uchar_t lmmagic[8] = { 0x4B, 0x47, 0x53, 0x21, 0x40, 0x23, 0x24, 0x25 };

static const uint64_t lmalphanum10k_sizes[10] = { 4, 6910420, 30819324, 61638648, 30814120, 61628240, 30824574, 61649148, 30811564, 61623128};
static const uint64_t lmalphanum5k_sizes[10] = { 4, 6910420, 59026754, 118053508, 59033296, 118066592, 59036744, 118073488, 59032468, 118064936};
static const uint64_t lmextended_sizes[10] = { 4, 92001844, 636849574, 1273699148, 636848624, 1273697248, 636859776, 1273719552, 636844860, 1273689720};

int lmtable_setup(void *tbl_);

void lmtable_find(void *hsh_, void *tbl_, void *el_);
int lmtable_lookup_idx(void *hsh_, void *tbl_, void *el_);
int lmtable_lookup_end(void *hsh_, void *tbl_, void *el_);
int lmtable_lookup_srt(void *hsh_, void *tbl_, void *el_);
int lmtable_check(void *hsh_, void *tbl_, void *el_);
int lmtable_isvalid(void *hsh_, void *tbl_);

void lmtable_mkredux(table_t *tbl, uchar_t *hash, uchar_t *pwd, int n_redux);
void lmtable_mkhash(uchar_t *pwd, uchar_t *hash);

#ifdef  __cplusplus
}
#endif
#endif
