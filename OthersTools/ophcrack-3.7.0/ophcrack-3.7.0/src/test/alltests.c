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
#undef NDEBUG

#include <sys/time.h>
#include <assert.h>
#include <limits.h>

#include "ophcrack.h"
#include "message.h"
#include "lmtable.h"
#include "ntproba.h"
#include "ntseven.h"
#include "nteightxl.h"

struct timeval tm_main_total;
struct timeval tm_main_start;

void table_set_size_4test(table_t *tbl) {
  const uint64_t *sizes = NULL;

  switch (tbl->kind) {
  case lmalphanum5k:
    sizes = lmalphanum5k_sizes;
    break;

  case ntseven: 
    sizes = ntseven_sizes;
    break;

  case nteightxl:
    sizes = nteightxl_sizes;
    break;

  case ntprobafree:
    sizes = ntprobafree_sizes;
    break;

  case ntproba10g:
    sizes = ntproba10g_sizes;
    break;

  case ntproba60g:
    sizes = ntproba60g_sizes;
    break;

  default:
    assert(0);
  }

  tbl->idxsize = sizes[1];
  tbl->endsize = sizes[2*tbl->idx+2];
  tbl->srtsize = sizes[2*tbl->idx+3];
}

uint64_t find_freeram_4preload(void) {
  return ULLONG_MAX;
}

ophcrack_t *create_crack_4test(table_t **tables, int ntables) {
  arg_t *arg = arg_alloc();
  ophcrack_t *crack = ophcrack_alloc(1, arg);

  crack->find_freeram = find_freeram_4preload;

  // Add the tables

  for (int i=0; i<ntables; ++i) {
    table_t *tbl = tables[i];

    table_set_size_4test(tbl);
    ophcrack_setup_table(tbl);
    ophcrack_add_table(crack, tbl);

    tbl->enabled = 1;
    list_add_tail(crack->enabled, tbl);
  }

  // Add some hashes

  char pwdump[] = 
    "3e645334fd788c5f949a30c86b9f0fb1:535f34e7eb41001a5d5eea2714d20315";

  list_t *hashes = list_alloc();
  hash_extract_lmnt(pwdump, hashes, 0);

  assert(hashes->size == 3);

  for (list_nd_t *nd = hashes->head; nd != NULL; nd = nd->next) {
    hash_t *hash = nd->data;

    ophcrack_add_hash(crack, hash);

    for (list_nd_t *nd = crack->enabled->head; nd != NULL; nd = nd->next) {
      table_t *tbl = nd->data;
      ophcrack_associate(crack, hash, tbl);
    }
  }

  list_free(hashes);

  return crack;
}

void preload_4test(ophcrack_t *crack, table_t **tables, int ntables) {
  ophtask_t *task = ophtask_alloc(preload_all);
  ophload_t *load = task->data;

  for (int i=0; i<ntables; ++i)
    list_add_tail(load->tables, tables[i]);

  ophcrack_preload_all(crack, task, 0);
  ophtask_free(task);
}

void free_crack_4test(ophcrack_t *crack) {
  for (list_nd_t *nd = crack->hashes->head; nd != NULL; nd = nd->next) {
    hash_t *hash = nd->data;
    hash_free(hash);
  }

  for (list_nd_t *nd = crack->tables->head; nd != NULL; nd = nd->next) {
    table_t *tbl = nd->data;
    table_free(tbl);
  }

  arg_free(crack->arg);
  ophcrack_free(crack);
}

void check_unload_task(scheduler_t *sched, table_t *tbl) {
  assert(sched->ntasks > 0);

  ophtask_t *task = scheduler_get(sched, 0);
  assert(task->kind == unload);

  ophload_t *load = task->data;
  assert(load->tbl == tbl);

  ophtask_free(task);
  scheduler_done(sched, 0);
}

void check_preload_task0(scheduler_t *sched, table_t *tbl, table_preload_t preload) {
  ophtask_t *task = scheduler_get(sched, 0);
  assert(task->kind == preload_one);

  ophload_t *load = task->data;
  assert(load->tbl == tbl);
  assert(load->preload == preload);
  
  ophtask_free(task);
  scheduler_done(sched, 0);
}

void check_preload_task(scheduler_t *sched, table_t *tbl, table_preload_t preload) {
  static const table_preload_t preloads[] = { preload_init,
					      preload_idx,
					      preload_end,
					      preload_srt };

  static const int npreloads = sizeof(preloads) / sizeof(table_preload_t);

  if (preload == preload_none)
    check_preload_task0(sched, tbl, preload_none);
  else
    for (int i=0; i<npreloads; ++i)
      if (preload & preloads[i])
	check_preload_task0(sched, tbl, preloads[i]);
}

