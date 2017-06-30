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
#ifndef NTEIGHTXL_H
#define NTEIGHTXL_H

#include <stdint.h>

#include "misc.h"
#include "table.h"

#ifdef  __cplusplus
extern "C" {
#endif
  
static const uchar_t nteightxl_ext95[95]="0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";

static const uint64_t nteightxl_sizes[10] = { 4, 3954293915, 152934456304LLU, 382336140760LLU, 152934550784LLU, 382336376960LLU, 152934374040LLU, 382335935100LLU, 152933057672LLU, 382332644180LLU};

int nteightxl_setup(void *tbl_);
void nteightxl_find(void *hsh_, void *tbl_, void *el_);
int nteightxl_lookup_idx(void *hsh_, void *tbl_, void *el_);
int nteightxl_lookup_end(void *hsh_, void *tbl_, void *el_);
int nteightxl_lookup_srt(void *hsh_, void *tbl_, void *el_);
int nteightxl_check(void *hsh_, void *tbl_, void *el_);
int nteightxl_isvalid(void *hsh_, void *tbl_);

void nteightxl_mkredux(table_t *tbl, uchar_t *hash, uchar_t *pwd, int n_redux);
void nteightxl_mkhash(uchar_t *pwd, uchar_t *hash);

uint64_t nteightxl_bin95(uchar_t *input, int length);
int nteightxl_unbin95(uint64_t input, uchar_t *output, int length);

#ifdef  __cplusplus
}
#endif
#endif
