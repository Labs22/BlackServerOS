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
#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include <sys/mman.h>
#endif
#include <unistd.h>
#include <string.h>

#include "misc.h"
#include "table.h"
/*-------------------------------------------------------------------------*/
table_t *table_alloc(uint32_t code, char *path, int idx) {
  table_t *tbl = (table_t*)malloc(sizeof(table_t));

  tbl->kind    = table_kind(code);
  tbl->code    = code;

  tbl->path    = strdup(path);
  tbl->name    = strdup(table_string(tbl->kind));

  tbl->id      = 0;
  tbl->idx     = idx;
  tbl->cmin    = 0;
  tbl->cmax    = 0;
  tbl->ncols   = 0;
  tbl->offset  = 0;
  tbl->enabled = 0;
  tbl->active  = 0;

  tbl->idxfile = 0;
  tbl->endfile = 0;
  tbl->srtfile = 0;

  tbl->shared_init = 0;

  tbl->inisize = 0;
  tbl->idxsize = 0;
  tbl->endsize = 0;
  tbl->srtsize = 0;
  tbl->sizes   = NULL;

  tbl->param   = NULL;
  tbl->idxmem  = 0;
  tbl->endmem  = 0;
  tbl->srtmem  = 0;

  tbl->init    = NULL;
  tbl->cleanup = NULL;
  tbl->find    = NULL;

  tbl->lookup_idx = NULL;
  tbl->lookup_end = NULL;
  tbl->lookup_srt = NULL;

  tbl->check   = NULL;
  tbl->isvalid = NULL;

  return tbl;
}
/*-------------------------------------------------------------------------*/
void table_free(table_t *tbl) {
  if (tbl->path) free(tbl->path);
  if (tbl->name) free(tbl->name);

  if (tbl->srtfile) fclose(tbl->srtfile);
  if (tbl->idxfile) fclose(tbl->idxfile);
  if (tbl->endfile) fclose(tbl->endfile);

#ifndef WIN32
  if (tbl->srtmem) munmap(tbl->srtmem, tbl->srtsize);
  if (tbl->idxmem) munmap(tbl->idxmem, tbl->idxsize);
  if (tbl->endmem) munmap(tbl->endmem, tbl->endsize);
#else
  if (tbl->srtmem) free(tbl->srtmem);
  if (tbl->idxmem) free(tbl->idxmem);
  if (tbl->endmem) free(tbl->endmem);
#endif

  free(tbl);
}
/*-------------------------------------------------------------------------*/
int table_load(table_t *tbl) {
  char fname[STR_BUFF_SIZE];
  char *path = tbl->path;
  int idx = tbl->idx;

  assert(tbl->srtfile == 0);
  assert(tbl->idxfile == 0);
  assert(tbl->endfile == 0);

  snprintf(fname, sizeof(fname), "%s/table%d.start", path, idx);
  if ((tbl->srtfile = fopen(fname, "rb")) == 0) return 0;
  
  snprintf(fname, sizeof(fname), "%s/table%d.index", path, idx);
  if ((tbl->idxfile = fopen(fname, "rb")) == 0) return 0;
  
  snprintf(fname, sizeof(fname), "%s/table%d.bin", path, idx);
  if ((tbl->endfile = fopen(fname, "rb")) == 0) return 0;

  table_set_size(tbl);

  return 1;
}
/*-------------------------------------------------------------------------*/
void table_set_size(table_t *tbl) {
  int fd;
  struct stat buf;

  fd = fileno(tbl->srtfile);
  fstat(fd, &buf);
  tbl->srtsize = (uint64_t)buf.st_size;

  fd = fileno(tbl->idxfile);
  fstat(fd, &buf);
  tbl->idxsize = (uint64_t)buf.st_size;

  fd = fileno(tbl->endfile);
  fstat(fd, &buf);
  tbl->endsize = (uint64_t)buf.st_size;
}
/*-------------------------------------------------------------------------*/
int table_verify(table_t *tbl) {
  int idx = tbl->idx;
  const uint64_t *sizes = tbl->sizes;

  if (tbl->sizes == 0) return 0;

  if (idx < sizes[0]) {
    if (tbl->idxsize != sizes[1]) return -1;
    if (tbl->endsize != sizes[2*idx+2]) return -1;
    if (tbl->srtsize != sizes[2*idx+3]) return -1;
  } else return -1;

  return 1;
}
/*-------------------------------------------------------------------------*/
char *table_mmap(table_t *tbl, table_preload_t preload) {
  FILE *file = NULL;
  uint64_t size = 0;

  switch (preload) {
  case preload_idx:
    file = tbl->idxfile;
    size = tbl->idxsize;
    break;

  case preload_end:
    file = tbl->endfile;
    size = tbl->endsize;
    break;

  case preload_srt:
    file = tbl->srtfile;
    size = tbl->srtsize;
    break;

  default:
    assert(0);
  }

  char *mem = NULL;

#ifndef WIN32
  int prot  = PROT_READ | PROT_WRITE;
  int flags = MAP_PRIVATE | MAP_ANON;

  /* We use mmap instead of malloc because malloc might not be able to
     allocate the large amount of memory we request. */

  mem = (char*)mmap(0, size, prot, flags, -1, 0);

  if (mem == MAP_FAILED) {
    fprintf(stderr, 
	    "Problem while mapping memory for table %s,%d (preload=%d).\n",
	    tbl->name, tbl->idx, preload);
    return 0;
  }
#else
  mem = (char*)malloc(size);

  if (mem == 0) {
    fprintf(stderr,
	    "Malloc failed for table %s,%d (preload=%d).\n",
	    tbl->name, tbl->idx, preload);
    return 0;
  }
#endif

  rewind(file);

  /* Copy the content of the file at the mapped memory location. */

  if (fread(mem, 1, size, file) != size) {
    fprintf(stderr,
	    "Problem while preloading table %s,%d (preload=%d).\n",
	    tbl->name, tbl->idx, preload);
    perror(0);
#ifndef WIN32
    munmap(mem, size);
#else
    free(mem);
#endif
    return 0;
  }

  return mem;
}
/*-------------------------------------------------------------------------*/
uint64_t table_preload(table_t *tbl, table_preload_t preload) {
  uint64_t size = 0;

  /* tbl->param is used to determine whether the table has been
     initialised. Since a table for which tbl->init == NULL is
     implicitly considered initialised, we store a non NULL pointer
     into tbl->param to indicate that fact. */

  if ((preload & preload_init) && !tbl->param) {
    tbl->param = tbl->init ? tbl->init(tbl) : tbl;
    size += (preload & preload_init) ? tbl->inisize : 0;
  }

  if ((preload & preload_idx) && !tbl->idxmem) {
    tbl->idxmem = table_mmap(tbl, preload_idx);    
    size += tbl->idxsize;
  }

  if ((preload & preload_end) && !tbl->endmem) {
    tbl->endmem = table_mmap(tbl, preload_end);
    size += tbl->endsize;
  }
  
  if ((preload & preload_srt) && !tbl->srtmem) {
    tbl->srtmem = table_mmap(tbl, preload_srt);
    size += tbl->srtsize;
  }

  return size;
}
/*-------------------------------------------------------------------------*/
char *table_unmap(table_t *tbl, table_preload_t preload) {
  char *mem = NULL;
  uint64_t size = 0;

  switch (preload) {
  case preload_idx: 
    mem = tbl->idxmem; 
    size = tbl->idxsize;
    break;

  case preload_end: 
    mem = tbl->endmem; 
    size = tbl->endsize;
    break;

  case preload_srt: 
    mem = tbl->srtmem;
    size = tbl->srtsize;
    break;

  default: 
    assert(0);
  }

#ifndef WIN32
  munmap(mem, size);
#else
  free(mem);
#endif

  return NULL;
}
/*-------------------------------------------------------------------------*/
uint64_t table_unload(table_t *tbl, table_preload_t preload) {
  uint64_t size = 0;

  if ((preload & preload_init) && tbl->param) {
    if (tbl->cleanup) tbl->cleanup(tbl);
    tbl->param = NULL;
    size += tbl->inisize;
  }

  if ((preload & preload_idx) && tbl->idxmem) {
    tbl->idxmem = table_unmap(tbl, preload_idx);
    size += tbl->idxsize;
  }
 
  if ((preload & preload_end) && tbl->endmem) {
    tbl->endmem = table_unmap(tbl, preload_end);
    size += tbl->endsize;
  }
  
  if ((preload & preload_srt) && tbl->srtmem) {
    tbl->srtmem = table_unmap(tbl, preload_srt);
    size += tbl->srtsize;
  }

  return size;
}
/*-------------------------------------------------------------------------*/
uint64_t table_preload_size(table_t *tbl, table_preload_t preload) {
  uint64_t size = 0;

  if (preload & preload_init) size += tbl->inisize;
  if (preload & preload_idx)  size += tbl->idxsize;
  if (preload & preload_end)  size += tbl->endsize;
  if (preload & preload_srt)  size += tbl->srtsize;

  return size;
}
/*-------------------------------------------------------------------------*/
uint64_t table_size(table_t *tbl) {
  return table_preload_size(tbl, preload_full);
}
/*-------------------------------------------------------------------------*/
int table_open(list_t *tables, const char *dir, const char *tblstr) {
  /* Get the name of the table. */
  
  char name[STR_BUFF_SIZE];
  char path[STR_BUFF_SIZE];
  char *args;

  strncpy(name, tblstr, sizeof(name));
  args = strchr(name, ',');

  if (args) {
    args[0] = 0;
    ++args;
  }

  if (strlen(dir) > 0)
    snprintf(path, sizeof(path), "%s/%s", dir, name);
  else
    snprintf(path, sizeof(path), "%s", name);

  /* We need to open the 0th .bin file to find out the table type. */

  char buff[STR_BUFF_SIZE];
  FILE *file;

  snprintf(buff, sizeof(buff), "%s/table0.bin", path);
  file = fopen(buff, "rb");

  if (file == 0) return -1;
  
  /* Read the first 4 bytes of the .bin table. Important: make sure
     that the value read is in the correct byte order (little endian). */
    
  uint32_t code;
  
  fread(&code, sizeof(code), 1, file);
  code = ftohl(code);
  
  fclose(file);
    
  /* Load all the table we find. */
  
  int ntables = 0;
  int i;
  
  for (i=0;; ++i) {
    table_t *tbl = table_alloc(code, path, i);
    
    if (table_load(tbl)) {
      list_add_tail(tables, tbl);
      ++ntables;
    } else
      break;
  }


  /* If no specific tables have been requested, then we enable all of
     them. */
  
  list_nd_t *nd = 0;
  int nenabled  = 0;

  if (args == 0)
    for (i=0, nd = tables->tail; i<ntables; ++i, nd = nd->prev) {
      table_t *tbl = nd->data;
      tbl->enabled = 1;
      ++nenabled;
    }

  /* If the first argument is a '-', then we do not enable any
     table. Otherwise, we enable only the ones which have been
     requested. */

  else if (args[0] != '-'){
    table_t **ptr = (table_t**)calloc(ntables, sizeof(table_t*));
    char *p;

    for (i=0, nd = tables->tail; i<ntables; ++i, nd = nd->prev) {
      table_t *tbl = nd->data;

      assert(tbl->idx >= 0 && tbl->idx < ntables);
      ptr[tbl->idx] = tbl;
    }
    
    while ((p = strsep(&args, ",")) != 0 && strcmp(p, "") != 0) {
      int idx = atoi(p);

      if (idx >= 0 && idx < ntables && ptr[idx] != 0) {
	ptr[idx]->enabled = 1;
	++nenabled;
      } else
	return -(idx+2);
    }

    free(ptr);
  }

  return nenabled;
}
/*-------------------------------------------------------------------------*/
table_preload_t table_preload_state(table_t *tbl) {
  table_preload_t state = preload_none;

  if (tbl->param)  state += preload_init;
  if (tbl->idxmem) state += preload_idx;
  if (tbl->endmem) state += preload_end;
  if (tbl->srtmem) state += preload_srt;

  return state;
}
/*-------------------------------------------------------------------------*/
table_kind_t table_kind(uint32_t code) {
  switch (code) {
  case 0x3cc21790: return lmalphanum10k;
  case 0x0fa2031c: return lmalphanum5k;
  case 0x0e3402df: return lmextended;
  case 0x103e02a6: return lmgermanv1;
  case 0x10cb073e: return lmgermanv2;
  case 0xc7ed7df5: return ntextended;
  case 0x16501507: return ntdict;
  case 0x2ee9fa88: return ntnine;
  case 0x1990057f: return nteight;
  case 0xdf28cff0: return ntnum;
  case 0x9542377a: return ntseven;
  case 0x05080366: return lmflash;
  case 0x229e1899: return nteightxl;
  case 0x82506b6e: return ntspecialxl;
  case 0xbb00950e: return ntprobafree;
  case 0x47000fdf: return ntproba10g;
  case 0x2c002335: return ntproba60g;
  default: return unknown;
  }
}
/*-------------------------------------------------------------------------*/
const char *table_string(table_kind_t kind) {
  switch (kind) {
  case lmalphanum10k: return "XP free small";
  case lmalphanum5k: return "XP free fast";
  case lmextended: return "XP special";
  case lmgermanv1: return "XP german v1";
  case lmgermanv2: return "XP german v2";
  case ntextended: return "Vista special";
  case ntdict: return "Vista free";
  case ntnine: return "Vista nine";
  case nteight: return "Vista eight";
  case ntnum: return "Vista num";
  case ntseven: return "Vista seven";
  case lmflash: return "XP flash";
  case nteightxl: return "Vista eight XL";
  case ntspecialxl: return "Vista special XL";
  case ntprobafree: return "Vista probabilistic free";
  case ntproba10g: return "Vista probabilistic 10G";
  case ntproba60g: return "Vista probabilistic 60G";
  case unknown: return "Unknown";
  }

  return "Unknown";
}