void check_preload_message(table_t *tbl, table_preload_t preload, int done, uint64_t size) {
  message_t *msg = message_tryget();

  assert(msg != NULL);
  assert(msg->kind == msg_preload);

  msg_load_t *msg_load = msg->data;

  assert(msg_load->tbl == tbl);
  assert(msg_load->preload == preload);
  assert(msg_load->done == done);
  assert(msg_load->size == size);

  message_free(msg);
}

void test_ophcrack_preload_all0() {
  // Create and initialise a crack object

  table_t *tables[] = {
    // xp free fast (lmalphanum5k)
    table_alloc(0x0fa2031c, ".", 0),
    table_alloc(0x0fa2031c, ".", 1),
    table_alloc(0x0fa2031c, ".", 2),
    table_alloc(0x0fa2031c, ".", 3),
  };

  int ntables = sizeof(tables) / sizeof(table_t*);
  ophcrack_t *crack = create_crack_4test(tables, ntables);

  assert(crack->enabled->size == ntables);

  // Assume we have enough RAM for all tables

  uint64_t preload_size = 0;

  for (int i=0; i<ntables; ++i)
    preload_size += table_preload_size(tables[i], preload_full);

  crack->freeram = preload_size;

  // Perform preload

  preload_4test(crack, tables, ntables);

  assert(crack->active->size == 4);
  assert(crack->remaining->size == 0);
  assert(crack->remaining->size == crack->enabled->size - crack->active->size);

  // Check the tasks

  assert(crack->sched->ntasks == 18);

  check_unload_task(crack->sched, NULL);

  for (int i=0; i<ntables; ++i)
    check_preload_task(crack->sched, tables[i], preload_full);

  check_preload_task(crack->sched, NULL, preload_none);

  assert(crack->sched->ntasks == 0);

  // Check the messages

  check_preload_message(NULL, preload_none, 0, preload_size);

  message_t *msg = message_tryget();
  assert(msg == NULL);

  // Free memory

  free_crack_4test(crack);
}

void test_ophcrack_preload_all1() {
  table_t *tables[] = {
    // vista probabilistic 10G (ntproba10g)
    table_alloc(0x47000fdf, ".", 0),
    table_alloc(0x47000fdf, ".", 1),
    table_alloc(0x47000fdf, ".", 2),
    table_alloc(0x47000fdf, ".", 3)
  };

  int ntables = sizeof(tables) / sizeof(table_t*);
  ophcrack_t *crack = create_crack_4test(tables, ntables);

  // Assume we have enough RAM to load everything but the .start files

  crack->arg->preload = 2;

  uint64_t preload_size =
    table_preload_size(tables[0], preload_full - preload_srt) +
    table_preload_size(tables[1], preload_full - preload_srt) +
    table_preload_size(tables[2], preload_full - preload_srt) +
    table_preload_size(tables[3], preload_full - preload_srt);

  crack->freeram =
    preload_size -
    table_preload_size(tables[1], preload_init) -
    table_preload_size(tables[2], preload_init) -
    table_preload_size(tables[3], preload_init);

  // Perform preload

  preload_4test(crack, tables, ntables);

  assert(crack->active->size == ntables);
  assert(crack->remaining->size == 0);
  assert(crack->remaining->size == crack->enabled->size - crack->active->size);

  // Check the tasks

  assert(crack->sched->ntasks == 1 + 4*3 + 1);
  check_unload_task(crack->sched, NULL);

  check_preload_task(crack->sched, tables[0], preload_full - preload_srt);
  check_preload_task(crack->sched, tables[1], preload_full - preload_srt);
  check_preload_task(crack->sched, tables[2], preload_full - preload_srt);
  check_preload_task(crack->sched, tables[3], preload_full - preload_srt);

  check_preload_task(crack->sched, NULL, preload_none);
  assert(crack->sched->ntasks == 0);

  // Check the messages

  check_preload_message(NULL, preload_none, 0, preload_size);

  message_t *msg = message_tryget();
  assert(msg == NULL);

  // Free memory

  free_crack_4test(crack);
}

