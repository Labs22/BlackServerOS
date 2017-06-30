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
#ifndef OPHSTAT_H
#define OPHSTAT_H

#include <stdint.h>

#include "list.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct ophstat_t_ {
  uint64_t hredux;
  uint64_t prefix;
  uint64_t postfix;
  uint64_t start;
  uint64_t fseek_idx;
  uint64_t fseek_end;
  uint64_t fseek_srt;
  uint64_t falarm;
  uint64_t falarm_hredux;
  uint64_t match_table;
  uint64_t match_bforce;
  uint32_t length[16];
  uint32_t category[16];
  list_t *time;
} ophstat_t;

ophstat_t *ophstat_alloc(void);
void ophstat_free(ophstat_t *stat);
void ophstat_add(ophstat_t *stat1, ophstat_t *stat2);
void ophstat_reset(ophstat_t *stat);

#ifdef  __cplusplus
}
#endif
#endif
