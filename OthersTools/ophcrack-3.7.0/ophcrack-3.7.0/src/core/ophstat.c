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
#include <stdlib.h>

#include "ophstat.h"
/*-------------------------------------------------------------------------*/
ophstat_t *ophstat_alloc(void) {
  ophstat_t *stat = (ophstat_t*)malloc(sizeof(ophstat_t));
  int i;

  for (i=0; i<16; i++) {
    stat->length[i] = 0;
    stat->category[i] = 0;
  }
  stat->time = list_alloc();

  ophstat_reset(stat);

  return stat;
}
/*-------------------------------------------------------------------------*/
void ophstat_free(ophstat_t *stat) {
  list_free(stat->time);
  free(stat);
}
/*-------------------------------------------------------------------------*/
void ophstat_add(ophstat_t *stat1, ophstat_t *stat2) {
  stat1->hredux        +=  stat2->hredux;
  stat1->prefix        +=  stat2->prefix;
  stat1->postfix       +=  stat2->postfix;
  stat1->start         +=  stat2->start;
  stat1->fseek_idx     +=  stat2->fseek_idx;
  stat1->fseek_end     +=  stat2->fseek_end;
  stat1->fseek_srt     +=  stat2->fseek_srt;
  stat1->falarm        +=  stat2->falarm;
  stat1->falarm_hredux +=  stat2->falarm_hredux;
  stat1->match_table   +=  stat2->match_table;
  stat1->match_bforce  +=  stat2->match_bforce;
}
/*-------------------------------------------------------------------------*/
void ophstat_reset(ophstat_t *stat) {
  stat->hredux = 0;
  stat->prefix = 0;
  stat->postfix = 0;
  stat->start = 0;
  stat->fseek_idx = 0;
  stat->fseek_end = 0;
  stat->fseek_srt = 0;
  stat->falarm = 0;
  stat->falarm_hredux = 0;
  stat->match_table = 0;
  stat->match_bforce = 0;

}
