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
#include <pthread.h>
#include <string.h>
#include <semaphore.h>

#include "message.h"
/*-------------------------------------------------------------------------*/
pthread_mutex_t *msg_mutex = 0;
sem_t *msg_sem = 0;

list_t *msg_queue;
struct timeval msg_time;
/*-------------------------------------------------------------------------*/
void message_init(void) {
  msg_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(msg_mutex, 0);

#ifndef OSX
  msg_sem = (sem_t*)malloc(sizeof(sem_t));
  
  if (sem_init(msg_sem, 0, 0) != 0) {
    fprintf(stderr, "message_init: cannot initialise the semaphore\n");
    exit(1);
  }
#else
  sem_unlink("ophcrack_message");
  msg_sem = sem_open("ophcrack_message", O_CREAT | O_EXCL, 0, 0);
  
  if (msg_sem == SEM_FAILED) {
    fprintf(stderr, "message_init: cannot initialise the semaphore\n");
    exit(1);
  }
#endif

  msg_queue = list_alloc();
  gettimeofday(&msg_time, 0);
}
/*-------------------------------------------------------------------------*/
message_t *message_alloc(msg_kind_t kind) {
  static uint32_t gid = 0;

  message_t *msg = (message_t*)malloc(sizeof(message_t));
  msg->kind = kind;
  msg->data = 0;
  msg->id   = gid++;
  
  gettimeofday(&msg->time, 0);
  msg->time.tv_sec -= msg_time.tv_sec;

  return msg;
}
/*-------------------------------------------------------------------------*/
void message_free(message_t *msg) {
  switch (msg->kind) {
  case msg_done: {
    msg_done_t *done = msg->data;
    free(done);
  } break;
    
  case msg_preload:
  case msg_unload: {
    msg_load_t *load = msg->data;
    free(load);
  } break;

  case msg_work: {
    msg_work_t *work = msg->data;
    free(work);
  } break;
    
  case msg_found: {
    msg_found_t *found = msg->data;
    free(found);
  } break;

  case msg_bforce: {
    msg_bforce_t *force = msg->data;
    free(force);
  } break;

  case msg_pause:
    break;
  }

  free(msg);
}
/*-------------------------------------------------------------------------*/
void message_add(message_t *msg) {
  pthread_mutex_lock(msg_mutex);
  list_add_tail(msg_queue, msg);

  sem_post(msg_sem);
  pthread_mutex_unlock(msg_mutex);
}
/*-------------------------------------------------------------------------*/
void message_insert_first(list_t *messages) {
  pthread_mutex_lock(msg_mutex);

  while (messages->size > 0) {
    message_t *msg = list_rem_tail(messages);

    list_add_head(msg_queue, msg);
    sem_post(msg_sem);
  }

  pthread_mutex_unlock(msg_mutex);
}
/*-------------------------------------------------------------------------*/
message_t *message_get(void) {
  sem_wait(msg_sem);

  pthread_mutex_lock(msg_mutex);
  message_t *msg = list_rem_head(msg_queue);
  pthread_mutex_unlock(msg_mutex);

  return msg;
}
/*-------------------------------------------------------------------------*/
message_t *message_tryget(void) {
  int ret = sem_trywait(msg_sem);
  message_t *msg = 0;

  if (ret == 0) {
    pthread_mutex_lock(msg_mutex);
    msg = list_rem_head(msg_queue);
    pthread_mutex_unlock(msg_mutex);
  }

  return msg;
}
/*-------------------------------------------------------------------------*/
void message_done(ophkind_t kind) {
  message_t *msg = message_alloc(msg_done);
  msg_done_t *done = (msg_done_t*)malloc(sizeof(msg_done_t));

  done->kind = kind;

  msg->data = done;
  message_add(msg);
}
/*-------------------------------------------------------------------------*/
void message_preload(table_t *tbl, table_preload_t preload, int done, uint64_t size) {
  message_t *msg = message_alloc(msg_preload);
  msg_load_t *load = (msg_load_t*)malloc(sizeof(msg_load_t));

  load->tbl = tbl;
  load->preload = preload;
  load->done = done;
  load->size = size;

  msg->data = load;
  message_add(msg);
}
/*-------------------------------------------------------------------------*/
void message_unload(table_t *tbl, uint64_t size) {
  message_t *msg = message_alloc(msg_unload);
  msg_load_t *load = (msg_load_t*)malloc(sizeof(msg_load_t));

  load->tbl = tbl;
  load->preload = preload_none;
  load->done = 1;
  load->size = size;
  
  msg->data = load;
  message_add(msg);
}
/*-------------------------------------------------------------------------*/
void message_work(hash_t *hsh, table_t *tbl, ophkind_t kind, int cmin, int cmax) {
  message_t *msg = message_alloc(msg_work);
  msg_work_t *work = (msg_work_t*)malloc(sizeof(msg_work_t));

  work->hsh   = hsh;
  work->tbl   = tbl;
  work->kind  = kind;
  work->cmin  = cmin;
  work->cmax  = cmax;

  msg->data = work;
  message_add(msg);
}
/*-------------------------------------------------------------------------*/
void message_found(hash_t *hsh, table_t *tbl, int col) {
  message_t *msg = message_alloc(msg_found);
  msg_found_t *found = (msg_found_t*)malloc(sizeof(msg_found_t));

  found->hsh = hsh;
  found->tbl = tbl;
  found->col = col;

  msg->data = found;
  message_add(msg);
}
/*-------------------------------------------------------------------------*/
void message_bforce(int done, uint64_t count) {
  message_t *msg = message_alloc(msg_bforce);
  msg_bforce_t *force = (msg_bforce_t*)malloc(sizeof(msg_bforce_t));

  force->done  = done;
  force->count = count;
  
  msg->data = force;
  message_add(msg);
}
/*-------------------------------------------------------------------------*/
void message_pause(void) {
  message_t *msg = message_alloc(msg_pause);
  message_add(msg);
}
