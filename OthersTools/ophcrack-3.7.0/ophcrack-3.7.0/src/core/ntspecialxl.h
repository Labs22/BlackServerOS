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
#ifndef NTSPECIALXL_H
#define NTSPECIALXL_H

#include <stdint.h>

#include "misc.h"
#include "table.h"

#ifdef  __cplusplus
extern "C" {
#endif
  
static const uchar_t ntspecialxl_ext95[95]="0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";

  static const uint64_t ntspecialxl_max[8] = {0ULL, 0ULL, 18446744073684721294ULL, 18446744071325840772ULL, 18446743847232191149ULL, 18446722558335476990ULL, 18444700113147631855ULL, 18252567820302344023ULL};

static const uint64_t ntspecialxl_offset[7] = {0ULL, 95ULL, 9120ULL, 866495ULL, 82317120ULL, 7820126495ULL, 742912017120ULL};


int ntspecialxl_setup(void *tbl_);
void ntspecialxl_find(void *hsh_, void *tbl_, void *el_);
int ntspecialxl_lookup_end(void *hsh_, void *tbl_, void *el_);
int ntspecialxl_lookup_srt(void *hsh_, void *tbl_, void *el_);
int ntspecialxl_check(void *hsh_, void *tbl_, void *el_);
int ntspecialxl_isvalid(void *hsh_, void *tbl_);

void ntspecialxl_mkredux(table_t *tbl, uchar_t *hash, uchar_t *pwd, int n_redux);
void ntspecialxl_mkhash(uchar_t *pwd, uchar_t *hash);

uint64_t ntspecialxl_bin95(uchar_t *input, int length);
int ntspecialxl_unbin95(uint64_t input, uchar_t *output, int length);

#ifdef  __cplusplus
}
#endif
#endif