void test_ophcrack_preload_all2() {
  table_t *tables[] = {
    // xp free fast (lmalphanum5k)
    table_alloc(0x0fa2031c, ".", 0),
    table_alloc(0x0fa2031c, ".", 1),
    table_alloc(0x0fa2031c, ".", 2),
    table_alloc(0x0fa2031c, ".", 3),

    // vista probabilistic 10G (ntproba10g)
    table_alloc(0x47000fdf, ".", 0),
    table_alloc(0x47000fdf, ".", 1),
    table_alloc(0x47000fdf, ".", 2),
    table_alloc(0x47000fdf, ".", 3)
  };

  int ntables = sizeof(tables) / sizeof(table_t*);
  ophcrack_t *crack = create_crack_4test(tables, ntables);

  // Assume a specific amount of RAM is available

  uint64_t preload_size = 
    table_preload_size(tables[0], preload_full) +
    table_preload_size(tables[1], preload_full) +
    table_preload_size(tables[2], preload_full) +
    table_preload_size(tables[3], preload_full - preload_srt) +
    table_preload_size(tables[4], preload_full - preload_srt) +
    table_preload_size(tables[5], preload_full - preload_srt) +
    table_preload_size(tables[6], preload_full - preload_srt) +
    table_preload_size(tables[7], preload_full - preload_srt);

  crack->freeram = 
    preload_size +
    table_preload_size(tables[3], preload_srt) / 2 -
    table_preload_size(tables[5], preload_init) -
    table_preload_size(tables[6], preload_init) -
    table_preload_size(tables[7], preload_init);

  // Perform two consecutive preloads

  for (int s=0; s<2; ++s) {
    // Perform preload

    if (s == 0)
      preload_4test(crack, tables, ntables);
    else
      preload_4test(crack, NULL, 0);

    assert(crack->active->size == ntables);
    assert(crack->remaining->size == 0);
    assert(crack->remaining->size == crack->enabled->size - crack->active->size);

    // Check the tasks

    if (s == 0) {
      assert(crack->sched->ntasks == 1 + 3*4 + 5*3 + 1);
      check_unload_task(crack->sched, NULL);

      check_preload_task(crack->sched, tables[0], preload_full);
      check_preload_task(crack->sched, tables[1], preload_full);
      check_preload_task(crack->sched, tables[2], preload_full);
      check_preload_task(crack->sched, tables[3], preload_full - preload_srt);
      check_preload_task(crack->sched, tables[4], preload_full - preload_srt);
      check_preload_task(crack->sched, tables[5], preload_full - preload_srt);
      check_preload_task(crack->sched, tables[6], preload_full - preload_srt);
      check_preload_task(crack->sched, tables[7], preload_full - preload_srt);
    }
    else {
      assert(crack->sched->ntasks == 1 + 4*4 + 4*3 + 1);
      check_unload_task(crack->sched, NULL);

      check_preload_task(crack->sched, tables[0], preload_full);
      check_preload_task(crack->sched, tables[1], preload_full);
      check_preload_task(crack->sched, tables[2], preload_full);
      check_preload_task(crack->sched, tables[3], preload_full);
      check_preload_task(crack->sched, tables[4], preload_full - preload_srt);
      check_preload_task(crack->sched, tables[5], preload_full - preload_srt);
      check_preload_task(crack->sched, tables[6], preload_full - preload_srt);
      check_preload_task(crack->sched, tables[7], preload_full - preload_srt);
    }

    check_preload_task(crack->sched, NULL, preload_none);
    assert(crack->sched->ntasks == 0);

    // Check the messages

    check_preload_message(NULL, preload_none, 0, preload_size);

    message_t *msg = message_tryget();
    assert(msg == NULL);

    if (s == 0) {
      // Assume a bit more RAM is now available

      crack->freeram += table_preload_size(tables[3], preload_srt);
      preload_size += table_preload_size(tables[3], preload_srt);
    }
  }

  // Free memory

  free_crack_4test(crack);
}

/* Check that if no RAM is available, all tables are active. */

void test_ophcrack_preload_all3() {
  table_t *tables[] = {
    // xp free fast (lmalphanum5k)
    table_alloc(0x0fa2031c, ".", 0),
    table_alloc(0x0fa2031c, ".", 1),
    table_alloc(0x0fa2031c, ".", 2),
    table_alloc(0x0fa2031c, ".", 3)
  };

  int ntables = sizeof(tables) / sizeof(table_t*);
  ophcrack_t *crack = create_crack_4test(tables, ntables);

  // Assume no RAM is available

  crack->freeram = 0;

  // Perform preload

  preload_4test(crack, tables, ntables);

  assert(crack->active->size == ntables);
  assert(crack->remaining->size == 0);
  assert(crack->remaining->size == crack->enabled->size - crack->active->size);

  // Check the tasks

  assert(crack->sched->ntasks == 1 + 4 + 1);

  check_unload_task(crack->sched, NULL);
  check_preload_task(crack->sched, tables[0], preload_init);
  check_preload_task(crack->sched, tables[1], preload_init);
  check_preload_task(crack->sched, tables[2], preload_init);
  check_preload_task(crack->sched, tables[3], preload_init);
  check_preload_task(crack->sched, NULL, preload_none);

  assert(crack->sched->ntasks == 0);

  // Check the messages

  check_preload_message(NULL, preload_none, 0, 0);

  message_t *msg = message_tryget();
  assert(msg == NULL);

  // Free memory

  free_crack_4test(crack);
}

