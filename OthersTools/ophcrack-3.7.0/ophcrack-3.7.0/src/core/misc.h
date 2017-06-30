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
#ifndef MISC_H
#define MISC_H

#include <stdio.h>
#include <stdint.h>
#include <config.h>

#include "bswap.h"

#define MY_MAX(a,b) ((a)>(b)?(a):(b))
#define MY_MIN(a,b) ((a)<(b)?(a):(b))

#define my_setbit(x,pos) x | (1 << pos);
#define my_getbit(x,pos) (x & (1 << pos)) ? 1 : 0

/** Maximum size of the buffers. */

#define STR_BUFF_SIZE 512

/** Maximum length of a password. */

#define MAX_PWD_LEN 50

/* Endianness conversion */

#ifdef WORDS_BIGENDIAN
#   define ftohl64(x) __bswap_64 (x)   /**< File (f) to host (h) int64.  */
#   define ftohl(x)   __bswap_32 (x)   /**< File (f) to host (h) int32.  */
#   define ftohs(x)   __bswap_16 (x)   /**< File (f) to host (h) short.  */
#else
#   define ftohl64(x) (x)            /**< File (f) to host (h) int64.  */
#   define ftohl(x)   (x)            /**< File (f) to host (h) int32.  */
#   define ftohs(x)   (x)            /**< File (f) to host (h) short.  */
#endif

#ifdef  __cplusplus
extern "C" {
#endif

typedef unsigned char uchar_t;

#if defined WIN32 || defined sun
char *strsep(char** stringp, const char* delim);
#endif

uint64_t find_freeram(void);
void convert_to_colon(uchar_t *input);
void convert_from_colon(uchar_t *input);
void wincp1252_to_ascii(uchar_t *str);
void fprintf_hex(FILE *file, char *str, int len);
int categorize_password(char *pwd);
const char *category_string(int category);

#ifdef __cplusplus
}
#endif
#endif
