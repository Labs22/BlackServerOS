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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <openssl/md4.h>

#ifndef WIN32
#include <netinet/in.h>
#else
#include <winsock.h>
#endif

#include "original.h"
#include "misc.h"
#include "hash.h"
/*-------------------------------------------------------------------------*/
int resolve_nt_hash(char p1[], char p2[], char h[], unsigned char pw[], 
		    int insist) {

  unsigned int i,j, combinations;
  unsigned int l, max = 0;
  unsigned char md4[17],*p, orig[15]={0};
  
  static unsigned char is_multi[256]={
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 8, 2, 2, 2, 8, 2, 2, 2, 8, 2, 2, 2, 2, 2, 8, 
    2, 2, 2, 2, 2, 8, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 

    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 2, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 1, 
    1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

  static unsigned char multi_char[256][12]= {
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, 
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, 
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, 
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, 
    {0}, {'A','a',0xe0,0xe2,0xc0,0xc2,0xe3,0xc3}, "Bb", "Cc", "Dd", 
    {'E','e',0xc8,0xe8,0xca,0xea,0xeb,0xcb}, "Ff", "Gg", "Hh", {'I','i',0xee,0xef,0xce,0xcf,0xed,0xcd},
    "Jj", "Kk", "Ll", "Mm", "Nn", {'O','o',0xf4,0xd4,0xf3,0xf5,0xd3,0xd5},
    "Pp", "Qq", "Rr", "Ss", "Tt", {'U','u',0xf9,0xfb,0xd9,0xdb,0xfa,0xda}, "Vv", 
    "Ww", "Xx", "Yy", "Zz", {0}, {0}, {0}, {0}, {0},
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, 
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, 
    
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, 
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, 
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, 
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, 
    {0}, {0}, {0}, {0}, {0xc4,0xe4}, {0}, {0}, {0xe7,0xc7}, {0}, {0xc9,0xe9}, {0}, {0}, {0}, {0}, {0}, {0}, 
    {0}, {0xd1,0xf1}, {0}, {0}, {0}, {0}, {0xd6,0xf6}, {0}, {0}, {0}, {0}, {0}, {0xdc,0xfc}, {0}, {0}, {0xdf}, 
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, 
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}} ;
  
  static int max_multi=8;
  
  memset(pw, 0, 16);
  strcpy((char*)pw,p1);
  strcpy((char*)(pw+7),p2);
  
  l=strlen((char*)pw);

  p = (unsigned char *)pw;
  for (i=0; i<l; i++)
    if (p[i]>=128) 
      switch(p[i]){
      case 0x8e: p[i]=0xc4;break; // capital a umlaut
      case 0x99: p[i]=0xd6;break; // capital o umlaut
      case 0x9a: p[i]=0xdc;break; // capital u umlaut  
      case 0xe1: p[i]=0xdf;break; // double s
      case 0xa5: p[i]=0xd1;break; // capital n tilda
      case 0x90: p[i]=0xc9;break; // capital e accute
      case 0x80: p[i]=0xc7;break; // capital c cedilla
      }

  strcpy((char*)orig, (char*)pw);
  combinations=1;

  for (i=0; i<l; i++)
    combinations *=is_multi[pw[i]];

  if (!insist) {
    max = max_multi;
    while (combinations > RESOLVE_MAX_COMBINATIONS) {
      max--;
      combinations = 1;
      for (i=0; i<l; i++)
	combinations *= MY_MIN(is_multi[pw[i]],max);
    }
  }
  
  for (i=0; i<combinations; i++) {
    int c,m;
    c=i;
    
    for (j=0; j<l; j++) {
      m = MY_MIN(is_multi[orig[j]],max);
      pw[j]=((m>1) ? multi_char[orig[j]][c%m]:orig[j]);
      c/=m;
    }
    md4[16]=0;
    make_nthash((char*)pw, (char*)md4);
 
    if (!memcmp(md4,h,16)) 
      return 1;    
  }

  memset(pw, 0, 16);
  return 0;    
}
/*-------------------------------------------------------------------------*/
void make_nthash(char *pw, char *out) {
  uchar_t *pwd = (uchar_t*)pw;

   /* Convert to unicode. */

  uint16_t unipwd[128] = {0};
  int len = strlen(pw);
  int i;

  for (i=0; i<len; ++i)
    unipwd[i] = htons(pwd[i] << 8);
  
  /* Compute MD4 of Unicode password. */

  MD4_CTX ctx;

  MD4_Init(&ctx);
  MD4_Update(&ctx, unipwd, len * sizeof(uint16_t));
  MD4_Final((uchar_t*)out, &ctx);
}
