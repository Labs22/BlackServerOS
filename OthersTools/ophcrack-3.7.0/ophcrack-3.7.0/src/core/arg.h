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
#ifndef ARG_H
#define ARG_H

#include <stdio.h>
#include "list.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct arg_t_ {
  int verbose;
  int showempty;
  int debug;
  int quiet;
  int run;
  int use_gui;
  int nthreads;
  int nhredux;
  int mdqueue;
  int bforce;
  int bforce_len;
  int hideuname;
  int auditmode;
  int stats;
  int ssave;
  int exportcsv;
  int preload;

  char *cfname;
  char *hfname;
  char *ofname;
  char *xfname;
  char *tblstr;
  char *samdir;
  char *sfname;
  char *lfname;

  FILE *lfile;

  list_t *table_path;
  list_t *table_str_conf;
  list_t *table_str_cmd;
} arg_t;

arg_t *arg_alloc(void);
void arg_free(arg_t *arg);
void arg_default(arg_t *arg);
int arg_read_conf(arg_t *arg);

#ifdef  __cplusplus
}
#endif
#endif
