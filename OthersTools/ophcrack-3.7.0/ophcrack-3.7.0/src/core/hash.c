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
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <regex.h>

#include "hash.h"
#include "samdump2.h"
/*-------------------------------------------------------------------------*/
hash_t *hash_alloc(hash_kind_t kind, int hlen, int plen, int id) {
  hash_t *hsh = (hash_t*)malloc(sizeof(hash_t));

  hsh->kind = kind; 
  hsh->hash = (uchar_t*)malloc(hlen*sizeof(uchar_t));
  hsh->pwd  = (char*)malloc(plen*sizeof(char));
  hsh->str  = 0;
  hsh->status = 0;

  memset(hsh->hash, 0, hlen);
  memset(hsh->pwd, 0, plen);
  memset(hsh->info, 0, 64);

  hsh->lmhsh1 = 0;
  hsh->lmhsh2 = 0;

  hsh->id     = id;
  hsh->uid    = 0;
  hsh->done   = 0;
  hsh->tables = list_alloc();
  hsh->tnd    = 0;

  hsh->length   = 0;
  hsh->category = 0;
  hsh->time     = -1;
  hsh->table    = 0;
  
  return hsh;
}
/*-------------------------------------------------------------------------*/
void hash_free(hash_t *hsh) {
  free(hsh->hash);
  free(hsh->pwd);
  if (hsh->str) free(hsh->str);
  if (hsh->status) free(hsh->status);
  if (hsh->table) free(hsh->table);

  while (hsh->tables->size > 0) {
    htbl_t *htbl = list_rem_head(hsh->tables);
    free(htbl);
  }

  list_free(hsh->tables);
  free(hsh);
}
/*-------------------------------------------------------------------------*/
void hash_add_table(hash_t *hsh, table_t *tbl) {
  htbl_t *htbl = (htbl_t*)malloc(sizeof(htbl_t));
  const char *status = hsh->status;

  /* Fill in the htbl_t data structure for the given table. */

  htbl->tbl = tbl;
  htbl->col = tbl->ncols-1;
  htbl->covered = tbl->ncols;

  /* Look for the table in the status string. */

  if (status != 0) {
    char *buff = strdup(status);
    char *tmp  = buff;

    /* Compile the regexp. */
    
    static const char pattern[] = "([^,]+),([0-9]+)\\(([0-9]+)\\)";
    
    regex_t regex;
    regmatch_t match[4];
    
    regcomp(&regex, pattern, REG_EXTENDED);

    while (regexec(&regex, tmp, 4, match, 0) == 0) {
      if (match[1].rm_so == -1 || 
	  match[2].rm_so == -1 || 
	  match[3].rm_so == -1) 
	break;

      tmp[match[1].rm_eo] = 0;
      tmp[match[2].rm_eo] = 0;
      tmp[match[3].rm_eo] = 0;

      const char *name = tmp + match[1].rm_so;
      int idx = atoi(tmp + match[2].rm_so);

      if (idx == tbl->idx && strcmp(name, table_string(tbl->kind)) == 0) {
	htbl->covered = atoi(tmp + match[3].rm_so);
	htbl->col = htbl->covered - 1;
	break;
      }

      tmp += match[3].rm_eo+1;
    }

    free(buff);
    regfree(&regex);
  }

  list_add_tail(hsh->tables, htbl);
}
/*-------------------------------------------------------------------------*/
int hash_extract_lmnt(char *buff, list_t *hashes, int id) {
  /* Split the buffer according to the positions of ':'. */

  int l;
  char *p[11] = {0};

  for (l=0; l<11; ++l)
    if ((p[l] = strsep(&buff, ":")) == 0) break;

  /* Copy the LM and NT hashes. */

  char lmstr[33] = {0};
  char ntstr[33] = {0};

  /* PWDUMP formatted string. */

  if (l >= 7) {
    if ((strcmp(p[2], pwdump_nopwd) != 0) &&
	(strcmp(p[2], samdump_nopwd) != 0))
      strncpy(lmstr, p[2], sizeof(lmstr));
    else 
      strncpy(lmstr, "aad3b435b51404eeaad3b435b51404ee", sizeof(lmstr));

    if ((strcmp(p[3], pwdump_nopwd) != 0) &&
	(strcmp(p[3], samdump_nopwd) != 0))
      strncpy(ntstr, p[3], sizeof(ntstr));
    else
      strncpy(ntstr, "31d6cfe0d16ae931b73c59d7e0c089c0", sizeof(ntstr));
  } 

  /* A pair of LM and NT hashes. */

  else if (l == 2) {
    if ((strcmp(p[0], pwdump_nopwd) != 0) &&
	(strcmp(p[0], samdump_nopwd) != 0))
      strncpy(lmstr, p[0], sizeof(lmstr));
    else 
      strncpy(lmstr, "aad3b435b51404eeaad3b435b51404ee", sizeof(lmstr));

    if ((strcmp(p[1], pwdump_nopwd) != 0) &&
	(strcmp(p[1], samdump_nopwd) != 0))
      strncpy(ntstr, p[1], sizeof(ntstr));
    else
      strncpy(ntstr, "31d6cfe0d16ae931b73c59d7e0c089c0", sizeof(ntstr));
  } 

  /* A single LM hash. */

  else if (l == 1) {
    if ((strcmp(p[0], pwdump_nopwd) != 0) &&
	(strcmp(p[0], samdump_nopwd) != 0))
      strncpy(lmstr, p[0], sizeof(lmstr));
    else 
      strncpy(lmstr, "aad3b435b51404eeaad3b435b51404ee", sizeof(lmstr));
  } 

  /* Unknown format. */

  else
    return 0;

  /* Check that the hashes have the correct length. */

  int lmlen = strlen(lmstr);
  int ntlen = strlen(ntstr);

  if (lmlen != 32 && ntlen != 32) return 0; 
/*   if (l == 1 && lmlen == 0) return 0; */
/*   if (lmlen > 0 && lmlen != 32) return 0; */
/*   if (ntlen > 0 && ntlen != 32) return 0; */
  
  /* Extract the LM hashes. */

  hash_t *lmhsh1 = 0;
  hash_t *lmhsh2 = 0;

  int lmplen1 = l == 7 ? strlen(p[4]) : 8;
  int lmplen2 = l == 7 ? strlen(p[5]) : 8;
  
  if (strcmp(lmstr, "") != 0) {
    lmhsh1 = hash_alloc(lm1, 8, lmplen1 < 8 ? 8 : lmplen1, id);
    lmhsh2 = hash_alloc(lm2, 8, lmplen2 < 8 ? 8 : lmplen2, id);

    lmhsh1->str = strdup(lmstr);
    lmhsh2->str = strdup(lmstr);

    int i;
    uint32_t tmp;
    
    for (i=0; i<8; ++i) {
      if (sscanf(lmstr+2*i, "%2x", &tmp) != 1) return 0;
      lmhsh1->hash[i] = (uchar_t)tmp;
      
      if (sscanf(lmstr+2*i+16, "%2x", &tmp) != 1) return 0;
      lmhsh2->hash[i] = (uchar_t)tmp;
    }
  }

  /* Extract the NT hash. */

  hash_t *nthsh = 0;
  int ntplen = l == 7 ? strlen(p[6]) : 16;

  if (strcmp(ntstr, "") != 0) {
    nthsh = hash_alloc(nt, 16, ntplen < 16 ? 16 : ntplen, id);
    nthsh->str = strdup(ntstr);

    int i;
    uint32_t tmp;

    for (i=0; i<16; ++i) {
      if (sscanf(ntstr+2*i, "%2x", &tmp) != 1) return 0;
      nthsh->hash[i] = (uchar_t)tmp;
    }
  }

  /* Store the user name, id and password if provided. */

  if (l >= 7) {
    int uid = atoi(p[1]);

    if (lmhsh1) {
      strncpy(lmhsh1->info, p[0], sizeof(lmhsh1->info));
      lmhsh1->uid = uid;

      if (lmplen1 < 8) 
	strncpy(lmhsh1->pwd, p[4], lmplen1);
      convert_to_colon((uchar_t*)lmhsh1->pwd);
	

      if (l >= 8)
	lmhsh1->status = strdup(p[7]);
    }

    if (lmhsh2) {
      strncpy(lmhsh2->info, p[0], sizeof(lmhsh2->info));
      lmhsh2->uid = uid;

      if (lmplen2 < 8) 
	strncpy(lmhsh2->pwd, p[5], lmplen2);
      convert_to_colon((uchar_t*)lmhsh2->pwd);
      

      if (l >= 9)
	lmhsh2->status = strdup(p[8]);
    }

    if (nthsh) {
      strncpy(nthsh->info, p[0], sizeof(nthsh->info));
      nthsh->uid  = uid;

      strncpy(nthsh->pwd, p[6], ntplen);
      convert_to_colon((uchar_t*)nthsh->pwd);

      if (l >= 10)
	nthsh->status = strdup(p[9]);
    }
  }

  /* Check for empty hashes. */

  int empty1 = 0;
  int empty2 = 0;

  if (lmhsh1 && memcmp(lmhsh1->hash, empty_lmhash, 8) == 0) {
    empty1 = 1;
    lmhsh1->done = 1;
  }

  if (lmhsh2 && memcmp(lmhsh2->hash, empty_lmhash, 8) == 0) {
    empty2 = 1;
    lmhsh2->done = 1;
  }

  if (nthsh && memcmp(nthsh->hash, empty_nthash, 16) == 0)
    nthsh->done = 1;

  /* Check for non empty passwords. */

  if (lmhsh1 && lmhsh1->pwd[0] != 0)
    lmhsh1->done = 1;

  if (lmhsh2 && lmhsh2->pwd[0] != 0)
    lmhsh2->done = 1;

  if (nthsh && nthsh->pwd[0] != 0)
    nthsh->done = 1;

  /* If both LM hashes are empty, then its like if we do not have any
     LM at all. */

  if (empty1 && empty2) {
    hash_free(lmhsh1);
    hash_free(lmhsh2);

    lmhsh1 = 0;
    lmhsh2 = 0;
  }

  /* Each 1st/2nd LM hash keeps a pointer on the associated NT and
     2nd/1st LM hashes. */

  if (lmhsh1) {
    lmhsh1->lmhsh1 = nthsh;
    lmhsh1->lmhsh2 = lmhsh2;
  }

  if (lmhsh2) {
    lmhsh2->lmhsh1 = lmhsh1;
    lmhsh2->lmhsh2 = nthsh;
  }

  /* The NT hash keeps a pointer on both LM hashes. */

  if (nthsh) {
    nthsh->lmhsh1 = lmhsh1;
    nthsh->lmhsh2 = lmhsh2;
  }

  /* Add the 1st LM, 2nd LM and NT hashes to the list of hashes. */

  if (lmhsh1) list_add_tail(hashes, lmhsh1);
  if (lmhsh2) list_add_tail(hashes, lmhsh2);
  if (nthsh) list_add_tail(hashes, nthsh);

  return 1;
}
/*-------------------------------------------------------------------------*/
int hash_load_pwdump(list_t *hashes, FILE *file, int id) {
  char buff[STR_BUFF_SIZE];

  /* Read and parse each entry of the hash file. */
  
  int npwds = 0;
  int n;  

  for (n=0;; ++n) {
    if (fgets(buff, sizeof(buff), file) == 0) break;
    
    /* Remove a potential (\r)\n at the end of the line. */
    
    int len = strlen(buff);
    if (buff[len-1] == '\n') buff[len-1] = 0;
    if (buff[len-2] == '\r') buff[len-2] = 0;
    
    /* Parse the line according to the type of hash we want to crack. */
    
    if (hash_extract_lmnt(buff, hashes, id) > 0) {
      ++id;
      ++npwds;
    }
  }

  return npwds;
}
/*-------------------------------------------------------------------------*/
int hash_load_sam(list_t *hashes, const char *dir, int id) {
  static const char *name[2] = { "SYSTEM", "SAM" };
  static const int da = 'a' - 'A';
  
  /* Try to find the SYSTEM and SAM file by enumerating all the
     possible upper/lower case variations of both file names. */

  char fname[2][STR_BUFF_SIZE] = { {0}, {0} };
  int i, j;

  for (i=0; i<2; ++i) {
    int len = strlen(name[i]);
    int count = 1 << len;
    char tmp[10] = {0};

    while (count--) {
      for (j=0; j<len; ++j) {
	tmp[j] = name[i][j];
	if ((count >> j) & 1) tmp[j] += da;
      }

      snprintf(fname[i], sizeof(fname[i]), "%s/%s", dir, tmp);

      if (access(fname[i], R_OK) != 0) 
	fname[i][0] = 0;
      else
	break;
    }
  }

  /* If we did not find both files, then we stop. */

  char *sys = fname[0];
  char *sam = fname[1];

  if (sys[0] == 0) return -1;
  if (sam[0] == 0) return -2;

  /* Otherwise, we extract the hashes. */

  uchar_t pkey[0x10];
  char error[512];
  
  if (bkhive((uchar_t*)sys, pkey, error, 0) == -1) return -3;

  /* Retrieve the hashes. */

  list_t *hstr = list_alloc();
  list_nd_t *nd;

  int ret = samdump2((uchar_t*)sam, hstr, pkey, error, 0, 0, 0);
  int npwds = 0;

  if (ret != -1)
    for (nd = hstr->head; nd != 0; nd = nd->next) {
      char *str = (char*)nd->data;
      
      if (hash_extract_lmnt(str, hashes, id)) {
	++id;
	++npwds;
      }
    }

  list_free(hstr);

  return ret != -1 ? npwds : -4;
}
/*-------------------------------------------------------------------------*/
int hash_dump_sam(list_t *hashes, int id) {

  int npwds = 0;

#ifdef WIN32
  char error[512]; 
  uchar_t pkey[0x10];
  uchar_t *buff_sam;
  int size = 0;
  
  if (get_live_syskey(pkey, error, 0) == -1) 
    return -1;
  
  if (get_sam(&buff_sam, &size, error, 0) == -1)
    return -2;
  
  list_t *hstr = list_alloc();
  list_nd_t *nd;
  
  if (samdump2(buff_sam, hstr, pkey, error, 0, 1, size) == -1) {
    list_free(hstr);
    return -3;  
  }

  for (nd = hstr->head; nd != 0; nd = nd->next) {
    char *str = (char*)nd->data;
    
    if (hash_extract_lmnt(str, hashes, id)) {
      ++id;
      ++npwds;
    }
  }

  list_free(hstr);

#endif
  
  return npwds;
  
}
/*-------------------------------------------------------------------------*/
void hash_print(hash_t *hsh, FILE *file, int nice, int status, int hide) {
  hash_kind_t kind = hsh->kind;
  
  /* Find the associated LM and NT hashes. */

  hash_t *lmhsh1 = 0;
  hash_t *lmhsh2 = 0;
  hash_t *nthsh  = 0;

  switch (kind) {
  case lm1:
    lmhsh1 = hsh;
    lmhsh2 = hsh->lmhsh2;
    nthsh  = hsh->lmhsh1;
    break;

  case lm2:
    lmhsh1 = hsh->lmhsh1;
    lmhsh2 = hsh;
    nthsh  = hsh->lmhsh2;
    break;

  case nt:
    lmhsh1 = hsh->lmhsh1;
    lmhsh2 = hsh->lmhsh2;
    nthsh  = hsh;
    break;

  default:
    break;
  }

  int uid    = hsh->uid;
  char *info = hsh->info;

  /* Print in pwdump format. */

  if (nice == 0) {
    /* User name, user id, LM hash and NT hash. */

    if (info[0]) {
      if (hide)
	fprintf(file, "*****");
      else
	fprintf(file, "%s", info);
    }
    fprintf(file, ":");
    
    if (uid) fprintf(file, "%d", uid);
    fprintf(file, ":");
    
    if (lmhsh1) fprintf(file, "%s", lmhsh1->str);
    fprintf(file, ":");
    
    if (nthsh) fprintf(file, "%s", nthsh->str);
    fprintf(file, ":");
    
    /* Both LM passwords and the NT password. */

    if (lmhsh1) {
      convert_from_colon((uchar_t*)lmhsh1->pwd);
      fprintf(file, "%s", lmhsh1->pwd);
      convert_to_colon((uchar_t*)lmhsh1->pwd);
    }
    
    fprintf(file, ":");
    
    if (lmhsh2) {
      convert_from_colon((uchar_t*)lmhsh2->pwd);
      fprintf(file, "%s", lmhsh2->pwd);
      convert_to_colon((uchar_t*)lmhsh2->pwd);
    }
    
    fprintf(file, ":");
    
    if (nthsh) {
      convert_from_colon((uchar_t*)nthsh->pwd);
      fprintf(file, "%s", nthsh->pwd);
      convert_to_colon((uchar_t*)nthsh->pwd);
    }

    if (status) {
      fprintf(file, ":");

      /* Status of the search in the tables for the 1st LM hash. */

      if (lmhsh1) {
	list_t *tables = lmhsh1->tables;
	list_nd_t *nd;
	
	for (nd = tables->head; nd != 0; nd = nd->next) {
	  htbl_t *htbl = nd->data;
	  table_t *tbl = htbl->tbl;
	  
	  int idx = tbl->idx;
	  int covered  = htbl->covered;
	  const char *name = table_string(tbl->kind);
	  
	  fprintf(file, "%s,%d(%d)", name, idx, covered); 
	}
      }

      /* Status of the search in the tables for the 2nd LM hash. */

      fprintf(file, ":");

      if (lmhsh2) {
	list_t *tables = lmhsh2->tables;
	list_nd_t *nd;
	
	for (nd = tables->head; nd != 0; nd = nd->next) {
	  htbl_t *htbl = nd->data;
	  table_t *tbl = htbl->tbl;

	  int idx = tbl->idx;
	  int covered  = htbl->covered;
	  const char *name = table_string(tbl->kind);
	  
	  fprintf(file, "%s,%d(%d)", name, idx, covered); 
	}
      }
      
      /* Status of the search in the tables for the NT hash. */
      
      fprintf(file, ":");

      if (nthsh) {
	list_t *tables = nthsh->tables;
	list_nd_t *nd;
	
	for (nd = tables->head; nd != 0; nd = nd->next) {
	  htbl_t *htbl = nd->data;
	  table_t *tbl = htbl->tbl;
	  
	  int idx = tbl->idx;
	  int covered  = htbl->covered;
	  const char *name = table_string(tbl->kind);
	  
	  fprintf(file, "%s,%d(%d)", name, idx, covered); 
	}
      }
    }
  }

  /* Print in a human readable format. */

  else {
    if (info[0])
      if (hide)
	fprintf(file, "*****");
      else
	fprintf(file, "%-32s", info);
    else {
      if (nthsh)
	fprintf(file, "%-32s", nthsh->str);
      else 
	if (lmhsh1) fprintf(file, "%-32s", lmhsh1->str);
    }
    fprintf(file, " ");
    
    if (lmhsh1) {
      if (lmhsh1->done) {
	convert_from_colon((uchar_t*)lmhsh1->pwd);
	fprintf(file, "%-7s", lmhsh1->pwd);
	convert_to_colon((uchar_t*)lmhsh1->pwd);
      } else fprintf(file, ".......");
	  
      if (lmhsh2 && lmhsh2->done) {
	convert_from_colon((uchar_t*)lmhsh2->pwd);
	fprintf(file, "%-7s", lmhsh2->pwd);
	convert_to_colon((uchar_t*)lmhsh2->pwd);
      } else fprintf(file, ".......");
    } else fprintf(file, "*** empty *** ");

    fprintf(file, " ");

    if (nthsh) {
      if (nthsh->done) {
	convert_from_colon((uchar_t*)nthsh->pwd);
	fprintf(file, "%s", nthsh->pwd);
	convert_to_colon((uchar_t*)nthsh->pwd);
	if (nthsh->pwd[0] == 0) fprintf(file, "*** empty ***");
      } else fprintf(file, ".......");
    } else fprintf(file, "*** empty ***");
  }
  
  fprintf(file, "\n");
}
