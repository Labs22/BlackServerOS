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
#include <string.h>
#include <stdio.h>
#include <math.h>
#ifdef __NetBSD__
#include <sys/param.h>
#endif
#ifdef WIN32
#define WINVER 0x0500
#include <windows.h>
#else
#if HAVE_STRUCT_SYSINFO
#include <sys/sysinfo.h>
#elif HAVE_SYS_SYSCTL_H
#include <sys/types.h>
#include <sys/sysctl.h>
#endif
#endif
#ifdef sun
#include <kstat.h>
#include <unistd.h>
#endif

#include "misc.h"
/*-------------------------------------------------------------------------*/
#if defined WIN32 || defined sun
char *strsep(char** stringp, const char* delim) {
  char *s;
  const char *spanp;
  int c, sc;
  char *tok;
  
  if ((s = *stringp) == NULL)
    return (NULL);
  for (tok = s;;) {
    c = *s++;
    spanp = delim;
    do {
      if ((sc = *spanp++) == c) {
	if (c == 0)
	  s = NULL;
	else
	  s[-1] = 0;
	*stringp = s;
	return (tok);
      }
    } while (sc != 0);
  }
  /* NOTREACHED */
}
#endif
/*-------------------------------------------------------------------------*/
uint64_t find_freeram(void) {
  uint64_t freeram  = 0;

#ifdef sun
  static long pagesize=0;
  static kstat_ctl_t *kc;
  static kstat_t *sys_pagesp;
  static size_t lotsfree;
  kstat_named_t *kn;
  size_t freemem;
  
  if (pagesize == 0) {
    /* Get the constant values (pagesize and lotsfree) once */
    pagesize = sysconf(_SC_PAGESIZE);
    if ((kc=kstat_open()) == 0) {
      perror("kstat_open");
      return 0;
    };
    if ((sys_pagesp=kstat_lookup(kc, "unix", 0, "system_pages")) == 0) {
      perror("kstat_lookup(system_pages)");
      return 0;
    };
    if (kstat_read(kc, sys_pagesp, 0) == -1) {
      perror("kstat_read(syspages)");
      return 0;
    };
    if ((kn=kstat_data_lookup(sys_pagesp, "lotsfree")) == 0) {
      perror("kstat_data_lookup(lotsfree)");
      return 0;
    };
    lotsfree = kn->value.ul;
  }
  
  if (kstat_read(kc, sys_pagesp, 0) == -1) {
    perror("kstat_read(syspages)");
    return 0;
  };
  if ((kn=kstat_data_lookup(sys_pagesp, "freemem")) == 0) {
    perror("kstat_data_lookup(freemem)");
    return 0;
  };
  freemem = kn->value.ul;
  freeram = freemem-lotsfree < 0 ? 0 /* avoid paging */ : pagesize*(freemem-lotsfree);
#elif !defined WIN32
  FILE *file = fopen("/proc/meminfo", "r");
  char buff[STR_BUFF_SIZE];

  /* Try to find the info we need from /proc/meminfo.
   * https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=34e431b0ae398fc54ea69ff85ec700722c9da773
   */

  if (file) {
    while (1) {
      char *r = fgets(buff, sizeof(buff), file);

      if (r == NULL) break;

      if (strncmp(buff, "MemFree", 7) == 0) {
        char *from = strchr(buff, ':');
        uint64_t mfree = 0;

        sscanf(from+1, "%lu", &mfree);
        freeram = mfree;
      }
      else if (strncmp(buff, "Cached", 6) == 0) {
        char *from = strchr(buff, ':');
        uint64_t mcached = 0;

        sscanf(from+1, "%lu", &mcached);
        freeram += mcached;

        break;
      }
      else if (strncmp(buff, "MemAvailable", 12) == 0) {
        char *from = strchr(buff, ':');
        sscanf(from+1, "%lu", &freeram);
        break;
      }
    }

    fclose(file);
    freeram = freeram << 10;
  }

  /* Otherwise, we try to guess. */
  else {

#if HAVE_STRUCT_SYSINFO
    struct sysinfo info;   
    sysinfo(&info);
    freeram = (uint64_t)info.freeram;
#elif HAVE_SYS_SYSCTL_H
    int mib[2] = {CTL_HW, HW_USERMEM}, mem;
    size_t len;
    len = sizeof(mem);
    if (sysctl(mib, 2, &mem, &len, NULL, 0) == 0) {
      freeram = mem;
    } else {
      return 0;
    }
#else
    /* unix system but we don't know how to get available RAM */
    return 0;
#endif
    
  }
#else
  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof(statex);
  GlobalMemoryStatusEx(&statex);
  uint64_t freephysram  = (uint64_t)statex.ullAvailPhys;
  uint64_t freevirtram = (uint64_t)statex.ullAvailVirtual;
  freeram = MY_MIN(freephysram, freevirtram);
#endif

  /* Take 90% of the available free RAM and return the resulting value
     in bytes (not kB). */
  /* Ensure that at least 150M stay free */

  if (freeram < 150*1024*1024)
    return 0;

  uint64_t ram = MY_MIN(floor(0.9 * freeram),
                        freeram - 150*1024*1024);

  return ram;
}
/*-------------------------------------------------------------------------*/
void convert_to_colon(uchar_t *input) {
  uchar_t *tmp;

  for (tmp = input; (*tmp = (*tmp == 193U ? ':' : *tmp)); ++tmp);
}
/*-------------------------------------------------------------------------*/
void convert_from_colon(uchar_t *input) {
  uchar_t *tmp;

  for (tmp = input; (*tmp = (*tmp == ':' ? 193U : *tmp)); ++tmp);
}
/*-------------------------------------------------------------------------*/
void wincp1252_to_ascii(uchar_t *str) {
  int len = strlen((char*)str);
  int i;

  for (i=0; i<len; i++)
    if (str[i] >= 128) 
      switch (str[i]) {
      case 0x8e: str[i] = 0xc4; break; // capital a umlaut
      case 0x99: str[i] = 0xd6; break; // capital o umlaut
      case 0x9a: str[i] = 0xdc; break; // capital u umlaut
      case 0xe1: str[i] = 0xdf; break; // double s (sharp/german s)
      case 0xa5: str[i] = 0xd1; break; // capital n tilda
      case 0x90: str[i] = 0xc9; break; // capital e accute
      case 0x80: str[i] = 0xc7; break; // capital c cedilla
      }
}
/*-------------------------------------------------------------------------*/
void fprintf_hex(FILE *file, char *str, int len) {
  int i;

  fprintf(file, "0x");

  for (i=0; i<len; ++i) 
    fprintf(file, "%2x", str[i]);
}
/*-------------------------------------------------------------------------*/
int categorize_password(char *pwd) {
  int i;
  int category = 0;

  for (i=0; i<strlen(pwd); i++) {
    if (pwd[i] >= 'a' && pwd[i] <= 'z') category |= 1;
    else if (pwd[i] >= 'A' && pwd[i] <= 'Z') category |= 2;
    else if (pwd[i] >= '0' && pwd[i] <= '9') category |= 4;
    else category |= 8;
  }
  return category;
}
/*-------------------------------------------------------------------------*/
const char *category_string(int category) {
  switch (category) {
  case 0: return "empty";
  case 1: return "lowalpha";
  case 5: return "lowalphanum";
  case 2: return "upalpha";
  case 6: return "upalphanum";
  case 3: return "mixedalpha";
  case 7: return "mixedalphanum";
  case 4: return "num";
  case 8: return "special";
  case 9: return "lowalpha+special";
  case 13: return "lowalphanum+special";
  case 10: return "upalpha+special";
  case 14: return "upalphanum+special";
  case 11: return "mixedalpha+special";
  case 15: return "mixedalphanum+special";
  case 12: return "num+special";
  }

  return "unknown";
}
