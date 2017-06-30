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
#ifndef NTDICT_H
#define NTDICT_H

#include <stdint.h>

#include "misc.h"
#include "table.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define PREFIX     0x3F
#define CAPITALIZE 0x40
#define CAPITALIZE_1ST 0x80
#define L33TSP34K  0x01
#define CHOSE_POST 0x02
#define POST_1     0xFC
#define POST_2     0x3F

static const uchar_t ntdict63[64]="0123456789abcdefghijklmnopqrstuvwxyz!\"#$%&'()*+,-./:;<=>?@[\\]_|\0"; 
static const uchar_t ntdict64[64]="0123456789abcdefghijklmnopqrstuvwxyz !\"#$%&'()*+,-./:;<=>?@[\\]_|";

int ntdict_setup(void *tbl_);
void ntdict_find(void *hsh_, void *tbl_, void *el_);
int ntdict_lookup_srt(void *hsh_, void *tbl_, void *el_);
int ntdict_check(void *hsh_, void *tbl_, void *el_);
int ntdict_isvalid(void *hsh_, void *tbl_);

void ntdict_mkredux(table_t *tbl, uchar_t *hash, uchar_t *pwd, int n_redux);
void ntdict_mkhash(uchar_t *pwd, uchar_t *hash);

#ifdef  __cplusplus
}
#endif
#endif
