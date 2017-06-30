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
#include <config.h>
#ifndef WIN32
#include <stdlib.h>
#endif
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#ifdef WIN32
#include <windows.h>
#endif
#ifdef HAVE_GUI
#include <QApplication>
#endif

#include "ophcrack.h"
#include "message.h"
#ifdef HAVE_GUI
#include "ophcrackgui.h"
#endif
#include "fsm.h"
#include "arg.h"
#include "misc.h"
/*-------------------------------------------------------------------------*/
struct timeval tm_main_start;
struct timeval tm_main_total;
fsm_t *fsm = 0;
/*-------------------------------------------------------------------------*/
void usage() {
  printf("ophcrack %s by Objectif Securite (http://www.objectif-securite.ch)\n\n", PACKAGE_VERSION);
  printf("Usage: ophcrack [OPTIONS]\n");
  printf("Cracks Windows passwords with Rainbow tables\n\n");
  printf("  -a              disable audit mode (default)\n");
  printf("  -A              enable audit mode\n");
  printf("  -b              disable bruteforce\n");
  printf("  -B              enable bruteforce (default)\n");
  printf("  -c config_file  specify the config file to use\n");
  printf("  -D              display (lots of!) debugging information\n");
  printf("  -d dir          specify tables base directory\n");
  printf("  -e              do not display empty passwords\n");
  printf("  -f file         load hashes from the specified file (pwdump or session)\n");
  printf("  -g              disable GUI\n");
  printf("  -h              display this information\n");
  printf("  -i              hide usernames\n");
  printf("  -I              show usernames (default)\n");
  printf("  -l file         log all output to the specified file\n");
  printf("  -n num          specify the number of threads to use\n");
  printf("  -o file         write cracking output to file in pwdump format\n");
  printf("  -p num          preload (0 none, 1 index, 2 index+end, 3 all default)\n"); 
  printf("  -q              quiet mode\n");
  printf("  -r              launch the cracking when ophcrack starts (GUI only)\n");
  printf("  -s              disable session auto-saving\n");
  printf("  -S session_file specify the file to use to automatically save the progress of the search\n");
  printf("  -u              display statistics when cracking ends\n");
  printf("  -t table1[,a[,b,...]][:table2[,a[,b,...]]]\n");
  printf("                  specify which table to use in the directory given by -d\n");
  printf("  -v              verbose\n");
  printf("  -w dir          load hashes from encrypted SAM file in directory dir\n");
  printf("  -x file         export data in CSV format to file\n");
  printf("\n\nExample:\tophcrack -g -d /path/to/tables -t xp_free_fast,0,3:vista_free -f in.txt\n\n");
  printf("\t\tLaunch ophcrack in command line using tables 0 and 3 in\n");
  printf("\t\t/path/to/tables/xp_free_fast and all tables in /path/to/tables/vista_free\n");
  printf("\t\tand cracks hashes from pwdump file in.txt\n\n");
  exit(0);
}
/*-------------------------------------------------------------------------*/
void load_config(arg_t *arg) {
  char buff[STR_BUFF_SIZE];

  /* If no config file is given on the command line, then we try the
     default one. */
  
  if (arg->cfname == 0) {
#ifndef WIN32
    char *home  = getenv("HOME");
    snprintf(buff, sizeof(buff), "%s/.ophcrackrc", home);
#else
    snprintf(buff, sizeof(buff), "./.ophcrackrc");
#endif

    arg->cfname = strdup(buff);
    arg_read_conf(arg);
  }
  
  /* Otherwise, we read the one provided. */

  else
    if (arg_read_conf(arg) == -1) {
      fprintf(stderr, "Cannot open file %s for reading.\n", arg->cfname);
      exit(1);
    }
}
/*-------------------------------------------------------------------------*/
int load_tables(ophcrack_t *crack, arg_t *arg, list_t *table_str) {
  list_nd_t *tnd;
  list_nd_t *dnd;

  list_t *table_path = arg->table_path;
  list_t *tables = list_alloc();

  for (tnd = table_str->head; tnd != 0; tnd = tnd->next) {
    char *tblstr = (char*)tnd->data;
    
    for (dnd = table_path->head; dnd != 0; dnd = dnd->next) {
      char *dir = (char*)dnd->data;
      int ret = table_open(tables, dir, tblstr);
      
      /* The tables have been successfully opened. */
      
      if (ret >= 0) {
	if (!arg->use_gui && ret > 0) {
	  if (!arg->quiet || arg->lfile != stdout) {
	    if (dir[0] != 0)
	      fprintf(arg->lfile, 
		      "Opened %d table(s) from %s/%s.\n", 
		      ret, dir, tblstr);
	    else
	      fprintf(arg->lfile,
		      "Opened %d table(s) from %s.\n", 
		      ret, tblstr);
	  }
	}

	break;
      } 

      /* The opening of the tables failed. */
      
      else if (!arg->use_gui && ret < -1) {
	fprintf(stderr, "Cannot open the table(s) %s.\n", tblstr);
	exit(1);
      }
    }
    
    // If the table has not been found and we are in command line
    // mode, then we complain.

    if (!arg->use_gui && dnd == 0) {
      fprintf(stderr, "Did not find the requested table(s) %s.\n", tblstr);
      exit(1);
    }
  }

  /* If we are in command line mode, then we check that something has
     actually been loaded. */

  int ntables = tables->size;

  if (!arg->use_gui && ntables == 0) {
    fprintf(stderr, "No tables were loaded. Use the -t option to specify some.\n");
    exit(1);
  } 

  /* Add the tables to the list of tables used by ophcrack. */
  
  list_nd_t *nd;
  
  for (nd = tables->head; nd != 0; nd = nd->next) {
    table_t *tbl = (table_t*)nd->data;

    ophcrack_setup_table(tbl);

    int ret = table_verify(tbl);
    if (ret > 0)
      ophcrack_add_table(crack, tbl);
    else if (!arg->use_gui && ret < 0) 
      fprintf(stderr, "Size of table(s) %s,%d is invalid. Download it again.\n", tbl->name, tbl->idx);
    
    
  }  
  
  list_free(tables);
  
  return ntables;
}
/*-------------------------------------------------------------------------*/
int load_hashes_pwdump(ophcrack_t *crack, int id) {
  arg_t *arg = crack->arg;

  const char *hfname = arg->hfname;
  FILE *file = fopen(hfname, "r");
  
  if (!file) {
    fprintf(stderr, "Cannot open file %s.\n", hfname);
    exit(1);
  }
  
  list_t *hashes = list_alloc();
  int npwds = hash_load_pwdump(hashes, file, id);
  int nhashes = hashes->size;

  fclose(file);
  
  /* Check that something has actually been loaded. */
  
  if (npwds == 0) {
    fprintf(stderr, "No proper hashes have been found in %s.\n", hfname);
    exit(1);
  } else if (!arg->use_gui)
    if (!arg->quiet || arg->lfile != stdout)
      fprintf(arg->lfile, "%d hashes have been found in %s.\n", 
	      nhashes, hfname);
  
  /* Add the hashes to the list of hash we must potentially crack. */
  
  list_nd_t *nd;
  
  for (nd = hashes->head; nd != 0; nd = nd->next) {
    hash_t *hsh = (hash_t*)nd->data;
    ophcrack_add_hash(crack, hsh);
  }
  
  list_free(hashes);
  
  return npwds;
}
/*-------------------------------------------------------------------------*/
int load_hashes_sam(ophcrack_t *crack, int id) {
  arg_t *arg = crack->arg;

  const char *dir = arg->samdir;

  list_t *hashes = list_alloc();
  int npwds = hash_load_sam(hashes, dir, id);
  int nhashes = hashes->size;

  // Check that something has actually been loaded.

  if (npwds == 0) {
    fprintf(stderr,
	    "No proper hashes have been found in the encrypted SAM file in %s.\n", 
	    arg->samdir);
    exit(1);
  }

  else if (npwds > 0 && !arg->use_gui)
    if (!arg->quiet || arg->lfile != stdout)
      fprintf(arg->lfile, 
	      "%d hashes have been found in the encrypted SAM found in %s.\n\n", 
	      nhashes, dir);

  // Check if the SYSTEM or SAM file could not be found.
  
  if (npwds == -1) {
    fprintf(stderr, "No SYSTEM file has been found in %s.\n", arg->samdir);
    exit(1);
  }

  else if (npwds == -2) {
    fprintf(stderr, "No SAM file has been found in %s.\n", arg->samdir);
    exit(1);
  }

  // Check if there was a problem while reading the SYSTEM or SAM file.

  else if (npwds == -3) {
    fprintf(stderr, 
	    "A problem occured while reading the SYSTEM file found in %s.\n", 
	    arg->samdir);
    exit(1);
  }

  else if (npwds == -4) {
    fprintf(stderr,
	    "A problem occured while reading the SAM file found in %s.\n", 
	    arg->samdir);
    exit(1);
  }

  // Add the hashes to the list of hash we must potentially crack.
  
  list_nd_t *nd;
  
  for (nd = hashes->head; nd != 0; nd = nd->next) {
    hash_t *hsh = (hash_t*)nd->data;
    ophcrack_add_hash(crack, hsh);
  }

  list_free(hashes);

  return npwds;
}
/*-------------------------------------------------------------------------*/
void display_stats(ophcrack_t *crack) {
  ophstat_t *stat = crack->stat;
  arg_t *arg = crack->arg;

  if (arg->quiet && arg->lfile == stdout) return;
  
  fprintf(arg->lfile, "\n\nStatistics:\n");
  fprintf(arg->lfile, "%-32s: %llu\n", "# of hash/redux", (long long unsigned int) stat->hredux);
  fprintf(arg->lfile, "%-32s: %llu\n", "# of fseek", (long long unsigned int) stat->fseek_idx+stat->fseek_end+stat->fseek_srt);
  fprintf(arg->lfile, "%-32s: %llu\n", "# of false alarms", (long long unsigned int) stat->falarm);
  fprintf(arg->lfile, "%-32s: %llu\n", "# of hash/redux per false alarm", 
	 (stat->falarm >0) ? (long long unsigned int) (stat->falarm_hredux / stat->falarm) : (long long unsigned int) 0);
  fprintf(arg->lfile, "%-32s: %llu\n", "# of matches in tables", (long long unsigned int) stat->match_table);
  fprintf(arg->lfile, "%-32s: %llu\n", "# of matches in brute force", (long long unsigned int) stat->match_bforce);
  fprintf(arg->lfile, "%-32s: %llu\n", "# of prefixs found", (long long unsigned int) stat->prefix);
  fprintf(arg->lfile, "%-32s: %llu\n", "# of postfixs found", (long long unsigned int) stat->postfix);
  fprintf(arg->lfile, "%-32s: %llu\n", "# of start", (long long unsigned int) stat->start);
  fprintf(arg->lfile, "%-32s: %llu\n", "# of fseek in .index", (long long unsigned int) stat->fseek_idx);
  fprintf(arg->lfile, "%-32s: %llu\n", "# of fseek in .bin", (long long unsigned int) stat->fseek_end);
  fprintf(arg->lfile, "%-32s: %llu", "# of fseek in .start", (long long unsigned int) stat->fseek_srt);
}
/*-------------------------------------------------------------------------*/
void display_work(ophcrack_t *crack, msg_work_t *msg, char *info) {
  arg_t *arg = crack->arg;

  if (arg->quiet && arg->lfile == stdout) return;

  hash_t *hsh  = msg->hsh;
  table_t *tbl = msg->tbl;
  ophkind_t kind = msg->kind;
  int cmin  = msg->cmin;
  int cmax  = msg->cmax;

  char *name = tbl->name;
  int tidx   = tbl->idx;
  int hid    = hsh->id;
  
  fprintf(arg->lfile, "%s", info);

  switch (kind) {
  case find:
    fprintf(arg->lfile, "Find chain end for ");
    break;

  case lookup_idx:
    fprintf(arg->lfile, "Lookup prefixes for ");
    break;

  case lookup_end:
    fprintf(arg->lfile, "Lookup postfixes for ");
    break;

  case lookup_srt:
    fprintf(arg->lfile, "Lookup chain starts for ");
    break;

  case check:
    fprintf(arg->lfile, "Checking chain starts for ");
    break;

  default:
    break;
  }

  switch (hsh->kind) {
  case lm1:
    fprintf(arg->lfile, "1st LM hash #%d in table %s,%d from column %d to %d", 
	   hid, name, tidx, cmax, cmin);
    break;
    
  case lm2:
    fprintf(arg->lfile, "2nd LM hash #%d in table %s,%d from column %d to %d",
	   hid, name, tidx, cmax, cmin);
    break;

  case nt:
    fprintf(arg->lfile, "NT hash #%d in table %s,%d from column %d to %d",
	   hid, name, tidx, cmax, cmin);
    break;

  default:
    break;
  }

  fprintf(arg->lfile, "\n");
}
/*-------------------------------------------------------------------------*/
void display_load(ophcrack_t *crack, msg_load_t *msg, char *info) {
  arg_t *arg = crack->arg;

  if (arg->quiet && arg->lfile == stdout) return;

  table_t *tbl = msg->tbl;
  table_preload_t preload = msg->preload;
  int done  = msg->done;
  uint64_t size = msg->size >> 20;

  char *name = tbl->name;
  int tidx   = tbl->idx;

  fprintf(arg->lfile, "%s", info);

  if (preload >= preload_init && preload <= preload_srt) {
    if (done)
      fprintf(arg->lfile, "Done preloading ");
    else
      fprintf(arg->lfile, "Preloading ");
  }

  switch (preload) {
  case preload_init:
    if (tbl->shared_init)
      fprintf(arg->lfile, "shared init data ");
    else
      fprintf(arg->lfile, "init data ");
    break;

  case preload_idx:
    fprintf(arg->lfile, ".index file ");
    break;

  case preload_end:
    fprintf(arg->lfile, ".bin file ");
    break;

  case preload_srt:
    fprintf(arg->lfile, ".start file ");
    break;

  default:
    break;
  }

  if (preload >= preload_init && preload <= preload_srt) {
    fprintf(arg->lfile, "(%llu MB) ", (long long unsigned)size);
    fprintf(arg->lfile, "of table %s,%d\n", name, tidx);
  }
}
/*-------------------------------------------------------------------------*/
void display_found(ophcrack_t *crack, msg_found_t *found, char *info) {
  arg_t *arg = crack->arg;

  if (arg->quiet && arg->lfile == stdout) return;

  // Show that we have found something.
  
  hash_t *hsh  = found->hsh;
  table_t *tbl = found->tbl;
  int col = found->col;

  char *pwd = hsh->pwd;
  char *hinfo = hsh->info;

  int hid = hsh->id;
  int showempty = arg->showempty;

  /* If the table pointer is null, then it is either a brute forced or
     an empty pwd. */

  fprintf(arg->lfile, "%s", info);

  if (!arg->auditmode) {
    if (strcmp(pwd, "") == 0 && showempty)
      fprintf(arg->lfile, "Found empty password for ");
    else
      fprintf(arg->lfile, "Found password %s for ", pwd);

    switch (hsh->kind) {
    case lm1:
      fprintf(arg->lfile, "1st LM hash #%d ", hid);
      break;

    case lm2:
      fprintf(arg->lfile, "2nd LM hash #%d ", hid);
      break;

    case nt:
      if (strcmp(hinfo, "") == 0)
        fprintf(arg->lfile, "NT hash #%d ", hid);
      else
        if (arg->hideuname)
          fprintf(arg->lfile, "user ***** (NT hash #%d) ", hid);
        else
          fprintf(arg->lfile, "user %s (NT hash #%d) ", hinfo, hid);
      break;

    default:
      break;
    }
  } else {
    if (strcmp(pwd, "") == 0 && showempty)
      fprintf(arg->lfile, "Found empty password for ");
    else
      fprintf(arg->lfile, "Found password of length %d for ", hsh->length);

    switch (hsh->kind) {
    case lm1:
      fprintf(arg->lfile, "1st LM hash #%d ", hid);
      break;

    case lm2:
      fprintf(arg->lfile, "2nd LM hash #%d ", hid);
      break;

    case nt:
      fprintf(arg->lfile, "NT hash #%d ", hid);
      break;

    default:
      break;
    }
    fprintf(arg->lfile, "category: %s ", category_string(hsh->category));
  }
  if (tbl != 0)
    fprintf(arg->lfile, "in table %s #%d at column %d.", tbl->name, tbl->idx, col);

  fprintf(arg->lfile, "\n");
}
/*-------------------------------------------------------------------------*/
void display_status(ophcrack_t *crack, char *info) {
  scheduler_t *sched = crack->sched;
  pthread_mutex_t *mutex = sched->mutex;

  pthread_mutex_lock(mutex);
  int nactive  = crack->active->size;
  int nenabled = crack->enabled->size;
  int nremain  = crack->remaining->size;  

  int npwds_total = crack->npwds_total;
  int npwds_found  = crack->npwds_found;
  pthread_mutex_unlock(mutex);

  printf("%s", info);

  // Brute force.

  if (crack->arg->bforce) {
    uint64_t curr  = fsm->bforce_curr;
    uint64_t total = fsm->bforce_total;
    int ratio = 0;

    if (total > 0)
      ratio = (100*curr) / total;

    if (ratio < 100)
      printf("brute force (%d%%); ", ratio);
  }

  // Preload.

  if (fsm->state == st_preload) {
    uint64_t curr  = fsm->psize_curr;
    uint64_t total = fsm->psize_total;
    int ratio = 0;

    if (total > 0)
      ratio = (100*curr) / total;

    if (ratio < 100)
      printf("preload (%d%%); ", ratio);
    else {
      assert(curr == total);
      printf("preload done; ");
    }
  }

  // Work.

  else {
    list_t *tables = crack->tables;
    list_nd_t *nd;

    int mean_cmax = 0;
    int count = 0;

    for (nd = tables->head; nd != 0; nd = nd->next) {
      table_t *tbl = (table_t*)nd->data;

      if (!tbl->enabled) continue;

      uint64_t ncols = tbl->ncols;
      uint64_t cmax  = tbl->cmax;

      uint64_t n = ncols * ncols;
      uint64_t dcmax = ncols-cmax;
      int ratio_cmax = (100*dcmax*dcmax) / n;

      mean_cmax += ratio_cmax;
      ++count;
    }

    if (count > 0)
      mean_cmax /= count;

    int ndone = nenabled - nactive - nremain;

    printf("search (%d%%); ", mean_cmax);
    printf("tables: total %d, done %d, using %d; ", nenabled, ndone, nactive);
  }

  printf("pwd found %d/%d.", npwds_found, npwds_total);
}
/*-------------------------------------------------------------------------*/
void handle_messages(ophcrack_t *crack) {
  scheduler_t *sched = crack->sched;
  pthread_mutex_t *mutex = sched->mutex;

  char info[STR_BUFF_SIZE] = {0};
  message_t *msg = 0;
  int count = 1000;
  int pwdfound = 0;
  
  arg_t *arg = crack->arg;

  /* Handle at most 'count' messages. */

  struct timeval now;
  gettimeofday(&now, 0);

  long tv_sec = now.tv_sec - tm_main_start.tv_sec;
  long hour   = tv_sec / 3600;
  long sec    = tv_sec - 3600 * hour;
  long min    = sec / 60;
  
  sec %= 60;
  
  snprintf(info, sizeof(info), "%ldh %2ldm %2lds; ", hour, min, sec);
  
  while (count-- && (msg = message_tryget())) {
    if (arg->debug == 0 && !arg->quiet) printf("\x1B[2K\r");

    /* A password has been found. */

    if (msg->kind == msg_found) {
      msg_found_t *found = (msg_found_t*)msg->data;
      display_found(crack, found, info);
      pwdfound = 1;
    }

    /* Work message. */

    else if (arg->verbose && msg->kind == msg_work) {
      msg_work_t *work = (msg_work_t*)msg->data;
      display_work(crack, work, info);
    }

    /* Preload message. */

    else if (arg->verbose && msg->kind == msg_preload) {
      msg_load_t *load = (msg_load_t*)msg->data;
      if (load->tbl) display_load(crack, load, info);
    }

    /* Ask the finite state machine to take the appropriate steps
       given the received message. */

    msg = fsm_next(fsm, msg);

    if (msg)
      message_free(msg);
  }

  /* Save the passwords found so far. */
  
  if (pwdfound) {
    FILE *ofile = fopen(crack->arg->ofname, "w");
    FILE *xfile = 0;
    if (crack->arg->exportcsv)
      xfile = fopen(crack->arg->xfname, "w");
    
    int fields[] = {1,1,1,1,1,1,1,1,1,1,1,1,1};
    
    if (ofile) {
      pthread_mutex_lock(mutex);
      if (xfile) {
	ophcrack_export_csv(crack, xfile, fields, ';', 0);
	fclose(xfile);
      }
      ophcrack_save(crack, ofile, 0, 0);
      pthread_mutex_unlock(mutex);
      
      fclose(ofile);
    }
  }

  /* Display the status. */
  
  ophcrack_update(crack);

  if (arg->debug == 0 && !arg->quiet) {
    printf("\x1B[2K\r");
    display_status(crack, info);
    fflush(stdout);
  }

  fflush(arg->lfile);
}
/*-------------------------------------------------------------------------*/
int main(int argc, char **argv) {
  static const char options[] = "aAbBc:Dd:ef:ghiIl:n:o:p:qrsS:t:uvw:x:";

  arg_t *arg = arg_alloc();
  int c;

  // Add the empty table_path, so that we can specify the absolute
  // path with the 'table' keyword in the configuration file.

  list_add_tail(arg->table_path, strdup(""));

  if (argc == 1 && !arg->use_gui) usage();

  // Load the config file.

  int i = 1;

  while (i < argc && strcmp(argv[i++], "-c") != 0) {};
  if (i < argc) arg->cfname = strdup(argv[i]);

  load_config(arg);

  // Parse the command line.

#ifdef OSX
  // On Mac OS X, double-clicked applications have only one argument
  // of the form "-psn...". We therefore do not parse the arguments if
  // that one is present.

  if (argc >= 2 && strncmp(argv[1], "-psn", 4) != 0) {
#endif

  while ((c = getopt(argc, argv, options)) != -1)
    switch (c) {
    case 'a':
      arg->auditmode = 0;
      break;

    case 'A':
      arg->auditmode = 1;
      break;

    case 'b':
      arg->bforce = 0;
      break;

    case 'B':
      arg->bforce = 1;
      break;

    case 'D':
      arg->debug = 1;
      arg->verbose = 1;
      break;

    case 'd':
      list_add_head(arg->table_path, strdup(optarg));
      break;

    case 'e':
      arg->showempty = 0;
      break;

    case 'f':
      arg->hfname = strdup(optarg);
      break;

    case 'g':
      arg->use_gui = 0;
      break;

    case 'h':
      usage();
      break;

    case 'i':
      arg->hideuname = 1;
      break;
      
    case 'I':
      arg->hideuname = 0;
      break;
      
    case 'l':
      arg->lfname = strdup(optarg);
      break;

    case 'n':
      arg->nthreads = atoi(optarg);
      break;

    case 'o':
      arg->ofname = strdup(optarg);
      break;

    case 'p':
      arg->preload = atoi(optarg);
      break;

    case 'q':
      arg->quiet = 1;
      break;

    case 'r':
      arg->run = 1;
      break;

    case 's':
      arg->ssave = 0;
      break;

    case 'S':
      arg->ssave  = 1;
      arg->sfname = strdup(optarg);
      break;

    case 't':
      arg->tblstr = strdup(optarg);
      break;

    case 'u':
      arg->stats = 1;
      break;

    case 'v':
      arg->verbose = 1;
      break;

    case 'w':
      arg->samdir = strdup(optarg);
      break;

    case 'x':
      arg->exportcsv = 1;
      arg->xfname = strdup(optarg);
      break;

    default:
      break;
    }

#ifdef OSX
  }
#endif

  // If a log file has been provided, then we open it.

  if (arg->lfname != 0) {
    arg->lfile = fopen(arg->lfname, "w");
    
    if (arg->lfile == 0) {
      fprintf(stderr, "Cannot open file %s for writing.\n", arg->lfname);
      exit(1);
    }
  }

  // If we are in command line mode, then we check that the output
  // file is writable.

  if (!arg->use_gui && arg->ofname) {
    FILE *ofile = fopen(arg->ofname, "w");
    
    if (ofile == 0) {
      fprintf(stderr, "Cannot open file %s for writing.\n", arg->ofname);
      exit(1); 
    } else
      fclose(ofile);
  }

  // If we are in command line mode, then we check that the csv
  // file is writable.

  if (!arg->use_gui && arg->exportcsv) {
    FILE *xfile = fopen(arg->xfname, "w");
    
    if (xfile == 0) {
      fprintf(stderr, "Cannot open file %s for writing.\n", arg->xfname);
      exit(1); 
    } else
      fclose(xfile);
  }

  // If the session auto-saving is enabled, then we check that the
  // session file is writable.

  if (arg->ssave && arg->sfname) {
    FILE *sfile = fopen(arg->sfname, "a");
    
    if (sfile == 0) {
      fprintf(stderr, 
	      "The session file %s you provided either in ", 
	      arg->sfname);
      fprintf(stderr, 
	      "the config file or on the command line is not writable.\n");
      exit(1);
    } else
      fclose(sfile);
  }

  // Store the table arguments given on the command line.

  if (arg->tblstr != 0) {
    list_t *table_str = arg->table_str_cmd;
    char *tmp;

    while ((tmp = strsep(&arg->tblstr, ":")) != 0 && strcmp(tmp, "") != 0)
      list_add_tail(table_str, strdup(tmp));
  }

  // Initialise the message queue.

  message_init();
   
  // Start ophcrack. Past this point we are in a multi-threaded
  // application. The number of threads is generally set equal to the
  // number of cores in the CPU plus one.

  ophcrack_t *crack = ophcrack_alloc(arg->nthreads+1, arg);
  ophcrack_start(crack);

  // Load the hashes from the pwdump file if one has been provided.

  int maxhid = 0;
  int npwds  = 0;

  if (arg->hfname != 0)
    npwds += load_hashes_pwdump(crack, maxhid);

  assert(crack->maxhid == npwds);

  // Load the hashes from the encrypted SAM if a directory is
  // specified and one is found.

  maxhid = crack->maxhid;

  if (arg->samdir != 0)
    npwds += load_hashes_sam(crack, maxhid);

  assert(crack->maxhid == npwds);
  assert(crack->npwds_total == npwds);

  // Complain if no hashes have been loaded.

  if (!arg->use_gui && npwds == 0) {
    fprintf(stderr, "No hash loaded. Use the -f or -w option.\n");
    exit(1);
  }

#ifdef HAVE_GUI
  // Start the GUI.

  if (arg->use_gui) {
    QApplication app(argc, argv);
    OphcrackGUI gui(crack);

    gui.show();
  
    return app.exec();
  }
#endif

  // If no tables were specified, then we load those specified in the
  // config file. Otherwise, we load the ones specified on the command
  // line.

  if (arg->table_str_cmd->size == 0)
    load_tables(crack, arg, arg->table_str_conf);
  else
    load_tables(crack, arg, arg->table_str_cmd);

  // Build the list of enabled tables.

  list_t *tables  = crack->tables;
  list_t *enabled = crack->enabled;
  list_nd_t *nd;

  list_clear(enabled);

  for (nd = tables->head; nd != 0; nd = nd->next) {
    table_t *tbl = (table_t*)nd->data;

    if (tbl->enabled)
      list_add_tail(enabled, tbl);
  }

  // Associate the appropriate tables to each hash.

  list_t *hashes = crack->hashes;
  list_nd_t *tnd, *hnd;

  for (hnd = hashes->head; hnd != 0; hnd = hnd->next) {
    hash_t *hsh = (hash_t*)hnd->data;

    for (tnd = enabled->head; tnd != 0; tnd = tnd->next) {
      table_t *tbl = (table_t*)tnd->data;
      ophcrack_associate(crack, hsh, tbl);
    }
  }

  // Main loop. Launch the preload and brute force, treat the messages
  // and exit when everything is done.

  int count = 0;
  fsm = fsm_alloc(crack);

  gettimeofday(&tm_main_start, 0);
  tm_main_total.tv_sec = 0;
  fsm_handle_start(fsm);

  while (fsm->state != st_wait) {
    handle_messages(crack);

    // Wait one second

#ifdef WIN32
    Sleep(1000);
#else
    sleep(1);
#endif
    
    ++count;

    // Check if it is time to save the session.
    
    if (arg->ssave && fsm->state & (st_work1 + st_work2) && count >= 30) {
      fsm_ssave(fsm);
      count = 0;
    }
  }

  // Display the results in command line mode

  if (!arg->use_gui) {
    if (arg->stats) 
      display_stats(crack);

    if (!arg->quiet || arg->lfile != stdout) {
      fprintf(arg->lfile, "\n\nResults:\n\n");
      fprintf(arg->lfile, 
	      "%-32s %-14s %s\n", "username / hash", "LM password", "NT password");
      ophcrack_save(crack, arg->lfile, 1, 0);
    }
  }
  
  if (arg->debug == 0 && !arg->quiet) printf("\n");

  // Close the log file.

  if (arg->lfile != stdout) fclose(arg->lfile);


  /* Save the passwords to the output and csv files. */

  if (!arg->use_gui && arg->ofname) {
    FILE *ofile = fopen(arg->ofname, "w");
    FILE *xfile = 0;
    if (arg->exportcsv)
      xfile = fopen(arg->xfname, "w");
    
    int fields[] = {1,1,1,1,1,1,1,1,1,1,1,1,1};
    
    if (ofile) {
      if (xfile) {
	ophcrack_export_csv(crack, xfile, fields, ';', 0);
	fclose(xfile);
      }
      ophcrack_save(crack, ofile, 0, 0);
      fclose(ofile);
    }
  }

    
  return 0;
}
