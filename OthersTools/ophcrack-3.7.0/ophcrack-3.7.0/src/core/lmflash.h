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
#ifndef LMFLASH_H
#define LMFLASH_H

#include "misc.h"
#include "table.h"

#ifdef  __cplusplus
extern "C" {
#endif

static const uint32_t lmflash_max[8] = { 0, 0, 1, 4, 191, 13076, 902116, 62245904 };

int lmflash_setup(void *tbl_);
void lmflash_find(void *hsh_, void *tbl_, void *el_);
int lmflash_lookup_srt(void *hsh_, void *tbl_, void *el_);

uint64_t lmflash_bin69(uchar_t *input);
void lmflash_unbin69(uint32_t input, uchar_t *output);

#ifdef  __cplusplus
}
#endif
#endif
