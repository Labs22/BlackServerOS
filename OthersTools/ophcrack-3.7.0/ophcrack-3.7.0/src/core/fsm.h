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
#ifndef STATE_H
#define STATE_H

#include <stdint.h>

#include "ophcrack.h"
#include "message.h"
#include "arg.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef enum { st_wait    = 0x01,
	       st_pause1  = 0x02,
	       st_pause2  = 0x04,
	       st_start   = 0x08,
	       st_preload = 0x10,
	       st_work1   = 0x20, 
	       st_work2   = 0x40 } state_t;

typedef struct fsm_t_ {
  state_t oldstate;
  state_t state;

  ophcrack_t *crack;
  arg_t *arg;

  int preload;
  int bforce;
  int pwait;
  int ssave;

  uint64_t psize_curr;
  uint64_t psize_total;
  uint64_t bforce_curr;
  uint64_t bforce_total;

  list_t *pending_msg;
  list_t *htoremove;
} fsm_t;

fsm_t *fsm_alloc(ophcrack_t *ophcrack);

message_t *fsm_next(fsm_t *fsm, message_t *msg);

void fsm_handle_start(fsm_t *fsm);
message_t *fsm_handle_preload(fsm_t *fsm, message_t *msg);
message_t *fsm_handle_pause1(fsm_t *fsm, message_t *msg);
message_t *fsm_handle_pause2(fsm_t *fsm, message_t *msg);
message_t *fsm_handle_work1(fsm_t *fsm, message_t *msg);
message_t *fsm_handle_work2(fsm_t *fsm, message_t *msg);

void fsm_reset_preload(fsm_t *fsm);
void fsm_reset_bforce(fsm_t *fsm);

void fsm_launch_preload(fsm_t *fsm);
void fsm_launch_bforce(fsm_t *fsm);

void fsm_ssave(fsm_t *fsm);
void fsm_pause(fsm_t *fsm);
void fsm_reset(fsm_t *fsm);

#ifdef  __cplusplus
}
#endif
#endif
