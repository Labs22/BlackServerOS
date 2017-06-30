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
#include <assert.h>

#include "fsm.h"
/*-------------------------------------------------------------------------*/
fsm_t *fsm_alloc(ophcrack_t *crack) {
  fsm_t *fsm = (fsm_t*)malloc(sizeof(fsm_t));

  fsm->oldstate = st_wait;
  fsm->state = st_wait;

  fsm->crack = crack;
  fsm->arg = crack->arg;

  fsm->preload = 1;
  fsm->bforce = fsm->arg->bforce;
  fsm->pwait = 0;
  fsm->ssave = 0;
  
  fsm->pending_msg = list_alloc();
  fsm->htoremove = list_alloc();

  fsm_reset_preload(fsm);
  fsm_reset_bforce(fsm);

  return fsm;
}
/*-------------------------------------------------------------------------*/
message_t *fsm_next(fsm_t *fsm, message_t *msg) {
  ophcrack_t *crack = fsm->crack;
  scheduler_t *sched = crack->sched;

  /* A table is being or has been preloaded. */
    
  if (msg->kind == msg_preload) {
    msg_load_t *load = (msg_load_t*)msg->data;
    table_t *tbl = load->tbl;
    
    /* Beginning of preloading. */
    
    if (tbl == 0 && !load->done) {
      fsm->psize_curr  = 0;
      fsm->psize_total = load->size;
    }
    
    /* The preloading of a single file is done. */
    
    else if (tbl && load->done)
      fsm->psize_curr += load->size;
  }
  
  /* The brute force has started or a brute force task has been
     completed. */
  
  else if (msg->kind == msg_bforce) {
    msg_bforce_t *force = (msg_bforce_t*)msg->data;
    
    if (!force->done) {
      fsm->bforce_curr  = 0;
      fsm->bforce_total = force->count;
    }
    
    else {
      fsm->bforce_curr += force->count;
      assert(fsm->bforce_curr <= fsm->bforce_total);
    }
  }

  /* All the columns of a table have been considered. */
  
  else if (msg->kind == msg_done && ((msg_done_t*)msg->data)->kind == check) {

    if (fsm->state != st_pause1 && fsm->state != st_pause2) {
      scheduler_pause(sched);
      fsm->pwait = sched->nthreads;
      fsm->state = st_pause2;
    }

    fsm_reset_preload(fsm);
    fsm->oldstate = st_start;
  }

  // Treat the other messages according to the current state.
  
  else {
    switch (fsm->state) {
    case st_wait:
      break;
      
    case st_start:
      break;
      
    case st_pause1:
      msg = fsm_handle_pause1(fsm, msg);
      break;
      
    case st_pause2:
      msg = fsm_handle_pause2(fsm, msg);
      break;
      
    case st_preload: 
      msg = fsm_handle_preload(fsm, msg); 
      break;
      
    case st_work1:
      msg = fsm_handle_work1(fsm, msg); 
      break;
      
    case st_work2:
      msg = fsm_handle_work2(fsm, msg);
      break;
    }
  }
  
  return msg;
}
/*-------------------------------------------------------------------------*/
void fsm_handle_start(fsm_t *fsm) {
  /* At this point, no work task is running or scheduled. If the brute
     force is restarted or disabled, then it is safe to delete some
     hashes. */

  if (fsm->bforce || fsm->arg->bforce == 0) {
    list_t *htoremove = fsm->htoremove;

    while (htoremove->size > 0) {
      hash_t *hsh = (hash_t*)list_rem_head(htoremove);
      hash_free(hsh);
    }
  }

  /* Create and launch a preload task. */

  fsm_launch_preload(fsm);
  
  /* Create and launch a brute force task. */
  
  if (fsm->bforce) fsm_launch_bforce(fsm);
  
  fsm->state = st_preload;
}
/*-------------------------------------------------------------------------*/
message_t *fsm_handle_preload(fsm_t *fsm, message_t *msg) {
  ophcrack_t *crack = fsm->crack;
  scheduler_t *sched = crack->sched;

  // If the preloading of the tables is finished, then we start
  // searching into the tables.

  list_t *tasks = list_alloc();

  if (msg->kind == msg_done) {
    msg_done_t *done = (msg_done_t*)msg->data;

    if (done->kind == preload_all) {
      assert(fsm->psize_curr <= fsm->psize_total);

      // Check that the tables have been correctly preloaded.

      // if (fsm->psize_curr < fsm->psize_total)
      //   ophcrack_preload_check(crack);

      // This is necessary to prevent the status bar from indicating
      // that a preload task is 'waiting' if no preloading occurred.

      fsm->psize_curr  = 1;
      fsm->psize_total = 1;

      // Start the work tasks.

      pthread_mutex_lock(sched->mutex);

      int nthreads = sched->nthreads;
      int ntasks = MY_MAX(1, nthreads-1);

      for (int n=0; n<ntasks; ++n) {
        ophtask_t *task = ophcrack_next(crack);

        if (task)
          list_add_tail(tasks, task);
        else
          break;
      }

      pthread_mutex_unlock(sched->mutex);

      /* ... */

      ntasks = tasks->size;

      for (int i=0; i<ntasks; ++i) {
        ophtask_t *task = (ophtask_t*)list_rem_head(tasks);
        scheduler_add(sched, task, low);
      }

      assert(tasks->size == 0);

      // If no tasks have been scheduled, then we schedule a fake task
      // which will possibly trigger the 'done(all)' message if it is
      // the last one to be executed. This trick is required since the
      // brute force might still be running, therefore we should not
      // stop here, but wait until the 'done(all)' message is
      // received.

      if (ntasks == 0) {
        ophtask_t *task = ophtask_alloc(find);
        scheduler_add(sched, task, low);
        message_done(find);
      }

      fsm->state = st_work1;
    }
  }

  list_free(tasks);

  return msg;
}
/*-------------------------------------------------------------------------*/
message_t *fsm_handle_pause1(fsm_t *fsm, message_t *msg) {
  list_t *pending_msg = fsm->pending_msg;

  // A thread has entered into pause mode.

  if (msg->kind == msg_pause) {
    --fsm->pwait;

#ifndef NDEBUG
    int pwait = fsm->pwait;
    int nthreads = fsm->crack->sched->nthreads;

    assert(pwait >= 0 && pwait <= nthreads);
#endif
  }

  // The other messages are accumulated.
  
  else
    list_add_tail(pending_msg, msg);

  return 0;
}
/*-------------------------------------------------------------------------*/
message_t *fsm_handle_pause2(fsm_t *fsm, message_t *msg) {
  ophcrack_t *crack = fsm->crack;
  scheduler_t *sched = crack->sched;
  list_t *pending_msg = fsm->pending_msg;

  if (msg) {
    // A thread has entered into pause mode.
    
    if (msg->kind == msg_pause) {
      --fsm->pwait;

#ifndef NDEBUG
      int pwait = fsm->pwait;
      int nthreads = crack->sched->nthreads;

      assert(pwait >= 0 && pwait <= nthreads);
#endif
    }
    
    // The other messages are accumulated.
    
    else {
      list_add_tail(pending_msg, msg);
      msg = 0;
    }
  }

  // If all threads are in pause mode, then we can continue.

  if (fsm->pwait == 0) {
    fsm_reset(fsm);

    // If the brute force has been enabled, then we schedule a brute
    // force task. But we do not schedule a brute force task if a
    // preload will occur because it is better if the brute force is
    // started after the preload.

    if (fsm->bforce && fsm->preload == 0) fsm_launch_bforce(fsm);

    // Ask the scheduler to resume its activity. We also put back the
    // pending messages into the message queue so that they can be
    // processed normally.

    scheduler_continue(sched);
    message_insert_first(fsm->pending_msg);

    // We go back to the state we were in before the pause. If the
    // state has been reset to 'start', then we directly jump there.

    fsm->state = fsm->preload ? st_start : fsm->oldstate;

    if (fsm->state == st_start) {
      assert(fsm->preload != 0);
      fsm_handle_start(fsm);
    }
  }

  return msg;
}
/*-------------------------------------------------------------------------*/
message_t *fsm_handle_work1(fsm_t *fsm, message_t *msg) {
  if (msg->kind != msg_done) return msg;

  msg_done_t *done = (msg_done_t*)msg->data;

  if (done->kind == find)
    fsm->state = st_work2;

  return msg;
}
/*-------------------------------------------------------------------------*/
message_t *fsm_handle_work2(fsm_t *fsm, message_t *msg) {
  ophcrack_t *crack = fsm->crack;
  scheduler_t *sched = crack->sched;
  arg_t *arg = fsm->arg;

  if (msg->kind != msg_done) return msg;

  msg_done_t *done = (msg_done_t*)msg->data;

  if (done->kind == all) {
    // If there are still some hashes to crack and some tables remain,
    // then we continue.

    int npwds_total = crack->npwds_total;
    int npwds_found = crack->npwds_found;

    if (crack->remaining->size > 0 && npwds_found < npwds_total) {
      ophtask_t *task = ophtask_alloc(preload_all);
      scheduler_add(sched, task, disk);

      fsm->state = st_preload;
    }

    // Otherwise, we stop.

    else {
      if (arg->bforce) {
	fsm->bforce_curr  = 1;
	fsm->bforce_total = 1;
      }

      fsm->state = st_wait;
    }
  } 
  
  return msg;
}
/*-------------------------------------------------------------------------*/
void fsm_ssave(fsm_t *fsm) {
  fsm_pause(fsm);

  fsm->ssave = 1;
  fsm->state = st_pause2;
}
/*-------------------------------------------------------------------------*/
void fsm_pause(fsm_t *fsm) {
  ophcrack_t *crack = fsm->crack;
  scheduler_t *sched = crack->sched;

  scheduler_pause(sched);

  fsm->pwait = sched->nthreads;
  fsm->oldstate = fsm->state;
  fsm->state = st_pause1;
}
/*-------------------------------------------------------------------------*/
void fsm_reset_preload(fsm_t *fsm) {
  fsm->preload = 1;
  fsm->psize_curr = 1;
  fsm->psize_total = 0;
}
/*-------------------------------------------------------------------------*/
void fsm_reset_bforce(fsm_t *fsm) {
  fsm->bforce = fsm->arg->bforce;
  fsm->bforce_curr = 1;
  fsm->bforce_total = 0;
}
/*-------------------------------------------------------------------------*/
void fsm_launch_preload(fsm_t *fsm) {
  ophcrack_t *crack = fsm->crack;
  scheduler_t *sched = crack->sched;

  ophtask_t *task = ophtask_alloc(preload_all);
  ophload_t *load = (ophload_t*)task->data;

  list_t *enabled = crack->enabled;
  list_nd_t *tnd  = 0;

  for (tnd = enabled->head; tnd != 0; tnd = tnd->next)
    list_add_tail(load->tables, tnd->data);
  
  scheduler_add(sched, task, disk);
  fsm->preload = 0;
}
/*-------------------------------------------------------------------------*/
void fsm_launch_bforce(fsm_t *fsm) {
  ophcrack_t *crack = fsm->crack;
  scheduler_t *sched = crack->sched;

  ophtask_t *task = ophtask_alloc(bforce_all);
  ophbforce_t *force = (ophbforce_t*)task->data;

  list_t *hashes = crack->hashes;
  list_nd_t *nd;

  int nhashes = hashes->size;
  int n;

  force->nhashes = nhashes;
  force->hashes  = (hash_t**)malloc(nhashes*sizeof(hash_t*));
  
  for (nd=hashes->head, n=0; nd!=0; nd=nd->next, ++n)
    force->hashes[n] = (hash_t*)nd->data;

  scheduler_add(sched, task, low);
  fsm->bforce = 0;
}
/*-------------------------------------------------------------------------*/
void fsm_reset(fsm_t *fsm) {
  assert(fsm->state == st_wait || fsm->state == st_pause2);

  static const int bforce_mask = bforce_all + bforce_nt + bforce_lm;

  ophcrack_t *crack = fsm->crack;
  arg_t *arg = crack->arg;

  // If the saving of the session has been requested, we do it.

  if (arg->ssave && fsm->ssave) {
    FILE *file = fopen(arg->sfname, "w");

    if (file) {
      ophcrack_reset(crack, all - bforce_mask, 0);
      ophcrack_save(crack, file, 0, 1);
      fclose(file);
    }

    fsm->ssave = 0;
  }

  // If a preload will be performed, then we remove all non-brute
  // force tasks from the scheduler.
  
  if (fsm->preload)
    ophcrack_reset(crack, all - bforce_mask, 1);
  
  // If a brute force is scheduled or if it has been disabled, then
  // we remove all the brute force tasks from the scheduler.
  
  if (fsm->bforce || arg->bforce == 0)
    ophcrack_reset(crack, bforce_mask, 1);
}
