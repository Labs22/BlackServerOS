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
#ifndef SCHEDULER_H
#define SCHEDULER_H

#define NQUEUES 3

#include <pthread.h>
#include <semaphore.h>

#include "misc.h"
#include "list.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef enum { low=0, disk, high } prty_t;

typedef struct scheduler_t_ {
  int nthreads;               /**< Number of threads. */
  int ntasks;                 /**< Number of executable tasks pending. */
  int pause;
  int disk;                   /**< Indicates whether the disk is being
				   accessed by a thread */
  list_t *queue[NQUEUES];     /**< The queues. */
  sem_t *sem;                 /**< Semaphore for controlling the
				   access to the queues . */
  pthread_cond_t *cond;
  pthread_mutex_t *mutex;     /**< Mutex for blocking the access to
				   the shared ressources.  */
  pthread_t *thread;          /**< The threads. */
} scheduler_t;

scheduler_t *scheduler_alloc(int nthreads);
void scheduler_free(scheduler_t *sched);

/** Start the scheduler.
 *
 *  Create the requested number of threads.
 *
 *  @param sched A pointer to as scheduler structure.
 *  @param fun   A pointer to the function to be exectued by the threads.
 *  @param arg   A pointer to the arguments to be send to the threads.
 *  @return The number of threads successfully spawn. An error occured
 *          if this is less than the number of threads requested.
 */

int scheduler_start(scheduler_t *sched, void *(*fun)(void*), void *arg);
void scheduler_pause(scheduler_t *sched);
void scheduler_continue(scheduler_t *sched);
void scheduler_wait(scheduler_t *sched);
void scheduler_stop(scheduler_t *sched);

void scheduler_add(scheduler_t *sched, void *data, prty_t prty);
void *scheduler_get(scheduler_t *sched, int id);
int scheduler_done(scheduler_t *sched, int id);

#ifdef __cplusplus
}
#endif
#endif
