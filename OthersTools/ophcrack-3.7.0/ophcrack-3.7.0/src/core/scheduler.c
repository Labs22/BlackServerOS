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
#include <stdio.h>
#include <assert.h>

#include "scheduler.h"
#include "message.h"
/*-------------------------------------------------------------------------*/
scheduler_t *scheduler_alloc(int nthreads) {
  scheduler_t *sched = (scheduler_t*)malloc(sizeof(scheduler_t));

  sched->nthreads = nthreads;
  sched->ntasks = 0;
  sched->pause = 0;
  sched->disk = -1;

  sched->queue[low]  = list_alloc();
  sched->queue[disk] = list_alloc();
  sched->queue[high] = list_alloc();

  /* Create and initialise the semaphore. */

#ifndef OSX
  sched->sem = (sem_t*)malloc(sizeof(sem_t));

  if (sem_init(sched->sem, 0, 0) != 0) {
    fprintf(stderr, "scheduler_alloc: cannot initialise the semaphore\n");
    exit(1);
  }
#else
  sem_unlink("ophcrack_scheduler");
  sched->sem = sem_open("ophcrack_scheduler", O_CREAT | O_EXCL, 0, 0);

  if (sched->sem == SEM_FAILED) {
    fprintf(stderr, "scheduler_alloc: cannot initialise the semaphore\n");
    exit(1);
  }
#endif

  /* Create and initialise the mutex and conditional variable, and
     allocate space to store the threads. */

  sched->mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(sched->mutex, 0);

  sched->cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
  pthread_cond_init(sched->cond, 0);

  sched->thread = (pthread_t*)malloc(nthreads*sizeof(pthread_t));

  return sched;
}
/*-------------------------------------------------------------------------*/
void scheduler_free(scheduler_t *sched) {
  list_free(sched->queue[disk]);
  list_free(sched->queue[high]);
  list_free(sched->queue[low]);

  sem_destroy(sched->sem);
  pthread_mutex_destroy(sched->mutex);
  pthread_cond_destroy(sched->cond);

  free(sched->sem);  
  free(sched->mutex);
  free(sched->cond);

  free(sched->thread);
  free(sched);
}
/*-------------------------------------------------------------------------*/
int scheduler_start(scheduler_t *sched, void *(*fun)(void*), void *arg) {
  int i, j;
  int nthreads = sched->nthreads;
  pthread_t *thread = sched->thread;

  for (i=0; i<nthreads; ++i)
    if (pthread_create(thread+i, 0, fun, arg) != 0) break;

  if (i < nthreads)
    for (j=0; j<i; ++j) pthread_cancel(thread[j]);

  return i;
}
/*-------------------------------------------------------------------------*/
void scheduler_pause(scheduler_t *sched) {
  pthread_mutex_t *mutex = sched->mutex;
  int i;

  pthread_mutex_lock(mutex);
  sched->pause = 1;
  pthread_mutex_unlock(mutex);

  // Increment the semaphore by the number of threads which could
  // potentially be waiting on it. This is necessary to avoid having
  // threads waiting on the semaphore while in pause.

  for (i=0; i<sched->nthreads; ++i)
    sem_post(sched->sem);
}
/*-------------------------------------------------------------------------*/
void scheduler_continue(scheduler_t *sched) {
  pthread_mutex_t *mutex = sched->mutex;
  pthread_cond_t *cond = sched->cond;

  pthread_mutex_lock(mutex);
  sched->pause = 0;
  pthread_cond_broadcast(cond);
  pthread_mutex_unlock(mutex);
}
/*-------------------------------------------------------------------------*/
void scheduler_wait(scheduler_t *sched) {
  int i;
  int nthreads = sched->nthreads;
  pthread_t *thread = sched->thread;

  for (i=0; i<nthreads; ++i) pthread_join(thread[i], 0);
}
/*-------------------------------------------------------------------------*/
void scheduler_stop(scheduler_t *sched) {
  int i;
  int nthreads = sched->nthreads;
  pthread_t *thread = sched->thread;

  for (i=0; i<nthreads; ++i) pthread_cancel(thread[i]);
}
/*-------------------------------------------------------------------------*/
void scheduler_add(scheduler_t *sched, void *task, prty_t prty) {
  pthread_mutex_t *mutex = sched->mutex;
  list_t *queue = sched->queue[prty];

  /* Add the task to the appropriate queue. */
  
  pthread_mutex_lock(mutex);

  list_add_tail(queue, task);
  ++sched->ntasks;

  /* If the priorty is not disk, then the task is exectuable. We
     therefore increase the semaphore to reflect that. */

  if (prty != disk)
    sem_post(sched->sem);

  /* If somebody is acessing the disk at the moment, then the newly
     added disk task is not executable and therefore we do not
     increase the semaphore. Otherwise, we increase the semaphore only
     if the size of the disk queue is equal to one. */

  else if (sched->disk == -1 && queue->size == 1)
    sem_post(sched->sem);

  pthread_mutex_unlock(mutex);
}
/*-------------------------------------------------------------------------*/
void *scheduler_get(scheduler_t *sched, int id) {
  void *task = 0;
  pthread_mutex_t *mutex = sched->mutex;
  pthread_cond_t *cond = sched->cond;

  list_t **queue = sched->queue;

  /* We wait until an executable task is available. Although the
     semaphore might suggest that there are still some tasks in the
     queues, there might be actually nothing left. This might happen
     if, for example, during a pause, the scheduler has been
     reset. We therefore add a while in order to guaranty that when
     the function exits, a task is returned. */

  while (task == 0) {
    sem_wait(sched->sem);
    pthread_mutex_lock(mutex);

    /* If we are in pause mode, then we wait until the conditional
       variable is signaled. */

    if (sched->pause) {
      message_pause();
      pthread_cond_wait(cond, mutex);
    }
    
    /* Pick a task from the high priority queue if it is not empty. */
    
    if (queue[high]->size > 0)
      task = list_rem_head(queue[high]);
    
    /* Otherwise, if the disk queue is not empty and nobody is
       accessing the disk at the moment, then pick a task from the
       disk queue. */
    
    else if (queue[disk]->size > 0 && sched->disk == -1) {
      sched->disk = id;
      task = list_rem_head(queue[disk]);
    }      
    
    /* Finally, if none of the above cases where taken, then pick a
       task from the low priority queue. */
    
    else
      task = list_rem_head(queue[low]);
        
    pthread_mutex_unlock(mutex);
  }

  return task;
}
/*-------------------------------------------------------------------------*/
int scheduler_done(scheduler_t *sched, int id) {
  int ntasks;  
  list_t **queue = sched->queue;
  pthread_mutex_t *mutex = sched->mutex;

  /* Decrease the task counter. */

  pthread_mutex_lock(mutex);
  ntasks = --sched->ntasks;

  /* If the calling thread was dealing with the disk, then this means
     that a disk task can now be executed. We therefore increase the
     semaphore only if the disk queue is not empty. */

  if (sched->disk == id) {
    sched->disk = -1;

    if (queue[disk]->size > 0)
      sem_post(sched->sem);
  }

  pthread_mutex_unlock(mutex);

  return ntasks;
}