/* Test that only enabled tables are considered for preload. */

void test_ophcrack_preload_all4() {
  table_t *tables[] = {
    // xp free fast (lmalphanum5k)
    table_alloc(0x0fa2031c, ".", 0),
    table_alloc(0x0fa2031c, ".", 1),
    table_alloc(0x0fa2031c, ".", 2),
    table_alloc(0x0fa2031c, ".", 3)
  };

  int ntables = sizeof(tables) / sizeof(table_t*);
  ophcrack_t *crack = create_crack_4test(tables, ntables);

  // Assume a specific amount of RAM is availble

  uint64_t preload_size =
    table_preload_size(tables[0], preload_full) +
    table_preload_size(tables[1], preload_full) +
    table_preload_size(tables[2], preload_full) +
    table_preload_size(tables[3], preload_full);

  crack->freeram = preload_size;

  for (int s=0; s<2; ++s) {
    // Perform preload

    if (s == 0) {
      assert(crack->enabled->size == ntables);
      preload_4test(crack, tables, ntables);

      assert(crack->active->size == ntables);
      assert(crack->remaining->size == 0);
    }
    else {
      for (int i=0; i<ntables; ++i)
	tables[i]->enabled = 0;

      list_clear(crack->enabled);

      assert(crack->enabled->size == 0);
      preload_4test(crack, tables, 0);

      assert(crack->active->size == 0);
      assert(crack->remaining->size == 0);
    }

    assert(crack->remaining->size == crack->enabled->size - crack->active->size);

    // Check the tasks

    if (s == 0) {
      assert(crack->sched->ntasks == 1 + 4*4 + 1);

      while (crack->sched->ntasks > 0) {
	ophtask_t *task = scheduler_get(crack->sched, 0);
	ophtask_free(task);
	scheduler_done(crack->sched, 0);
      }
    }
    else {
      assert(crack->sched->ntasks == 1);
      check_preload_task(crack->sched, NULL, preload_none);
    }

    assert(crack->sched->ntasks == 0);

    // Check the messages

    if (s == 0)
      check_preload_message(NULL, preload_none, 0, preload_size);
    else
      check_preload_message(NULL, preload_none, 0, 0);

    message_t *msg = message_tryget();
    assert(msg == NULL);
  }

  // Free memory

  free_crack_4test(crack);
}

/* Test that at most one .bin file is searched on disk. */

void test_ophcrack_preload_all5() {
  table_t *tables[] = {
    // vista seven (ntseven)
    table_alloc(0x9542377a, ".", 0),
    // vista probabilistic 60G (ntproba60g)
    table_alloc(0x2c002335, ".", 0),
    // vista probabilistic free (ntprobafree)
    table_alloc(0xbb00950e, ".", 0),
    // xp free fast (lmalphanum5k)
    table_alloc(0x0fa2031c, ".", 0)
  };

  int ntables = sizeof(tables) / sizeof(table_t*);
  ophcrack_t *crack = create_crack_4test(tables, ntables);

  uint64_t preload_size =
    table_preload_size(tables[0], preload_full - preload_srt) +
    table_preload_size(tables[1], preload_full - preload_srt - preload_end) +
    table_preload_size(tables[2], preload_none) +
    table_preload_size(tables[3], preload_full - preload_srt);

  crack->freeram = 
    table_preload_size(tables[0], preload_full - preload_srt) +
    table_preload_size(tables[1], preload_full - preload_srt - preload_end) +
    table_preload_size(tables[2], preload_full - preload_srt - preload_end);
    table_preload_size(tables[3], preload_full - preload_srt - preload_end);

  preload_4test(crack, tables, ntables);

  assert(crack->active->size == 3);
  assert(crack->remaining->size == 1);
  assert(crack->enabled->size == crack->active->size + crack->remaining->size);

  assert(crack->sched->ntasks == 1 + 3 + 2 + 0 + 3 + 1);

  check_unload_task(crack->sched, NULL);
  check_preload_task(crack->sched, tables[0], preload_full - preload_srt);
  check_preload_task(crack->sched, tables[1], preload_full - preload_srt - preload_end);
  check_preload_task(crack->sched, tables[3], preload_full - preload_srt);
  check_preload_task(crack->sched, NULL, preload_none);

  check_preload_message(NULL, preload_none, 0, preload_size);
  assert(message_tryget() == NULL);

  free_crack_4test(crack);
}

