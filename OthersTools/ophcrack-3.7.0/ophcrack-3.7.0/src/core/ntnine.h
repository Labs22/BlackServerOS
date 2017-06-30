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
#ifndef NTNINE_H
#define NTNINE_H

#include <stdint.h>

#include "misc.h"
#include "table.h"

#ifdef  __cplusplus
extern "C" {
#endif
  
static const uchar_t ntnine_low36[36]="0123456789abcdefghijklmnopqrstuvwxyz";
static const uint32_t max8 = 116080197;
static const uint32_t power2_36 = 1296;

int ntnine_setup(void *tbl_);
void ntnine_find(void *hsh_, void *tbl_, void *el_);
int ntnine_lookup_srt(void *hsh_, void *tbl_, void *el_);
int ntnine_check(void *hsh_, void *tbl_, void *el_);
int ntnine_isvalid(void *hsh_, void *tbl_);

void ntnine_mkredux(table_t *tbl, uchar_t *hash, uchar_t *pwd, int n_redux);
void ntnine_mkhash(uchar_t *pwd, uchar_t *hash);

uint32_t ntnine_bin36(uchar_t *input, int length);
uint32_t ntnine_bin37(uchar_t *input, int length);
int ntnine_unbin36(uint32_t input, uchar_t *output, int length);

#ifdef  __cplusplus
}
#endif
#endif
