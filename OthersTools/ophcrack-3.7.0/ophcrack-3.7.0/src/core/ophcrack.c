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
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#include "ophcrack.h"
#include "misc.h"
#include "table.h"
#include "hash.h"
#include "list.h"
#include "original.h"
#include "lmtable.h"
#include "lmgerman.h"
#include "ntextended.h"
#include "ntdict.h"
#include "ntnine.h"
#include "nteight.h"
#include "ntnum.h"
#include "ntseven.h"
#include "lmflash.h"
#include "nteightxl.h"
#include "ntspecialxl.h"
#include "ntproba.h"
#include "message.h"
/*-------------------------------------------------------------------------*/
ophcrack_t *ophcrack_alloc(int nthreads, arg_t *arg) {
  ophcrack_t *crack = (ophcrack_t*)malloc(sizeof(ophcrack_t));

  crack->sched = scheduler_alloc(nthreads);
  crack->tables = list_alloc();

  crack->hashes = list_alloc();
  crack->active = list_alloc();
  crack->enabled = list_alloc();
  crack->remaining = list_alloc();
  crack->hnd = 0;
  crack->stat = ophstat_alloc();
  crack->maxtid = 0;
  crack->maxhid = 0;
  crack->maxtkind = unknown;
  crack->npwds_total = 0;
  crack->npwds_found = 0;
  crack->find_freeram = find_freeram;
  crack->freeram = 0;
  crack->arg = arg;
  
  return crack;
}
/*-------------------------------------------------------------------------*/
void ophcrack_free(ophcrack_t *crack) {
  scheduler_free(crack->sched);

  list_free(crack->tables);
  list_free(crack->hashes);
  list_free(crack->active);
  list_free(crack->enabled);
  list_free(crack->remaining);

  ophstat_free(crack->stat);

  free(crack);
}
/*-------------------------------------------------------------------------*/
void ophcrack_start(ophcrack_t *crack) {
  crack->freeram = crack->find_freeram();
  scheduler_start(crack->sched, ophcrack_run, crack);
}
/*-------------------------------------------------------------------------*/
void ophcrack_stop(ophcrack_t *crack) {
  scheduler_stop(crack->sched);
  scheduler_wait(crack->sched);
}
/*-------------------------------------------------------------------------*/
int ophcrack_setup_table(table_t *tbl) {

  /* Setup the table. We set the kind variable ourselves and
     then call the appropriate setup function. */
  
  tbl->kind = table_kind(tbl->code);
  int ret = 0;

  switch (tbl->kind) {
  case lmalphanum10k:
    ret = lmtable_setup(tbl);
    break;
    
  case lmalphanum5k:
    ret = lmtable_setup(tbl);
    break;
    
  case lmextended:
    ret = lmtable_setup(tbl);
    break;
    
  case lmgermanv1:
    ret = lmgerman_setup(tbl);
    break;
    
  case lmgermanv2:
    ret = lmgerman_setup(tbl);
    break;
    
  case ntextended:
    ret = ntextended_setup(tbl);
    break;

  case ntdict:
    ret = ntdict_setup(tbl);
    break;

  case ntnine:
    ret = ntnine_setup(tbl);
    break;

  case nteight:
    ret = nteight_setup(tbl);
    break;

  case ntnum:
    ret = ntnum_setup(tbl);
    break;

  case ntseven:
    ret = ntseven_setup(tbl);
    break;

  case lmflash:
    ret = lmflash_setup(tbl);
    break;

  case nteightxl:
    ret = nteightxl_setup(tbl);
    break;

  case ntspecialxl:
    ret = ntspecialxl_setup(tbl);
    break;

  case ntprobafree:
    ret = ntproba_setup(tbl);
    break;

  case ntproba10g:
    ret = ntproba_setup(tbl);
    break;

  case ntproba60g:
    ret = ntproba_setup(tbl);
    break;

  case unknown:
    ret = -1;
    break;
  }

  return ret;
}
/*-------------------------------------------------------------------------*/
void ophcrack_add_table(ophcrack_t *crack, table_t *tbl) {
  scheduler_t *sched = crack->sched;
  pthread_mutex_t *mutex = sched->mutex;

  crack->maxtkind = MY_MAX(crack->maxtkind, tbl->kind+1);

  tbl->id   = crack->maxtid++;
  tbl->cmin = tbl->ncols;
  tbl->cmax = tbl->ncols;

  pthread_mutex_lock(mutex);
  list_add_tail(crack->tables, tbl);
  pthread_mutex_unlock(mutex);
}
/*-------------------------------------------------------------------------*/
void ophcrack_add_hash(ophcrack_t *crack, hash_t *hsh) {
  scheduler_t *sched = crack->sched;
  pthread_mutex_t *mutex = sched->mutex;

  pthread_mutex_lock(mutex);

  /* Add the hash to the list of hash we want to crack and update the
     hash id. */

  list_add_tail(crack->hashes, hsh);
  crack->maxhid = MY_MAX(crack->maxhid, hsh->id+1);

  /* Since we consider a group of LM and NT hashes as a single
     password, we only increase the password counter if the hash is the
     1st LM hash or if the hash is not connected to any other hash. */

  if (hsh->kind == lm1)
    ++crack->npwds_total;

  else if (hsh->lmhsh1 == 0 && hsh->lmhsh2 == 0)
    ++crack->npwds_total;

  pthread_mutex_unlock(mutex);

  /* If the hash has already been cracked, then we mark it as not done
     so that we go through the usual procedure which is carried out
     when a password is found. */

  if (hsh->done > 0) {
    hsh->done = 0;

    /* To avoid counting many times the same password as found, we
       temporary set the done variable of the connected LM and/or NT
       hash to zero. But we do this only if we are not dealing with a
       1st LM hash because we want to count the password at least
       once. */

    if (hsh->kind != lm1 && hsh->lmhsh1) {
      int tmp = hsh->lmhsh1->done;

      hsh->lmhsh1->done = 0;
      ophcrack_found(crack, hsh, 0, 0, hsh->pwd);
      hsh->lmhsh1->done = tmp;
    }

    /* If we have a 1st LM hash or a hash which is not connected to
       another hash, then we can safely call the _found function. */

    else
      ophcrack_found(crack, hsh, 0, 0, hsh->pwd);
  }

  else if (hsh->done < 0) {
    hsh->done = 0;
    ophcrack_notfound(crack, hsh);
  }
}
/*-------------------------------------------------------------------------*/
void ophcrack_associate(ophcrack_t *crack, hash_t *hsh, table_t *tbl) {
  scheduler_t *sched = crack->sched;
  pthread_mutex_t *mutex = sched->mutex;

  pthread_mutex_lock(mutex);
  if (tbl->isvalid(hsh, tbl)) hash_add_table(hsh, tbl);
  pthread_mutex_unlock(mutex);
}
/*-------------------------------------------------------------------------*/
void ophcrack_reset(ophcrack_t *crack, int kind_mask, int remove) {
  static int work_mask = find | lookup_idx | lookup_end | lookup_srt | check;

  scheduler_t *sched = crack->sched;
  pthread_mutex_t *mutex = sched->mutex;

  ophtask_t *task = 0;
  int i;

  pthread_mutex_lock(mutex);

  for (i=0; i<NQUEUES; ++i) {
    list_t *queue = sched->queue[i];
    int size = queue->size;
    int j;
    
    for (j=0; j<size; ++j) {
      task = list_rem_head(queue);
      
      if (task->kind & work_mask) {
	ophwork_t *work = task->data;
	htbl_t *htbl = work->htbl;

	if (htbl)
	  htbl->covered = MY_MAX(htbl->covered, work->cmax+1);
      }

      if (remove && task->kind & kind_mask) {
	ophtask_free(task);
	--sched->ntasks;
      } else
	list_add_tail(queue, task);
    }
  }
  
  pthread_mutex_unlock(mutex);
}
/*-------------------------------------------------------------------------*/
void *ophcrack_run(void *arg) {
  static int gid  = 0;
  static int wait = 0;

  int myid = gid++;

  ophcrack_t *crack  = (ophcrack_t*)arg;
  scheduler_t *sched = crack->sched;
  pthread_mutex_t *mutex = sched->mutex;

  /* Make sure we accept cancel requests in a deferred fashion. */

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, 0);

  /* Wait and process. */

  list_t *tasks = list_alloc();

  while (1) {
    ophtask_t *task = scheduler_get(sched, myid);
    ophkind_t kind  = task->kind;

    /* Tables preloading. */

    if (kind == preload_all)
      task = ophcrack_preload_all(crack, task, myid);

    else if (kind == preload_one)
      task = ophcrack_preload_one(crack, task, myid);

    /* Tables unloading. */

    else if (kind == unload)
      task = ophcrack_unload(crack, task, myid);

    /* Brute force. */

    else if (kind == bforce_all ||
             kind == bforce_nt  ||
             kind == bforce_lm  ||
             kind == resolve_nt) {

      ophbforce_t *force = task->data;

      /* Check whether all the hashes have been cracked. */

      int nhashes = force->nhashes;
      int n = 0;

      if (force->hashes)
        for (n=0; n<nhashes; ++n)
          if (force->hashes[n]->done == 0) break;

      /* If not all hashes have been cracked, then it is worth continuing the
         brute force. */

      if (n < nhashes)
        switch (kind) {
        case bforce_all:
          task = ophcrack_bforce_all(crack, task, myid);
          break;

        case bforce_nt:
          task = ophcrack_bforce_nt(crack, task, myid);
          break;

        case bforce_lm:
          task = ophcrack_bforce_lm(crack, task, myid);
          break;

        case resolve_nt:
          task = ophcrack_resolve_nt(crack, task, myid);
          break;

        default:
          break;
        }
    }

    /* Find, lookup, check.  */

    else {
      ophwork_t *work = task->data;

      if (work->hsh && work->hsh->done == 0) {

        /* If the password has not been found yet, then we proceed with the
           task. */

        switch (kind) {
        case find:
          task = ophcrack_find(crack, task, myid);
          break;

        case lookup_idx:
          task = ophcrack_lookup_idx(crack, task, myid);
          break;

        case lookup_end:
          task = ophcrack_lookup_end(crack, task, myid);
          break;

        case lookup_srt:
          task = ophcrack_lookup_srt(crack, task, myid);
          break;

        case check:
          task = ophcrack_check(crack, task, myid);
          break;

        default:
          break;
        }

        /* Store the mimimum column index which we have considered so far for
           the given hash and the given table. */

        if (task) {
          htbl_t *htbl = work->htbl;
          int cmin = work->cmin;

          pthread_mutex_lock(mutex);

          if (work->hsh->done != 0)
            htbl->covered = 0;
          else
            htbl->covered = MY_MIN(htbl->covered, cmin);

          pthread_mutex_unlock(mutex);
        }
      }
    }

    /* Beginning of mutexed region. */

    pthread_mutex_lock(mutex);

    /* Collect the statistics and delete the task. When the task we delete is a
       brute force task then we also update the status of the brute force so
       that things are displayed correctly. */

    if (task) {
      ophstat_add(crack->stat, task->stat);

      if (task->kind & (bforce_lm + bforce_nt)) {
        ophbforce_t *force = task->data;
        uint64_t count = force->count;

        message_bforce(1, count);
      }

      ophtask_free(task);
    }

    /* Check whether we can schedule other find tasks. */

    int disk_size = sched->queue[disk]->size;
    int mdqueue = crack->arg->mdqueue;
    int ntasks = kind == find ? 1 : 0;

    /* If we have been waiting for the disk queue to become smaller and it is
       the case, then we schedule the find tasks that we have not been able to
       schedule before. */

    if (wait > 0 && disk_size <= mdqueue / 2) {
      ntasks += wait;
      wait = 0;
    }

    /* Allocate the requested number of find tasks. If a task cannot be
       allocated, we will have to do it later, presumably when the size of the
       disk queue becomes acceptable again. */

    for (int i=0; i<ntasks; ++i) {
      ophtask_t *task = ophcrack_next(crack);

      if (task)
        list_add_tail(tasks, task);
      else
        ++wait;
    }

    /* End of mutexed region. */

    pthread_mutex_unlock(mutex);

    /* ... */

    ntasks = tasks->size;

    for (int i=0; i<ntasks; ++i) {
      ophtask_t *task = (ophtask_t*)list_rem_head(tasks);
      scheduler_add(sched, task, low);
    }

    assert(tasks->size == 0);

    /* ... */

    if (kind == find && ntasks == 0 && wait == 0)
      message_done(find);

    /* Tell the scheduler we are done with this task and send a 'done' message
       if I am the last one to finish my job. */

    ntasks = scheduler_done(sched, myid);
    if (ntasks == 0) message_done(all);
  }

  list_free(tasks);

  return 0;
}
/*-------------------------------------------------------------------------*/
ophtask_t *ophcrack_find(ophcrack_t *crack, ophtask_t *task, int id) {
  scheduler_t *sched = crack->sched;
  pthread_mutex_t *mutex = sched->mutex;

  ophwork_t *work = task->data;

  hash_t *hsh  = work->hsh;
  table_t *tbl = work->htbl->tbl;

  int cmin = work->cmin;
  int cmax = work->cmax;
  int size = cmax-cmin+1;

  arg_t *arg = crack->arg;

  if (arg->verbose) 
    message_work(hsh, tbl, find, cmin, cmax);
  
  /* Convert the task to a lookup_idx task. */

  ophtask_convert(task, find, lookup_idx);

  /* Try to find the pwd by starting from each column. */

  int i;
  ophel_t **tosearch = work->tosearch;

  for (i=0; i<size; ++i) {
    if (arg->debug) {
      pthread_mutex_lock(mutex);
      
      int hid = hsh->id;
      int idx = tbl->idx;
      const char *name = tbl->name;

      fprintf(arg->lfile, "find: ");

      switch (hsh->kind) {
      case lm1: fprintf(arg->lfile, "1st LM hash "); break;
      case lm2: fprintf(arg->lfile, "2nd LM hash "); break;
      case nt:  fprintf(arg->lfile, "NT hash "); break;
      }

      fprintf(arg->lfile, "#%d in table %s,%d ", hid, name, idx);
      fprintf(arg->lfile, "at column %d ... ", cmax-i);
    }

    assert(tbl->param != NULL);

    tosearch[i]->col = cmax-i;
    tbl->find(hsh, tbl, tosearch[i]);

    if (arg->debug) {
      uint32_t prefix  = tosearch[i]->prefix;
      uint16_t postfix = tosearch[i]->postfix;

      fprintf(arg->lfile, 
	      "prefix = %u, postfix = %u.\n", 
	      (unsigned int)prefix, 
	      (unsigned int)postfix);

      pthread_mutex_unlock(mutex);
    }
  }

  /* If the .index file has been preloaded, then we proceed with the
     corresponding lookup task. */

  if (tbl->idxmem)
    task = ophcrack_lookup_idx(crack, task, id);

  /* Otherwise, we add the lookup task to the scheduler. */

  else {
    scheduler_add(sched, task, disk);
    task = 0;
  }

  return task;
}
/*-------------------------------------------------------------------------*/
ophtask_t *ophcrack_lookup_idx(ophcrack_t *crack, ophtask_t *task, int id) {
  scheduler_t *sched = crack->sched;
  pthread_mutex_t *mutex = sched->mutex;

  ophwork_t *work = task->data;

  hash_t *hsh  = work->hsh;
  table_t *tbl = work->htbl->tbl;

  int cmin = work->cmin;
  int cmax = work->cmax;
  int size = cmax-cmin+1;

  arg_t *arg = crack->arg;
  
  if (arg->verbose) 
    message_work(hsh, tbl, lookup_idx, cmin, cmax);

  /* Convert the task to a lookup_end task. */

  task->kind = lookup_end;

  /* If we access the disk, then it is better to query the .index file
     by increasing order of the index. */

  ophel_t **tosearch = work->tosearch;

  if (!tbl->idxmem)
    qsort(tosearch, size, sizeof(ophel_t*), ophcrack_pwd_cmp);
  
  /* Find the lookup range of each prefix. */

  int i;

  for (i=0; i<size; ++i) {
    if (arg->debug) {
      pthread_mutex_lock(mutex);
      
      int hid = hsh->id;
      int idx = tbl->idx;
      const char *name = tbl->name;

      fprintf(arg->lfile, "lookup_idx: ");

      switch (hsh->kind) {
      case lm1: fprintf(arg->lfile, "1st LM hash "); break;
      case lm2: fprintf(arg->lfile, "2nd LM hash "); break;
      case nt:  fprintf(arg->lfile, "NT hash "); break;
      }

      uint32_t prefix  = tosearch[i]->prefix;
      uint16_t postfix = tosearch[i]->postfix;
      
      fprintf(arg->lfile, "#%d in table %s,%d ", hid, name, idx);
      fprintf(arg->lfile, "at column %d, ", cmax-i);
      fprintf(arg->lfile, "with prefix = %u ", prefix);
      fprintf(arg->lfile, "and postfix = %u ... ", (unsigned int)postfix);
    }

    assert(tbl->param != NULL);
    tbl->lookup_idx(hsh, tbl, tosearch[i]);

    if (arg->debug) {
      uint64_t low  = tosearch[i]->low;
      uint64_t high = tosearch[i]->high;

      fprintf(arg->lfile, "low = %llu, high = %llu.\n", (long long unsigned)low, (long long unsigned)high);
      pthread_mutex_unlock(mutex);
    }
  }

  /* If we have been searching in memory (on disk) and we can continue
     do so, we go ahead. */

  int both_in_mem  =  tbl->idxmem &&  tbl->endmem;
  int both_on_disk = !tbl->idxmem && !tbl->endmem;

  if (both_in_mem || both_on_disk)
    task = ophcrack_lookup_end(crack, task, id);

  /* Otherwise, we add a disk (memory) task to the scheduler. */

  else {
    prty_t priority = tbl->endmem ? low : disk;
    scheduler_add(sched, task, priority);
    task = 0;
  }

  return task;
}
/*-------------------------------------------------------------------------*/
ophtask_t *ophcrack_lookup_end(ophcrack_t *crack, ophtask_t *task, int id) {
  scheduler_t *sched = crack->sched;
  pthread_mutex_t *mutex = sched->mutex;

  ophwork_t *work = task->data;

  hash_t *hsh  = work->hsh;
  table_t *tbl = work->htbl->tbl;

  int cmin = work->cmin;
  int cmax = work->cmax;
  int size = cmax-cmin+1;

  arg_t *arg = crack->arg;

  if (arg->verbose) 
    message_work(hsh, tbl, lookup_end, cmin, cmax);

  /* Convert the task to a lookup_start task. */

  task->kind = lookup_srt;

  /* Find the chain ends. */

  int i;
  int count = size;
  ophel_t **tosearch = work->tosearch;

  for (i=0; i<size; ++i) {
    if (arg->debug) {
      pthread_mutex_lock(mutex);
      
      int hid = hsh->id;
      int idx = tbl->idx;
      const char *name = tbl->name;

      fprintf(arg->lfile, "lookup_end: ");

      switch (hsh->kind) {
      case lm1: fprintf(arg->lfile, "1st LM hash "); break;
      case lm2: fprintf(arg->lfile, "2nd LM hash "); break;
      case nt:  fprintf(arg->lfile, "NT hash "); break;
      }

      uint32_t prefix  = tosearch[i]->prefix;
      uint16_t postfix = tosearch[i]->postfix;
      uint64_t low  = tosearch[i]->low;
      uint64_t high = tosearch[i]->high;
      
      fprintf(arg->lfile, "#%d in table %s,%d ", hid, name, idx);
      fprintf(arg->lfile, "at column %d, ", cmax-i);
      fprintf(arg->lfile, "with prefix = %u, ", prefix);
      fprintf(arg->lfile, "postfix = %u, ", (unsigned int)postfix);
      fprintf(arg->lfile, "low = %llu and high = %llu ... ", (long long unsigned)low, (long long unsigned)high);
    }

    assert(tbl->param != NULL);
    int ret = tbl->lookup_end(hsh, tbl, tosearch[i]);

    /* Accumulate the statistics before freeing the element. */

    if (ret == 0) {
      ophstat_add(task->stat, tosearch[i]->stat);
      ophel_free(tosearch[i]);

      tosearch[i] = 0;
      --count;
    }

    if (arg->debug) {
      if (ret == 0)
	fprintf(arg->lfile, "no offset found.\n");
      else
	fprintf(arg->lfile, "offset = %llu.\n", (long long unsigned int)tosearch[i]->offset);

      pthread_mutex_unlock(mutex);
    }
  }

  /* If tosearch is not empty, then we sort the chain starts by
     increasing order of the column index. Otherwise we delete the
     task. */

  if (count > 0) 
      qsort(tosearch, size, sizeof(ophel_t*), ophcrack_col_cmp);
  else
    return task;

  /* If we have been searching in memory (on disk) and we can continue
     do so, we go ahead. */

  int both_in_mem  =  tbl->endmem &&  tbl->srtmem;
  int both_on_disk = !tbl->endmem && !tbl->srtmem;

  if (both_in_mem || both_on_disk)
    task = ophcrack_lookup_srt(crack, task, id);

  /* Otherwise, we add a disk (memory) task to the scheduler. */

  else {
    prty_t priority = tbl->srtmem ? low : disk;
    scheduler_add(sched, task, priority);
    task = 0;
  }

  return task;
}
/*-------------------------------------------------------------------------*/
ophtask_t *ophcrack_lookup_srt(ophcrack_t *crack, ophtask_t *task, int id) {
  scheduler_t *sched = crack->sched;
  pthread_mutex_t *mutex = sched->mutex;

  ophwork_t *work = task->data;

  hash_t *hsh  = work->hsh;
  table_t *tbl = work->htbl->tbl;

  int cmin = work->cmin;
  int cmax = work->cmax;
  int size = cmax-cmin+1;

  arg_t *arg = crack->arg;

  if (arg->verbose) 
    message_work(hsh, tbl, lookup_srt, cmin, cmax);

  /* Convert the task to a check task. */

  task->kind = check;

  /* Find the chain starts. */

  int i;
  ophel_t **tosearch = work->tosearch;

  for (i=0; i<size; ++i) {
    if (tosearch[i] == 0) break;

    if (arg->debug) {
      pthread_mutex_lock(mutex);
      
      int hid = hsh->id;
      int idx = tbl->idx;
      const char *name = tbl->name;

      fprintf(arg->lfile, "lookup_srt: ");

      switch (hsh->kind) {
      case lm1: fprintf(arg->lfile, "1st LM hash "); break;
      case lm2: fprintf(arg->lfile, "2nd LM hash "); break;
      case nt:  fprintf(arg->lfile, "NT hash "); break;
      }

      uint32_t prefix  = tosearch[i]->prefix;
      uint16_t postfix = tosearch[i]->postfix;
      uint64_t low  = tosearch[i]->low;
      uint64_t high = tosearch[i]->high;
      uint64_t offset = tosearch[i]->offset;
      
      fprintf(arg->lfile, "#%d in table %s,%d ", hid, name, idx);
      fprintf(arg->lfile, "at column %d, ", cmax-i);
      fprintf(arg->lfile, "with prefix = %u, ", prefix);
      fprintf(arg->lfile, "postfix = %u, ", (unsigned int)postfix);
      fprintf(arg->lfile, "low = %llu, high = %llu ", (long long unsigned)low, (long long unsigned)high);
      fprintf(arg->lfile, "and offset = %llu ... ", (long long unsigned int)offset);
    }
    
    assert(tbl->param != NULL);
    tbl->lookup_srt(hsh, tbl, tosearch[i]);

    if (arg->debug) {
      fprintf(arg->lfile, "start = ");
      fprintf_hex(arg->lfile, tosearch[i]->pwd, 16);
      fprintf(arg->lfile, "\n.");

      pthread_mutex_unlock(mutex);
    }
  }

  /* Add the task to the scheduler with a high priority. */

  scheduler_add(sched, task, high);

  return 0;
}
/*-------------------------------------------------------------------------*/
ophtask_t *ophcrack_check(ophcrack_t *crack, ophtask_t *task, int id) {
  scheduler_t *sched = crack->sched;
  pthread_mutex_t *mutex = sched->mutex;

  ophwork_t *work = task->data;

  hash_t *hsh  = work->hsh;
  htbl_t *htbl = work->htbl;
  table_t *tbl = htbl->tbl;

  int i;
  int cmin = work->cmin;
  int cmax = work->cmax;
  int size = cmax-cmin+1;

  arg_t *arg = crack->arg;

  if (arg->verbose > 0) 
    message_work(hsh, tbl, check, cmin, cmax);

  ophel_t **tosearch = work->tosearch;

  for (i=0; i<size; ++i) {    
    if (tosearch[i] == 0) continue;

    if (arg->debug) {
      pthread_mutex_lock(mutex);
      
      int hid = hsh->id;
      int idx = tbl->idx;
      const char *name = tbl->name;

      fprintf(arg->lfile, "check: ");

      switch (hsh->kind) {
      case lm1: fprintf(arg->lfile, "1st LM hash "); break;
      case lm2: fprintf(arg->lfile, "2nd LM hash "); break;
      case nt:  fprintf(arg->lfile, "NT hash "); break;
      }

      uint32_t prefix  = tosearch[i]->prefix;
      uint16_t postfix = tosearch[i]->postfix;
      uint64_t low  = tosearch[i]->low;
      uint64_t high = tosearch[i]->high;
      uint64_t offset = tosearch[i]->offset;
      
      fprintf(arg->lfile, "#%d in table %s,%d ", hid, name, idx);
      fprintf(arg->lfile, "at column %d, ", cmax-i);
      fprintf(arg->lfile, "with prefix = %u, ", prefix);
      fprintf(arg->lfile, "postfix = %u, ", (unsigned int)postfix);
      fprintf(arg->lfile, "low = %llu, high = %llu, ", (long long unsigned)low, (long long unsigned)high);
      fprintf(arg->lfile, "offset = %llu ", (long long unsigned int)offset);
      fprintf(arg->lfile, "and start = ");
      fprintf_hex(arg->lfile, tosearch[i]->pwd, 16);
      fprintf(arg->lfile, " ... ");
    }

    /* Check if the correct password has been found. */

    assert(tbl->param != NULL);

    int col = tosearch[i]->col;
    char *pwd = tosearch[i]->pwd;
    int found = tbl->check(hsh, tbl, tosearch[i]);

    if (found)
      ophcrack_found(crack, hsh, tbl, col, pwd);

    ophstat_add(task->stat, tosearch[i]->stat);

    if (arg->debug) {
      fprintf(arg->lfile, "found.\n");
      pthread_mutex_unlock(mutex);
    }

    /* If the password has been found, then we do not look any further. */

    if (found) break;
  }
  
  return task;
}
/*-------------------------------------------------------------------------*/
ophtask_t *ophcrack_bforce_all(ophcrack_t *crack, ophtask_t *task, int id) {
  scheduler_t *sched = crack->sched;

  ophbforce_t *force = task->data;
  hash_t **hashes = force->hashes;

  /* Create a task for each type of hash. */

  list_t *hashes_lm = list_alloc();
  list_t *hashes_nt = list_alloc();

  int nhashes = force->nhashes;
  int n;

  for (n=0; n<nhashes; ++n) {
    hash_t *hsh = hashes[n];
    hash_kind_t kind = hsh->kind;
    
    if (hsh->done) continue;

    switch (kind) {
    case lm1:
      /* Brute force a 1st LM hash only if the corresponding 2nd LM
	 hash is empty. */

      if (hsh->lmhsh2->done && hsh->lmhsh2->pwd[0] == 0)
	list_add_tail(hashes_lm, hsh);
      break;

    case lm2:
      list_add_tail(hashes_lm, hsh);
      break;
      
    case nt:
      /* Brute force a NT hash only if it is not connected to a LM
	 hash. */

      if (!hsh->lmhsh1 || !hsh->lmhsh2)
	list_add_tail(hashes_nt, hsh);
      break;
      
    default:
      break;
    }
  }

  /* Schedule the task only if there is something to do. */

  int maxlen = crack->arg->bforce_len;
  list_nd_t *nd;
  int i;

  /* LM hashes. */

  int size_lm = sizeof(extended_chars);
  uint64_t count_lm = 0;

  if (hashes_lm->size > 0) {
    ophtask_t *task = ophtask_alloc(bforce_lm);
    ophbforce_t *force = task->data;

    int nhashes = hashes_lm->size;
    
    force->nhashes = nhashes;
    force->hashes  = (hash_t**)malloc(nhashes*sizeof(hash_t*));

    for (nd=hashes_lm->head, n=0; nd!=0; nd=nd->next, ++n)
      force->hashes[n] = nd->data;

    for (i=1; i<=maxlen; ++i)
      count_lm += pow(size_lm, i);

    force->count = count_lm;
    scheduler_add(sched, task, low);
  }

  /* NT hashes. */

  int size_nt = sizeof(ntextended_ext95);
  uint64_t count_nt = 0;

  if (hashes_nt->size > 0) {
    ophtask_t *task = ophtask_alloc(bforce_nt);
    ophbforce_t *force = task->data;

    int nhashes = hashes_nt->size;

    force->nhashes = nhashes;
    force->hashes  = (hash_t**)malloc(nhashes*sizeof(hash_t*));

    for (nd=hashes_nt->head, n=0; nd!=0; nd=nd->next, ++n)
      force->hashes[n] = nd->data;

    for (i=1; i<=maxlen; ++i)
      count_nt += pow(size_nt, i);

    force->count = count_nt;
    scheduler_add(sched, task, low);
  }

  /* If there is nothing to do, then we direclty send a done message
     to indicate that the brute force is done. */

  int count = count_lm + count_nt;

  if (count == 0) {
    message_bforce(0, 1);
    message_bforce(1, 1);
  }
  
  /* Otherwise, we send the number of passwords we will enumerate. */

  else
    message_bforce(0, count);

  list_free(hashes_lm);
  list_free(hashes_nt);

  return task;
}
/*-------------------------------------------------------------------------*/
ophtask_t *ophcrack_resolve_nt(ophcrack_t *crack, ophtask_t *task, int id) {
  ophbforce_t *force = task->data;
  int mode = RESOLVE_DO_NOT_INSIST;

  hash_t **hashes = force->hashes;
  int nhashes = force->nhashes;
  int n;

  for (n=0; n<nhashes; ++n) {
    hash_t *nthsh  = hashes[n];
    hash_t *lmhsh1 = nthsh->lmhsh1;
    hash_t *lmhsh2 = nthsh->lmhsh2;
    
    assert(lmhsh1 && lmhsh2);
    assert(lmhsh1->done && lmhsh2->done);
    
    char *lmpwd1 = lmhsh1->pwd;
    char *lmpwd2 = lmhsh2->pwd;
    char *nthash = (char*)nthsh->hash;
    char ntpwd[16] = {0};
    
    if (resolve_nt_hash(lmpwd1, lmpwd2, nthash, (uchar_t*)ntpwd, mode))
      ophcrack_found(crack, nthsh, 0, 0, ntpwd);
  }

  return task;
}
/*-------------------------------------------------------------------------*/
ophtask_t *ophcrack_bforce_nt(ophcrack_t *crack, ophtask_t *task, int id) {
  scheduler_t *sched = crack->sched;
  pthread_mutex_t *mutex = sched->mutex;

  ophbforce_t *force = task->data;

  /* If they do not already exist, create two arrays which will hold
     the hash value and whether the password has been found. */

  hash_t **hashes = force->hashes;
  int nhashes = force->nhashes;
  int *bloom = force->bloom;
  int n;
  int bloom_byte, bloom_bit;

  if (force->hash == 0) {
    assert(force->found == 0);

    force->hash  = (uchar_t**)calloc(nhashes, sizeof(uchar_t*));
    force->found = (int*)calloc(nhashes, sizeof(int));

    for (n=0; n<nhashes; ++n) {
      hash_t *hsh = hashes[n];

      force->hash[n] = (uchar_t*)malloc(16*sizeof(uchar_t));
      memcpy(force->hash[n], hsh->hash, 16);

      bloom_byte = (* (int*)(force->hash[n])) & 0xfffff;
      bloom_bit = 1 << (bloom_byte >> 15);
      bloom[bloom_byte & 0x7fff] |= bloom_bit;
      
    }
  }

  /* Brute force. */

  const uchar_t *chars = ntextended_ext95;
  int size = sizeof(ntextended_ext95);
  
  uchar_t pwd[8] = {0};
  uchar_t hash[16] = {0};
  int *idx = force->idx;
  int len  = force->len;
  int nhredux = crack->arg->nhredux;
  int maxlen = crack->arg->bforce_len;

  assert(len > 0 && len <= maxlen);

  /* Test at most 'count' passwords. */

  int count = nhredux;
  int i = 0;

  while (count > 0 && len <= maxlen) {
    while (idx[0] < size && count > 0) {
      --count;
      --force->count;

      /* Move to the end of the password. */
      
      while (i < len) {
	pwd[i] = chars[idx[i]];
	++i;
      }
      
      /* Hash the password and compare the result to each hash of the
	 given list of hashes. */
      
      ntextended_mkhash(pwd, hash);

      int *bbp = (int*)hash;

      bloom_byte = *bbp & 0xfffff;
      bloom_bit = 1 << (bloom_byte >> 15);

      if (bloom[(bloom_byte & 0x7fff)] & bloom_bit) {

	for (n=0; n<nhashes; ++n) {
	  int found = force->found[n];
	  
	  if (!found && memcmp(force->hash[n], hash, 16) == 0) {
	    force->found[n] = 1;
	    /* Do not report deleted hashes as found. */
	    
	    if (hashes[n]->id >= 0) {
	      ophcrack_found(crack, hashes[n], 0, 0, (char*)pwd);
	      ++task->stat->match_bforce;
	    }
	  }
	}
      }
      
      /* Advance to the next password. */
      
      ++idx[--i];
      
      while (idx[i] == size && i > 0) {
	idx[i] = 0;
	++idx[--i];
      }
    }
    
    if (idx[0] == size) {
      ++len;
      idx[0] = 0;
    }
  }

  message_bforce(1, nhredux-count);
  
  /* If we did not enumerate all the possible passwords, then we
     schedule another brute force task. We also update the
     statistics to avoid having to wait until the brute force is
     done before seeing the updated values. */
  
  if (len <= maxlen) {
    force->len = len;
    
    pthread_mutex_lock(mutex);
    ophstat_add(crack->stat, task->stat);
    pthread_mutex_unlock(mutex);
    
    ophstat_reset(task->stat);
    scheduler_add(sched, task, low);
    
    task = 0;
  }

#ifndef NDEBUG
  else {
    assert(force->count == 0);
  }
#endif

  /* It is important to return 0 if we recycled the task above. */
  
  return task;
}
/*-------------------------------------------------------------------------*/
ophtask_t *ophcrack_bforce_lm(ophcrack_t *crack, ophtask_t *task, int id) {
  scheduler_t *sched = crack->sched;
  pthread_mutex_t *mutex = sched->mutex;

  ophbforce_t *force = task->data;

  /* If they do not already exist, create two arrays which will hold
     the hash value and the password of each hash. */

  hash_t **hashes = force->hashes;
  int nhashes = force->nhashes;
  int *bloom = force->bloom;
  int n;
  int bloom_byte, bloom_bit;

  if (force->hash == 0) {
    assert(force->found == 0);

    force->hash  = (uchar_t**)calloc(nhashes, sizeof(uchar_t*));
    force->found = (int*)calloc(nhashes, sizeof(int));

    for (n=0; n<nhashes; ++n) {
      hash_t *hsh = hashes[n];

      force->hash[n] = (uchar_t*)malloc(8*sizeof(uchar_t));
      memcpy(force->hash[n], hsh->hash, 8);

      bloom_byte = (* (int*)(force->hash[n])) & 0xfffff;
      bloom_bit = 1 << (bloom_byte >> 15);
      bloom[bloom_byte & 0x7fff] |= bloom_bit;
      
    }
  }

  /* Brute force. */
  
  const uchar_t *chars = extended_chars;
  int size = sizeof(extended_chars);
  
  uchar_t pwd[8] = {0};
  uchar_t hash[8] = {0};
  int *idx = force->idx;
  int len = force->len;
  int nhredux = crack->arg->nhredux;
  int maxlen = crack->arg->bforce_len;

  assert(len > 0 && len <= maxlen);

  /* Test at most 'count' passwords. */

  int count = nhredux;
  int i = 0;

  while (count > 0 && len <= maxlen) {
    while (idx[0] < size && count > 0) {
      --count;
      --force->count;
      
      /* Move to the end of the password. */
      
      while (i < len) {
	pwd[i] = chars[idx[i]];
	++i;
      }
      
      /* Hash the password and compare the result to each hash of the
	 given list of hashes. */

      lmtable_mkhash(pwd, hash);

      int *bbp = (int*)hash;

      bloom_byte = *bbp & 0xfffff;
      bloom_bit = 1 << (bloom_byte >> 15);

      if (bloom[(bloom_byte & 0x7fff)] & bloom_bit) {

	for (n=0; n<nhashes; ++n) {
	  int found = force->found[n];
	  
	  if (!found && memcmp(force->hash[n], hash, 8) == 0) {
	    force->found[n] = 1;
	    /* Do not report deleted hashes as found. */
	    
	    if (hashes[n]->id >= 0) {
	      ophcrack_found(crack, hashes[n], 0, 0, (char*)pwd);
	      ++task->stat->match_bforce;
	    }
	  }
	}
      }

      /* Advance to the next password. */
      
      ++idx[--i];
      
      while (idx[i] == size && i > 0) {
	idx[i] = 0;
	++idx[--i];
      }
    }

    if (idx[0] == size) {
      ++len;
      idx[0] = 0;
    } 
  }

  message_bforce(1, nhredux-count);

  /* If we did not enumerate all the possible passwords, then we
     schedule another brute force task. We also update the statistics
     to avoid having to wait until the brute force is done before
     seeing the updated values. */
  
  if (len <= maxlen) {
    force->len = len;

    pthread_mutex_lock(mutex);
    ophstat_add(crack->stat, task->stat);
    pthread_mutex_unlock(mutex);
    
    ophstat_reset(task->stat);
    scheduler_add(sched, task, low);

    task = 0;
  }

#ifndef NDEBUG
  else {
    assert(force->count == 0);
  }
#endif
  
  /* It is important to return 0 if we recycled the task above. */

  return task;  
}
/*-------------------------------------------------------------------------*/
int ophcrack_find_cmin(int ncols, int cmax, int total) {
  double y = ncols-cmax;
  double d = 1 - 4 * (-2*total - y*y + y);
  double x = floor(0.5 * (-1 + sqrt(d)));

  assert((x-y+1)*(x+y) <= 2*total);

  int cmin = ncols-(int)x;

  /* Make sure that we progress, even if arg->nhredux < chain length */
  if (cmin >= cmax)
    cmin = cmax-1;

  return cmin < 0 ? 0 : cmin;
}
/*-------------------------------------------------------------------------*/
int ophcrack_has_next(ophcrack_t *crack) {
  list_t *hashes = crack->hashes;
  list_nd_t *hnd;

  for (hnd = hashes->head; hnd != 0; hnd = hnd->next) {
    hash_t *hsh = hnd->data;

    list_t *tables = hsh->tables;
    list_nd_t *tnd;

    if (hsh->done != 0) continue;

    for (tnd = tables->head; tnd != 0; tnd = tnd->next) {
      htbl_t *htbl = tnd->data;
      table_t *tbl = htbl->tbl;

      if (tbl->active && htbl->col > 0) return 1;
    }
  }

  return 0;
}
/*-------------------------------------------------------------------------*/
ophtask_t *ophcrack_next(ophcrack_t *crack) {
  scheduler_t *sched = crack->sched;

  list_t *hashes = crack->hashes;
  int nhashes = hashes->size;

  int disk_size = sched->queue[disk]->size;
  int mdqueue = crack->arg->mdqueue;

  ophtask_t *task = 0;
  ophwork_t *work = 0;

  for (int h=0; h<nhashes && !task; ++h) {

    list_nd_t *hnd = crack->hnd == 0 ? hashes->head : crack->hnd;
    hash_t *hsh    = hnd->data;
    list_t *tables = hsh->tables;
    int ntables = tables->size;

    if (hsh->done == 0 && ntables > 0) {
      list_nd_t *tnd = hsh->tnd == 0 ? tables->head : hsh->tnd;
      htbl_t *htbl = tnd->data;
      table_t *tbl = htbl->tbl;
      table_preload_t preload = table_preload_state(tbl);

      /* Search for a table which is active and which has not been completely
         covered yet. We also check that the disk queue is not too big. If it is
         and searching into the table will cause a disk access, we look for
         another table. */

      int found = 0;

      for (int t=0; t<ntables && !found; ++t) {

        int disk_queue_full = disk_size > mdqueue && preload != preload_full;
        int table_valid = tbl->active && htbl->col >= 0;

        if (table_valid && !disk_queue_full) {
          found = 1;
          break;
        }

        tnd = tnd->next == 0 ? tables->head : tnd->next;
        htbl = tnd->data;
        tbl = htbl->tbl;
        preload = table_preload_state(tbl);
      }

      /* If a valid table has been found, we use it. */

      if (found) {
        assert(tbl->enabled);

        /* Calculate the mimimum column index such that at most 'nhredux'
           operations are performed. */

        int nhredux = crack->arg->nhredux;
        int cmax = htbl->col;
        int cmin = ophcrack_find_cmin(tbl->ncols, cmax, nhredux);

        /* Create a work task. */

        task = ophtask_alloc(find);
        work = task->data;

        work->hsh  = hsh;
        work->htbl = htbl;
        work->cmin = cmin;
        work->cmax = cmax;

        /* Store the column index from which we will start searching next time
           and move to the next table. */

        htbl->col = cmin-1;
        hsh->tnd  = tnd->next;
      }
    }

    crack->hnd = hnd->next;
  }

  return task;
}
/*-------------------------------------------------------------------------*/
int ophcrack_pwd_cmp(const void *a, const void *b) {
  ophel_t *ela = *(ophel_t**)a;
  ophel_t *elb = *(ophel_t**)b;

  assert(ela != 0);
  assert(elb != 0);

  if (ela->prefix > elb->prefix)
    return 1;
  else if (ela->prefix < elb->prefix)
    return -1;
  else
    if (ela->postfix > elb->postfix)
      return 1;
    else if (ela->postfix < elb->postfix)
      return -1;
    else
      return 0;
}
/*-------------------------------------------------------------------------*/
int ophcrack_col_cmp(const void *a, const void *b) {
  ophel_t *ela = *(ophel_t**)a;
  ophel_t *elb = *(ophel_t**)b;

  if (ela == 0 && elb == 0)
    return 0;
  else if (ela == 0)
    return 1;
  else if (elb == 0)
    return -1;

  if (ela->col < elb->col)
    return -1;
  else if (ela->col > elb->col)
    return 1;
  else
    return 0;
}
/*-------------------------------------------------------------------------*/
void ophcrack_found(ophcrack_t *crack, hash_t *hsh, table_t *tbl, int col,
		    char *pwd) {

  scheduler_t *sched = crack->sched;
  pthread_mutex_t *mutex = sched->mutex;
  ophtask_t *task = 0;
  ophbforce_t *force = 0;

  char *lmpwd1 = 0;
  char *lmpwd2 = 0;

  extern struct timeval tm_main_total;
  extern struct timeval tm_main_start;
  
  /* We must check the 'done' flag because somebody may have already
     cracked the hash before we reach this point. Furthermore, it
     might happen that we call this function with an hash whose
     password has already been set, in which case the pwd pointer is
     the same has the hsh->pwd pointer. We must also check that the
     hash id is valid because the hash could have been deleted. */

  pthread_mutex_lock(mutex);

  if (hsh->done == 0 && hsh->id >= 0) {
    if (hsh->pwd != pwd) strcpy(hsh->pwd, pwd);
    hsh->done = 1;

    if (hsh->length == 0) hsh->length = strlen(pwd);
    hsh->table = (char*) malloc(32*sizeof(char));
    if (tbl == 0) 
      snprintf(hsh->table, 32, "Bruteforce");
    else
      snprintf(hsh->table, 32, "%s", table_string(tbl->kind));
    hsh->category = categorize_password(hsh->pwd);
    
    if (hsh->time == -1) {
      if (tm_main_total.tv_sec == 0) {
	if (tm_main_start.tv_sec == 0)
	  hsh->time = 0;
	else {
	  struct timeval now;
	  gettimeofday(&now, 0);
	  long tv_sec = now.tv_sec - tm_main_start.tv_sec;
	  hsh->time = (int) tv_sec;
	}
      } else 
	hsh->time = (int) tm_main_total.tv_sec;
    }

    message_found(hsh, tbl, col);
    
    /* A NT hash has been cracked. */

    if (hsh->kind == nt) {
      ++crack->npwds_found;
      ++crack->stat->length[(hsh->length > 15) ? 15 : hsh->length];
      ++crack->stat->category[hsh->category];
      list_add_tail(crack->stat->time, &(hsh->time));
    }

    /* A 1st LM hash which is not associated to a NT hash and whose
       corresponding 2nd LM hash has been cracked. */

    else if (hsh->kind == lm1 && hsh->lmhsh1 == 0 && hsh->lmhsh2->done > 0) {
      ++crack->npwds_found;
      ++crack->stat->length[(hsh->length > 15) ? 15 : hsh->length];
      ++crack->stat->category[hsh->category];
      list_add_tail(crack->stat->time, &(hsh->time));
    }

    /* A 2nd LM hash which is not associated to a NT hash and whose
       corresponding 1st LM hash has been cracked. */

    else if (hsh->kind == lm2 && hsh->lmhsh2 == 0 && hsh->lmhsh1->done > 0) {
      ++crack->npwds_found;
      ++crack->stat->length[(hsh->length > 15) ? 15 : hsh->length];
      ++crack->stat->category[hsh->category];
      list_add_tail(crack->stat->time, &(hsh->time));
    }

    /* If both LM hashes have been cracked, then we schedule a brute
       force task to try to crack the corresponding NT hash. */
    
    if (hsh->kind == lm1 || hsh->kind == lm2) {
      hash_t *nthsh = hsh->kind == lm1 ? hsh->lmhsh1 : hsh->lmhsh2;
      
      if (nthsh && nthsh->done == 0) {
	hash_t *lmhsh1 = nthsh->lmhsh1;
	hash_t *lmhsh2 = nthsh->lmhsh2;
	
	if (lmhsh1->done > 0 && lmhsh2->done > 0) {
	  task  = ophtask_alloc(resolve_nt);
	  force = task->data;

	  force->nhashes = 1;
	  force->hashes  = (hash_t**)malloc(sizeof(hash_t*));
	  force->hashes[0] = nthsh;
	}
      }
    }

    /* If we cracked a NT hash and the associated LM hashes have not
       been cracked yet, then we can mark the LM hashes as done. */

    else {
      hash_t *lmhsh1 = hsh->lmhsh1;
      hash_t *lmhsh2 = hsh->lmhsh2;

      char *pwd = hsh->pwd;
      int len = strlen(hsh->pwd);

      if (lmhsh1 && lmhsh1->done == 0) {
	int n = MY_MIN(7, len);
	lmpwd1 = lm_from_nt(pwd, n, lmhsh1->hash);
      }

      if (lmhsh2 && lmhsh2->done == 0 && len > 7) {
	int n = MY_MIN(7, len-7);
	lmpwd2 = lm_from_nt(pwd+7, n, lmhsh2->hash);
      }
    }
  }

  pthread_mutex_unlock(mutex);

  /* If some LM hashes have been cracked, then we mark them as
     done. This must be performed outside of the mutexed region
     because otherwise the call to ophcrack_found will block on the
     mutex. */

  if (lmpwd1) {
    ophcrack_found(crack, hsh->lmhsh1, 0, 0, lmpwd1);
    free(lmpwd1);
  }

  if (lmpwd2) {
    ophcrack_found(crack, hsh->lmhsh2, 0, 0, lmpwd2);
    free(lmpwd2);
  }

  /* The scheduling of the task must also happen outside of the
     mutexed region because scheduler_add use the mutex too. */

  if (task != 0) scheduler_add(sched, task, high);
}
/*-------------------------------------------------------------------------*/
void ophcrack_notfound(ophcrack_t *crack, hash_t *hsh) {
  scheduler_t *sched = crack->sched;
  pthread_mutex_t *mutex = sched->mutex;

  /* We must check the 'done' flag because somebody may have already
     cracked the hash before we reach this point. */

  pthread_mutex_lock(mutex);

  if (hsh->done == 0) {
    hsh->done = -1;
    message_found(hsh, 0, 0);
  }

  pthread_mutex_unlock(mutex);
}
/*-------------------------------------------------------------------------*/
void ophcrack_preload_check(ophcrack_t *crack) {
  list_t *active = crack->active;

  assert(active->size > 0);

  /* If there is only one active table, then we do not have anything
     to do. */

  if (active->size == 1) return;

  /* Otherwise, we check that the way the tables have been preloaded
     satisfies the preloading policy. */

  list_t *tables = list_alloc();

  int size = active->size;
  int flag = 1;

  while (size--) {
    table_t *tbl = list_rem_head(active);

    if (tbl->idxmem && tbl->endmem && tbl->srtmem)
      list_add_tail(active, tbl);
    
    else if (tbl->idxmem && tbl->endmem && flag) {
      list_add_tail(active, tbl);
      flag = 0;
    }

    else
      list_add_tail(tables, tbl);
  }

  /* Put the tables which have been removed from the 'active' list
     into the 'remaining' list and mark them as inactive. */

  list_t *remaining = crack->remaining;

  while (tables->size > 0) {
    table_t *tbl = list_rem_tail(tables);
    
    tbl->active = 0;
    list_add_head(remaining, tbl);
  }

  list_free(tables);
}
/*-------------------------------------------------------------------------*/
ophtask_t *ophcrack_preload_all(ophcrack_t *crack, ophtask_t *task, int id) {
  scheduler_t *sched = crack->sched;
  ophload_t *load = task->data;

  list_t *tables = load->tables;
  list_t *active = crack->active;
  list_t *enabled = crack->enabled;
  list_t *remaining = crack->remaining;

  uint64_t freeram0 = crack->find_freeram();
  uint64_t freeram1 = crack->freeram;
  uint64_t freeram  = MY_MIN(freeram0, freeram1);

  /* Compute the amount of memory available if all tables were
     unloaded. */

  uint8_t *initialised = (uint8_t*)calloc(crack->maxtkind, sizeof(uint8_t));

  for (list_nd_t *nd = crack->tables->head; nd != NULL; nd = nd->next) {
    table_t *tbl = nd->data;
    table_preload_t state = table_preload_state(tbl);

    assert(tbl->kind >= 0 && tbl->kind < crack->maxtkind);

    if (tbl->shared_init && initialised[tbl->kind])
      freeram += table_preload_size(tbl, state & ~preload_init);
    else
      freeram += table_preload_size(tbl, state);

    initialised[tbl->kind] |= state & preload_init;
  }

  /* Mark all tables as inactive and assume that nothing as been
     preloaded yet. */

  table_preload_t *preload = (table_preload_t*)calloc(crack->maxtid, sizeof(table_preload_t));
  
  for (list_nd_t *nd = crack->tables->head; nd != NULL; nd = nd->next) {
    table_t *tbl = nd->data;

    assert(tbl->id >= 0 && tbl->id < crack->maxtid);

    preload[tbl->id] = preload_none;
    initialised[tbl->kind] = 0;

    tbl->active = 0;
  }

  /* If tables are provided in the task, we process them. Otherwise we
     process the active and remaining tables which are enabled.*/

  if (tables->size == 0) {
    for (list_nd_t *nd = active->head; nd != NULL; nd = nd->next) {
      table_t *tbl = nd->data;

      if (tbl->enabled) 
	list_add_tail(tables, tbl);
    }

    for (list_nd_t *nd = remaining->head; nd != NULL; nd = nd->next) {
      table_t *tbl = nd->data;
      
      if (tbl->enabled) 
	list_add_tail(tables, tbl);
    }
  }

  /* Try to preload as much as we can */

  list_clear(active);
  list_clear(remaining);

  static table_preload_t states[] = {
    preload_full,
    preload_full - preload_srt,
    preload_full - preload_srt - preload_end,
    preload_full - preload_srt - preload_end - preload_idx
  };

  static int nstates = sizeof(states) / sizeof(table_preload_t);

  for (int k=0; k<2; ++k) {

    int lst_state_idx = nstates-1;

    for (list_nd_t *nd = tables->head; nd != NULL; nd = nd->next) {
      table_t *tbl = nd->data;

      /* Fully covered tables are ignored during the 1st pass. During the 2nd
         pass, it is the not fully covered tables which are ignored. This way
         fully covered tables are unloaded only when some memory space is
         required to preload tables which have not yet been fully covered. */

      if ((k == 0 && tbl->cmax == 0) ||
          (k == 1 && tbl->cmax > 0))
        continue;

      assert(tbl->enabled);

      int fst_state_idx = nstates-1 - crack->arg->preload;

      if (k == 1) {
        table_preload_t preload = table_preload_state(tbl);

        for (int i=fst_state_idx; i<=lst_state_idx; ++i)
          if (states[i] == preload) {
            fst_state_idx = i;
            break;
          }
      }

      for (int i=fst_state_idx; i<=lst_state_idx; ++i) {
        /* Compute the amount of memory required for the preload */

        table_preload_t state = states[i];
        uint64_t size = 0;

        if (tbl->shared_init && initialised[tbl->kind])
          size = table_preload_size(tbl, state & ~preload_init);
        else
          size = table_preload_size(tbl, state);

        /* Check whether there is enough memory for the preload */

        if (size <= freeram) {
          freeram -= size;
          preload[tbl->id] = state;
          initialised[tbl->kind] = 1;

          /* If this table has not been completely preloaded, we want subsequent
             tables to have at least the same preload state. This is because we
             assume that a table with more files on disk will likely slow down
             the search. */

          if (k == 0 && state != preload_full) lst_state_idx = i;

          break;
        }
      }

      if (k == 0 && preload[tbl->id] == preload_none)
        list_add_tail(remaining, tbl);

      else if (tbl->cmax > 0)
        list_add_tail(active, tbl);
    }
  }

  /* Mark tables in the active list as active and tables in the
     remaining list as inactive. */
  
  for (list_nd_t *nd = active->head; nd != NULL; nd = nd->next) {
    table_t *tbl = nd->data;
    tbl->active = 1;
  }

  for (list_nd_t *nd = remaining->head; nd != NULL; nd = nd->next) {
    table_t *tbl = nd->data;
    tbl->active = 0;
  }

  /* Schedule an unload task for all tables which have been at least
     partially preloaded and have been disabled. */

  for (list_nd_t *nd = crack->tables->head; nd != 0; nd = nd->next) {
    table_t *tbl = nd->data;
    table_preload_t state = table_preload_state(tbl);

    if (!tbl->enabled && state != preload_none) {
      ophtask_t *task = ophtask_alloc(unload);
      ophload_t *load = task->data;

      load->tbl = tbl;
      load->preload = state;

      scheduler_add(sched, task, disk);
    }
  }

  /* Schedule the un/preload tasks only if there is actually
     something to do with the tables currently activated. */

  uint64_t total_size = 0;

  if (ophcrack_has_next(crack)) {
    /* Schedule an unload task for all the tables for which the
       desired preload status is different from the current one. */
    
    for (list_nd_t *nd = enabled->head; nd != 0; nd = nd->next) {
      table_t *tbl = nd->data;
      table_preload_t state = table_preload_state(tbl);
      table_preload_t unload_bits = state & ~preload[tbl->id];

      if (unload_bits != preload_none) {
	ophtask_t *task = ophtask_alloc(unload);
	ophload_t *load = task->data;
	
	load->tbl = tbl;
	load->preload = unload_bits;
	
	scheduler_add(sched, task, disk);
      }
    }
    
    /* Schedule an unload task with an empty table pointer. This will
       indicate, when the task will be executed, that the unloading is
       done. */
    
    ophtask_t *unload_done = ophtask_alloc(unload);
    scheduler_add(sched, unload_done, disk);
    
    /* Schedule a preload task for all the active tables for which the
       desired preload status is different from the current one. */

    static table_preload_t steps[] = {
      preload_init,
      preload_idx,
      preload_end,
      preload_srt
    };

    static int nsteps = sizeof(steps) / sizeof(table_preload_t);

    for (list_nd_t *nd = enabled->head; nd != 0; nd = nd->next) {
      table_t *tbl = nd->data;
      table_preload_t state = table_preload_state(tbl);
      table_preload_t load_bits = ~state & preload[tbl->id];

      if (load_bits == preload_none)
        continue;

      total_size += table_preload_size(tbl, load_bits);

      for (int i=0; i<nsteps; ++i) {
        if (load_bits & steps[i]) {
          ophtask_t *task = ophtask_alloc(preload_one);
          ophload_t *load = task->data;

          load->tbl = tbl;
          load->preload = steps[i];

          scheduler_add(sched, task, disk);
        }
      }
    }
  }

  /* If no tables have been activated and some tables remain to be
     preloaded, it means the remaining tables have to be initialised,
     but there is not enough memory available. */

  if (active->size == 0) {
    while (remaining->size > 0) {
#ifdef NDEBUG
      list_rem_head(remaining);
#else
      table_t *tbl = list_rem_head(remaining);
      assert(tbl->inisize > 0);
#endif
    }
  }

  /* Send a message indicating the amount of RAM occupied by the tables
     preloaded. */
  
  message_preload(0, preload_none, 0, total_size);
  
  /* Schedule a preload task with an empty table pointer. This will
     indicate, when the task will be executed, that the preloading is
     done. */
  
  ophtask_t *preload_done = ophtask_alloc(preload_one);
  scheduler_add(sched, preload_done, disk);

  free(preload);
  free(initialised);

  return task;
}
/*-------------------------------------------------------------------------*/
ophtask_t *ophcrack_preload_one(ophcrack_t *crack, ophtask_t *task, int id) {
  scheduler_t *sched = crack->sched;
  pthread_mutex_t *mutex = sched->mutex;

  ophload_t *load = task->data;
  table_t *tbl = load->tbl;

  if (tbl != 0) {
    assert(tbl->enabled);

    uint64_t size = table_preload_size(tbl, load->preload);

    message_preload(tbl, load->preload, 0, size);
    size = table_preload(tbl, load->preload);

    pthread_mutex_lock(mutex);
    crack->freeram -= size;
    pthread_mutex_unlock(mutex);

    message_preload(tbl, load->preload, 1, size);
  }
  else
    message_done(preload_all);

  return task;
}
/*-------------------------------------------------------------------------*/
ophtask_t *ophcrack_unload(ophcrack_t *crack, ophtask_t *task, int id) {
  scheduler_t *sched = crack->sched;
  pthread_mutex_t *mutex = sched->mutex;

  ophload_t *load = task->data;
  table_t *tbl = load->tbl;

  if (tbl != 0) {
    uint64_t size = table_unload(tbl, load->preload);

    pthread_mutex_lock(mutex);
    crack->freeram += size;
    pthread_mutex_unlock(mutex);

    message_unload(tbl, size);
  }
  else
    message_done(unload);

  return task;
}
/*-------------------------------------------------------------------------*/
void ophcrack_update(ophcrack_t *crack) {
  scheduler_t *sched = crack->sched;
  pthread_mutex_t *mutex = sched->mutex;

  /* Create an array which will store whether we have already seen a
     table with a given id. */

  int *seen = (int*)calloc(crack->maxtid, sizeof(int));

  /* Go through all the hashes and update cmax according to the value
     found in each table encountered. */
  
  list_t *hashes = crack->hashes;
  list_nd_t *hnd;

  pthread_mutex_lock(mutex);

  for (hnd = hashes->head; hnd != 0; hnd = hnd->next) {
    hash_t *hsh = hnd->data;
    list_t *tables = hsh->tables;
    list_nd_t *tnd;

    for (tnd = tables->head; tnd != 0; tnd = tnd->next) {
      htbl_t *htbl = tnd->data;
      table_t *tbl = htbl->tbl;

      /* cmin and cmax are set to zero if the hash has been cracked. */
      
      int id = tbl->id;
      int cmin = hsh->done != 0 ? 0 : htbl->col+1;
      int cmax = hsh->done != 0 ? 0 : htbl->covered;

      /* If we see the table for the first time, then we initialise
	 it with the values we found. */

      if (!seen[id]) {
	tbl->cmin = cmin;
	tbl->cmax = cmax;

	seen[id] = 1;
      } 

      /* Otherwise, we update the stored values. */

      else {
	tbl->cmin = MY_MAX(tbl->cmin, cmin);
	tbl->cmax = MY_MAX(tbl->cmax, cmax);
      }
    }
  }

  // Check each active table to see whether all columns have been
  // considered. If this happens, then the table is marked as inactive
  // and a message is sent.

  int done = 0;

  list_t *active = crack->active;
  list_nd_t *nd;

  for (nd = active->head; nd != 0; nd = nd->next) {
    table_t *tbl = nd->data;

    if (tbl->active && tbl->cmax == 0) {
      tbl->active = 0;
      done = 1;
    }
  }

  pthread_mutex_unlock(mutex);

  if (done) message_done(check);

  free(seen);
}
/*-------------------------------------------------------------------------*/
void ophcrack_save(ophcrack_t *crack, FILE *file, int nice, int status) {
  /* We need to be careful to write each line only once. */

  int maxhid = crack->maxhid;
  hash_t **id_to_hash = (hash_t**)calloc(maxhid, sizeof(hash_t*));
  
  list_t *hashes = crack->hashes;
  list_nd_t *nd;

  arg_t *arg = crack->arg;
  
  for (nd = hashes->head; nd != 0; nd = nd->next) {
    hash_t *hsh = nd->data;
    id_to_hash[hsh->id] = hsh;
  }
  
  /* Store the hashes in the order they have been read. */
  
  int i;
  
  for (i=0; i<maxhid; ++i) {
    assert(id_to_hash[i] != 0);
    hash_print(id_to_hash[i], file, nice, status, arg->hideuname);
  }

  free(id_to_hash);
}
/*-------------------------------------------------------------------------*/
void ophcrack_export_csv(ophcrack_t *crack, FILE *file, int *fields, char separator, char quote) {

  /* We need to be careful to write each line only once. */

  int maxhid = crack->maxhid;
  hash_t **id_to_hash = (hash_t**)calloc(maxhid, sizeof(hash_t*));
  
  list_t *hashes = crack->hashes;
  list_nd_t *nd;

  arg_t *arg = crack->arg;
  int hide = arg->hideuname;

  
  for (nd = hashes->head; nd != 0; nd = nd->next) {
    hash_t *hsh = nd->data;
    id_to_hash[hsh->id] = hsh;
  }
  
  /* Store the hashes in the order they have been read. */
  
  int i,first;

  for (i=0; i<maxhid; ++i) {
    assert(id_to_hash[i] != 0);

    first = 1;
    hash_t *hsh = id_to_hash[i];
    hash_kind_t kind = hsh->kind;
  
    /* Find the associated LM and NT hashes. */
    
    hash_t *lmhsh1 = 0;
    hash_t *lmhsh2 = 0;
    hash_t *nthsh  = 0;
    int type       = 0;
    
    switch (kind) {
    case lm1:
      lmhsh1 = hsh;
      lmhsh2 = hsh->lmhsh2;
      nthsh  = hsh->lmhsh1;
      if (lmhsh2 == 0 && nthsh == 0) type = 1;
      if (lmhsh2 != 0 && nthsh == 0) type = 2;
      break;
      
    case lm2:
      lmhsh1 = hsh->lmhsh1;
      lmhsh2 = hsh;
      nthsh  = hsh->lmhsh2;
      if (lmhsh1 != 0 && nthsh == 0) type = 2;
      break;
      
    case nt:
      lmhsh1 = hsh->lmhsh1;
      lmhsh2 = hsh->lmhsh2;
      nthsh  = hsh;
      if (lmhsh1 != 0 && lmhsh2 != 0) type = 3;
      if (lmhsh1 == 0 && lmhsh2 == 0) type = 4;
      break;
      
    default:
      break;
    }

    if (type) {

      if (fields[0]) {
	if (!first) fprintf(file,"%c", separator);
	if (quote) fprintf(file, "%c%d%c", quote, hsh->id, quote);
	else fprintf(file, "%d", hsh->id); 
	first = 0;
      }
      
      if (fields[1]) {
	if (!first) fprintf(file,"%c", separator);
	if (quote) {if (hsh->uid) fprintf(file, "%c%d%c", quote, hsh->uid, quote); }
	else {if (hsh->uid) fprintf(file, "%d", hsh->uid); }
	first = 0;
      }
      
      if (fields[2]) {
	if (!first) fprintf(file,"%c", separator);
	if (quote) {
	  if (hide) fprintf(file, "%c*****%c", quote, quote);
	  else {if (hsh->info[0]) fprintf(file, "%c%s%c", quote, hsh->info, quote); }
	}
	else {
	  if (hide) fprintf(file, "*****"); 
	  else {if (hsh->info[0]) fprintf(file, "%s", hsh->info);}
	}
	first = 0;
      }
      
      if (fields[3]) {
	if (!first) fprintf(file,"%c", separator);
	if (quote) {if (lmhsh1) fprintf(file, "%c%s%c", quote, lmhsh1->str, quote); }
	else {if (lmhsh1) fprintf(file, "%s", lmhsh1->str); }
	first = 0;
      }
      
      if (fields[4]) {
	if (!first) fprintf(file,"%c", separator);
	if (quote) {if (nthsh) fprintf(file, "%c%s%c", quote, nthsh->str, quote); }
	else {if (nthsh) fprintf(file, "%s", nthsh->str);}
	first = 0;
      }
      
      if (fields[5]) {
	if (!first) fprintf(file,"%c", separator);
	if (quote) {if (lmhsh1) fprintf(file, "%c%s%c", quote, lmhsh1->pwd, quote);} 
	else {if (lmhsh1) fprintf(file, "%s", lmhsh1->pwd);}
	first = 0;
      }
      
      if (fields[6]) {
	if (!first) fprintf(file,"%c", separator);
	if (quote) {if (lmhsh2) fprintf(file, "%c%s%c", quote, lmhsh2->pwd, quote);}
	else {if (lmhsh2) fprintf(file, "%s", lmhsh2->pwd); }
	first = 0;
      }
      
      if (fields[7]) {
	if (!first) fprintf(file,"%c", separator);
	if (quote) {if (nthsh) fprintf(file, "%c%s%c", quote, nthsh->pwd, quote); }
	else {if (nthsh) fprintf(file, "%s", nthsh->pwd);}
	first = 0;
      }
      
      if (fields[8]) {
	if (!first) fprintf(file,"%c", separator);
	switch (type) {
	case 1: if (quote) fprintf(file, "%c%d%c", quote, lmhsh1->done, quote);
	  else fprintf(file, "%d", lmhsh1->done);
	  break;
	case 2: if (quote) fprintf(file, "%c%d%c", quote, lmhsh1->done && lmhsh2->done, quote);
	  else fprintf(file, "%d", lmhsh1->done && lmhsh2->done);
	  break;
	case 3:
	case 4: 
	default: if (quote) fprintf(file, "%c%d%c", quote, nthsh->done, quote);
	  else fprintf(file, "%d", nthsh->done);
	  break;
	}
	first = 0;
      }
      
      if (fields[9]) {
	if (!first) fprintf(file,"%c", separator);
	switch (type) {
	case 1: if (quote) {if (lmhsh1->done > 0) fprintf(file, "%c%d%c", quote, lmhsh1->time, quote);}
	  else {if (lmhsh1->done > 0) fprintf(file, "%d", lmhsh1->time);}
	  break;
	case 2: if (quote) {if (lmhsh1->done > 0 && lmhsh2->done > 0) fprintf(file, "%c%d%c", quote, (lmhsh1->time > lmhsh2->time) ? lmhsh1->time : lmhsh2->time, quote);}
	  else {if (lmhsh1->done > 0 && lmhsh2->done > 0) fprintf(file, "%d", (lmhsh1->time > lmhsh2->time) ? lmhsh1->time : lmhsh2->time);}
	  break;
	case 3:
	case 4: 
	default: if (quote) {if (nthsh->done > 0) fprintf(file, "%c%d%c", quote, nthsh->time, quote);}
	  else {if (nthsh->done > 0) fprintf(file, "%d", nthsh->time);}
	  break;
	}
	first = 0;
      }
      
      if (fields[10]) {
	if (!first) fprintf(file,"%c", separator);
	switch (type) {
	case 1: if (quote) {if (lmhsh1->done > 0) fprintf(file, "%c%d%c", quote, lmhsh1->length, quote);}
	  else {if (lmhsh1->done > 0) fprintf(file, "%d", lmhsh1->length);}
	  break;
	case 2: if (quote) {if (lmhsh1->done > 0 && lmhsh2->done > 0) fprintf(file, "%c%d%c", quote, lmhsh1->length + lmhsh2->length, quote);}
	  else {if (lmhsh1->done > 0 && lmhsh2->done > 0) fprintf(file, "%d", lmhsh1->length + lmhsh2->length);}
	  break;
	case 3:
	case 4: 
	default: if (quote) {if (nthsh->done) fprintf(file, "%c%d%c", quote, nthsh->length, quote);}
	  else {if (nthsh->done) fprintf(file, "%d", nthsh->length);}
	  break;
	}
	first = 0;
      }
      
      if (fields[11]) {
	if (!first) fprintf(file,"%c", separator);
	switch (type) {
	case 1: if (quote) {if (lmhsh1->done > 0) fprintf(file, "%c%s%c", quote, category_string(lmhsh1->category), quote);}
	  else {if (lmhsh1->done > 0) fprintf(file, "%s", category_string(lmhsh1->category));}
	  break;
	case 2: if (quote) {if (lmhsh1->done > 0 && lmhsh2->done > 0) fprintf(file, "%c%s%c", quote, category_string(lmhsh1->category|lmhsh2->category), quote);}
	  else {if (lmhsh1->done > 0 && lmhsh2->done > 0) fprintf(file, "%s", category_string(lmhsh1->category|lmhsh2->category));}
	  break;
	case 3:
	case 4: 
	default: if (quote) {if (nthsh->done) fprintf(file, "%c%s%c", quote, category_string(nthsh->category), quote);}
	  else {if (nthsh->done) fprintf(file, "%s", category_string(nthsh->category));}
	  break;
	}
	first = 0;
      }
      
      if (fields[12]) {
	if (!first) fprintf(file,"%c", separator);
	switch (type) {
	case 1: if (quote) {if (lmhsh1->done > 0) fprintf(file, "%c%s%c", quote, lmhsh1->table, quote);}
	  else {if (lmhsh1->done > 0) fprintf(file, "%s", lmhsh1->table);}
	  break;
	case 2: if (quote) {if (lmhsh1->done > 0 && lmhsh2->done > 0) fprintf(file, "%c%s/%s%c", quote, lmhsh1->table, lmhsh2->table, quote);}
	  else {if (lmhsh1->done > 0 && lmhsh2->done > 0) fprintf(file, "%s/%s", lmhsh1->table, lmhsh2->table);}
	  break;
	case 3: if (quote) {if (nthsh->done) fprintf(file, "%c%s/%s%c", quote, lmhsh1->table, lmhsh2->table, quote);}
	  else {if (nthsh->done) fprintf(file, "%s/%s", lmhsh1->table, lmhsh2->table);}
	  break;
	case 4: 
	default: if (quote) {if (nthsh->done) fprintf(file, "%c%s%c", quote, nthsh->table, quote);}
	  else {if (nthsh->done) fprintf(file, "%s", nthsh->table);}
	  break;
	}
	first = 0;
      }
      fprintf(file, "\n");
    }
  }

  free(id_to_hash);
}
/*-------------------------------------------------------------------------*/
char *lm_from_nt(const char *pwd, int n, const uchar_t *lmhash) {
  uchar_t tmp[8] = {0};
  uchar_t hash[8] = {0};
  int i;

  /* Map the password on the appropriate charset. */

  assert(n < 8);
  memcpy(tmp, pwd, n);

  for (i=0; i<n; ++i)
    if (tmp[i] >= 'a' && tmp[i] <= 'z')
      tmp[i] = tmp[i] + - 'a' + 'A';
    else
      switch (tmp[i]) {
      case 0xc3: case 0xe3: case 0xc2: case 0xc0:
      case 0xe2: case 0xe0:
	tmp[i] = 'A';
	break;
	
      case 0xcb: case 0xeb: case 0xea: case 0xca:
      case 0xe8: case 0xc8:
	tmp[i] = 'E';
	break;
	
      case 0xcd: case 0xed: case 0xcf: case 0xce:
      case 0xef: case 0xee:
	tmp[i] = 'I';
	break;
	
      case 0xd5: case 0xd3: case 0xf5: case 0xf3:
      case 0xd4: case 0xf4:
	tmp[i] = 'O';
	break;
	
      case 0xda: case 0xfa: case 0xdb: case 0xd9:
      case 0xfb: case 0xf9:
	tmp[i] = 'U';
	break;
	
      case 0xe4:
	tmp[i] = 0xc4;
	break;
	
      case 0xe7:
	tmp[i] = 0xe7;
	break;
	
      case 0xe9:
	tmp[i] = 0xc9;
	break;
	
      case 0xf1:
	tmp[i] = 0xd1;
	break;
	
      case 0xf6:
	tmp[i] = 0xd6;
	break;
	
      case 0xfc:
	tmp[i] = 0xdc;
	break;

      default:
	break;
      }

  /* Check that the pwd corresponds to the hash. */

  lmtable_mkhash((uchar_t*)tmp, hash);

  if (memcmp(lmhash, hash, sizeof(hash)) == 0)
    return strdup((char*)tmp);
  else
    return 0;
}