/* Test that at most one .index or .bin file is searched on disk. */

void test_ophcrack_preload_all6() {
  table_t *tables[] = {
    // vista seven
    table_alloc(0x9542377a, ".", 0),
    // vista eight xl (nteightxl)
    table_alloc(0x229e1899, ".", 0),
    // xp free fast (lmalphanum5k)
    table_alloc(0x0fa2031c, ".", 0),
  };

  int ntables = sizeof(tables) / sizeof(table_t*);
  ophcrack_t *crack = create_crack_4test(tables, ntables);

  uint64_t preload_size =
    table_preload_size(tables[0], preload_full - preload_srt) +
    table_preload_size(tables[1], preload_init) +
    table_preload_size(tables[2], preload_none);

  crack->freeram = 
    table_preload_size(tables[0], preload_full - preload_srt) +
    table_preload_size(tables[1], preload_init) +
    table_preload_size(tables[2], preload_full - preload_srt) - 10;

  preload_4test(crack, tables, ntables);

  assert(crack->active->size == 2);
  assert(crack->remaining->size == 1);
  assert(crack->enabled->size == crack->active->size + crack->remaining->size);

  assert(crack->sched->ntasks == 1 + 3 + 1 + 0 + 1);

  check_unload_task(crack->sched, NULL);
  check_preload_task(crack->sched, tables[0], preload_full - preload_srt);
  check_preload_task(crack->sched, tables[1], preload_init);
  check_preload_task(crack->sched, NULL, preload_none);

  check_preload_message(NULL, preload_none, 0, preload_size);
  assert(message_tryget() == NULL);

  free_crack_4test(crack);
}

/* Test what happens when probabilistic tables cannot be initialised. */

void test_ophcrack_preload_all7() {
  table_t *tables[] = {
    // vista probabilistic free (ntprobafree)
    table_alloc(0xbb00950e, ".", 0),
    table_alloc(0xbb00950e, ".", 1),
    table_alloc(0xbb00950e, ".", 2),
  };

  int ntables = sizeof(tables) / sizeof(table_t*);
  ophcrack_t *crack = create_crack_4test(tables, ntables);

  crack->freeram = table_preload_size(tables[0], preload_init) / 2;

  preload_4test(crack, tables, ntables);

  assert(crack->active->size == 0);
  assert(crack->remaining->size == 0);

  assert(crack->sched->ntasks == 1);

  check_preload_task(crack->sched, NULL, preload_none);

  check_preload_message(NULL, preload_none, 0, 0);
  assert(message_tryget() == NULL);

  free_crack_4test(crack);
}

void test_ophcrack_preload_all_idx_only() {
  table_t *tables[] = {
    // vista eight xl (nteightxl)
    table_alloc(0x229e1899, ".", 0),
    table_alloc(0x229e1899, ".", 1),
    table_alloc(0x229e1899, ".", 2),
    table_alloc(0x229e1899, ".", 3)
  };

  int ntables = sizeof(tables) / sizeof(table_t*);
  ophcrack_t *crack = create_crack_4test(tables, ntables);

  // Assume we have enough RAM to load only the .idx files

  uint64_t preload_size =
    table_preload_size(tables[0], preload_full - preload_end - preload_srt) +
    table_preload_size(tables[1], preload_full - preload_end - preload_srt) +
    table_preload_size(tables[2], preload_full - preload_end - preload_srt) +
    table_preload_size(tables[3], preload_full - preload_end - preload_srt);

  crack->freeram = preload_size;

  preload_4test(crack, tables, ntables);

  assert(crack->active->size == 4);

  free_crack_4test(crack);
}

int main(int argc, char **argv) {
  message_init();

  gettimeofday(&tm_main_start, 0);
  tm_main_total.tv_sec = 0;

  test_ophcrack_preload_all0();
  test_ophcrack_preload_all1();
  // test_ophcrack_preload_all2();
  test_ophcrack_preload_all3();
  test_ophcrack_preload_all4();
  // test_ophcrack_preload_all5();
  // test_ophcrack_preload_all6();
  test_ophcrack_preload_all7();
  test_ophcrack_preload_all_idx_only();

  return 0;
}
