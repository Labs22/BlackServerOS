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
#ifndef LMGERMAN_H
#define LMGERMAN_H

#include "misc.h"
#include "table.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct lmgerman_max_t_ {
  char n;
  char k;
  uint32_t max;
} lmgerman_max_t;

static const uchar_t lmgerman_chars[73] = { 
  0x8e, 0x99, 0x9a, 0xe1, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B',
  'C',  'D',  'E',  'F',  'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
  'S',  'T',  'U',  'V',  'W', 'X', 'Y', 'Z', ' ', '!', '"', '#', '$', '%', '&', '\'',
  '(',  ')',  '*',  '+',  ',', '-', '.', '/', ':', ';', '<', '=', '>', '?', '@', '[',
  '\\', ']',  '^',  '_',  '`', '{', '|', '}', '~' };

static const uchar_t lmgerman_ranks[28][35] = {
  {1,2,4,8,16,32,64,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {3,5,9,17,33,65,6,10,18,34,66,12,20,36,68,24,40,72,48,80,96,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {7,11,19,35,67,13,21,37,69,25,41,73,49,81,97,14,22,38,70,26,42,74,50,82,98,28,44,76,52,84,100,56,88,104,112},
  {1,2,4,8,16,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {3,5,9,17,33,6,10,18,34,12,20,36,24,40,48,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {15,23,39,71,27,43,75,51,83,99,29,45,77,53,85,101,57,89,105,113,30,46,78,54,86,102,58,90,106,114,60,92,108,116,120},
  {1,2,4,8,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {7,11,19,35,13,21,37,25,41,49,14,22,38,26,42,50,28,44,52,56,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {31,47,79,55,87,103,59,91,107,115,61,93,109,117,121,62,94,110,118,122,124,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {3,5,9,17,6,10,18,12,20,24,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {15,23,39,27,43,51,29,45,53,57,30,46,54,58,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {1,2,4,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {7,11,19,13,21,25,14,22,26,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {63,95,111,119,123,125,126,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {3,5,9,6,10,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {31,47,55,59,61,62,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {15,23,27,29,30,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {1,2,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {7,11,13,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {63,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {3,5,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {31,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

static const char lmgerman_choose[8][8] = 
  { { 1, 0,  0,  0,  0,  0, 0, 0 },
    { 1, 1,  0,  0,  0,  0, 0, 0 },
    { 1, 2,  1,  0,  0,  0, 0, 0 },
    { 1, 3,  3,  1,  0,  0, 0, 0 },
    { 1, 4,  6,  4,  1,  0, 0, 0 },
    { 1, 5, 10, 10,  5,  1, 0, 0 },
    { 1, 6, 15, 20, 15,  6, 1, 0 },
    { 1, 7, 21, 35, 35, 21, 7, 1 } };

static const lmgerman_max_t lmgerman_spec_max[28] = {
  { 7, 1, 3560555371U },
  { 7, 2, 4179782393U },
  { 7, 3, 4239611091U },
  { 6, 1, 4283841592U },
  { 6, 2, 4290251810U },
  { 7, 4, 4293720140U },
  { 5, 1, 4294254325U },
  { 6, 3, 4294749801U },
  { 7, 5, 4294870439U },
  { 5, 2, 4294932373U },
  { 6, 4, 4294953915U },
  { 4, 1, 4294960109U },
  { 5, 3, 4294963699U },
  { 7, 6, 4294966030U },
  { 4, 2, 4294966569U },
  { 6, 5, 4294967069U },
  { 5, 4, 4294967173U },
  { 3, 1, 4294967240U },
  { 4, 3, 4294967261U },
  { 7, 7, 4294967280U },
  { 6, 6, 4294967285U },
  { 3, 2, 4294967289U },
  { 5, 5, 4294967290U },
  { 2, 1, 4294967291U },
  { 4, 4, 4294967292U },
  { 3, 3, 4294967293U },
  { 2, 2, 4294967294U },
  { 1, 1, 4294967295U } };

int lmgerman_setup(void *tbl_);
void lmgerman_find(void *hsh_, void *tbl_, void *el_);
int lmgerman_lookup_srt(void *hsh_, void *tbl_, void *el_);
int lmgerman_check(void *hsh_, void *tbl_, void *el_);
int lmgerman_isvalid(void *hsh_, void *tbl_);

void lmgerman_mkredux(table_t *tbl, uchar_t *hash, uchar_t *pwd, int n_redux);

uint64_t lmgerman_bin73(uchar_t *input);
void lmgerman_unbin36(uint32_t input, uchar_t *output);

#ifdef  __cplusplus
}
#endif
#endif
