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
#include <stdlib.h>
#include <assert.h>
#include <regex.h>
#include <string.h>

#include "arg.h"
#include "misc.h"
/*-------------------------------------------------------------------------*/
arg_t *arg_alloc(void) {
  arg_t *arg = (arg_t*)malloc(sizeof(arg_t));;

  arg_default(arg);

  arg->verbose   = 0;
  arg->showempty = 1;
  arg->debug     = 0;
  arg->quiet     = 0;
  arg->run       = 0;
  arg->exportcsv = 0;
  arg->preload   = 3;


#ifdef HAVE_GUI
  arg->use_gui   = 1;
#else
  arg->use_gui   = 0;
#endif

  arg->cfname = 0;
  arg->hfname = 0;
  arg->ofname = 0;
  arg->xfname = 0;
  arg->sfname = 0;
  arg->lfname = 0;

  arg->tblstr = 0;
  arg->samdir = 0;
  arg->stats  = 0;
  arg->ssave  = 0;

  arg->lfile = stdout;

  arg->table_path = list_alloc();
  arg->table_str_conf = list_alloc();
  arg->table_str_cmd  = list_alloc();

  return arg;
}
/*-------------------------------------------------------------------------*/
void arg_free(arg_t *arg) {
  list_free(arg->table_path);
  list_free(arg->table_str_conf);
  list_free(arg->table_str_cmd);
  free(arg);
}
/*-------------------------------------------------------------------------*/
void arg_default(arg_t *arg) {
#ifdef MY_CPU_COUNT
  arg->nthreads = MY_CPU_COUNT;
#else
  arg->nthreads = 2;
#endif
  arg->nhredux  = 50000;
  arg->mdqueue  = 500;
  arg->bforce   = 1;
  arg->bforce_len = 4;
  arg->hideuname = 0;
  arg->auditmode = 0;
}
/*-------------------------------------------------------------------------*/
int arg_read_conf(arg_t *arg) {
  assert(arg->cfname != 0);

  FILE *cfile = fopen(arg->cfname, "r");
  if (cfile == 0) return -1;

  static const char *pattern[11] = {
    "^[ \t]*#",
    "^[ \t]*$",
    "^[ \t]*table[ \t]+([^\t]+)[ \t]*$",
    "^[ \t]*nthreads[ \t]+([^ \t]+)[ \t]*$",
    "^[ \t]*nhredux[ \t]+([^ \t]+)[ \t]*$",
    "^[ \t]*maxdqueue[ \t]+([^ \t]+)[ \t]*$",
    "^[ \t]*bforce[ \t]+([^ \t]+)[ \t]*$",
    "^[ \t]*session_file[ \t]+([^ \t]+)[ \t]*$",
    "^[ \t]*session_save[ \t]+([^ \t]+)[ \t]*$",
    "^[ \t]*hideuname[ \t]+([^ \t]+)[ \t]*$",
    "^[ \t]*auditmode[ \t]+([^ \t]+)[ \t]*$"};

  regex_t regex[11];
  regmatch_t match[2];

  /* Compile the regular expressions. */

  int n = sizeof(regex) / sizeof(regex_t);
  int i;  

  for (i=0; i<n; ++i)
    regcomp(regex+i, pattern[i], REG_EXTENDED);

  /* Parse each line of the config file. */

  char buff[STR_BUFF_SIZE];
  char tmp[STR_BUFF_SIZE];
  int count = 0;

  while (fgets(buff, sizeof(buff), cfile) != 0 && ++count) {
    /* Remove the newline. */

    int len = strlen(buff);
    if (buff[len-1] == '\n') buff[len-1] = '\0';

    /* Test all the regexs to see if one match. */

    for (i=0; i<n; ++i)
      if (regexec(regex+i, buff, 2, match, 0) == 0) break;
    
    /* Nothing matched. We have found an invalid parameter. */

    if (i == n) {
      printf("Invalid parameter at line %d of %s\n", count, arg->cfname);
      continue;
    }
    
    /* Skip comments and empty lines. */

    if (i < 2) continue;
    
    /* Hande the parameters. */

    int so = match[1].rm_so;
    int eo = match[1].rm_eo;

    len = eo-so+1;
    snprintf(tmp, len+1, "%s", buff+so);
    
    /* table */

    if (i == 2)
      list_add_tail(arg->table_str_conf, strdup(tmp));

    /* nthreads */

    else if (i == 3)
      arg->nthreads = atoi(tmp);

    /* nhredux */
    
    else if (i == 4)
      arg->nhredux = atoi(tmp);

    /* maxdqueue */

    else if (i == 5)
      arg->mdqueue = atoi(tmp);

    /* bforce */

    else if (i == 6)
      arg->bforce = atoi(tmp);

    /* session file */

    else if (i == 7)
      arg->sfname = strdup(tmp);
    
    /* session save */

    else if (i == 8)
      arg->ssave = atoi(tmp);

    /* hide usernames */
    else if (i == 9)
      arg->hideuname = atoi(tmp);

    /* audit mode */
    else if (i == 10)
      arg->auditmode = atoi(tmp);

  }

  for (i=0; i<n; ++i) regfree(regex+i);
  fclose(cfile);

  return 0;
}
