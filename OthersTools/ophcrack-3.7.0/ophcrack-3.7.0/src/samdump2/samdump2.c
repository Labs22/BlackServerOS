/*  Samdump2
    Dump nt/lanman password hashes from a sam hive with Syskey enabled
    
    Thank to Dmitry Andrianov for the program name ^_^
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
   
    This program is released under the GPL with the additional exemption 
    that compiling, linking, and/or using OpenSSL is allowed.

    Copyright (C) 2004-2006 Nicola Cuomo <ncuomo@studenti.unina.it>
    Improvments and some bugs fixes by Objectif Securité
    <http://www.objectif-securite.ch>
*/

#include <stdio.h>
#include <stdlib.h>
#include <openssl/rc4.h>
#include <openssl/md5.h>
#include <openssl/des.h>
#if defined(__linux__)
#include <endian.h>
#elif defined(__FreeBSD__)
#include <machine/endian.h>
#endif
#include "hive.h"
#include "list.h"

#ifdef BYTE_ORDER
#if BYTE_ORDER == LITTLE_ENDIAN
#elif BYTE_ORDER == BIG_ENDIAN
#include <byteswap.h>
#else
#warning "Doesn't define a standard ENDIAN type"
#endif
#else
#warning "Doesn't define BYTE_ORDER"
#endif


/* Cut&Paste from pwdump source code */

/*
* Convert a 7 byte array into an 8 byte des key with odd parity.
*/

void str_to_key(unsigned char *str,unsigned char *key)
{
	// void des_set_odd_parity(des_cblock *);
	int i;

	key[0] = str[0]>>1;
	key[1] = ((str[0]&0x01)<<6) | (str[1]>>2);
	key[2] = ((str[1]&0x03)<<5) | (str[2]>>3);
	key[3] = ((str[2]&0x07)<<4) | (str[3]>>4);
	key[4] = ((str[3]&0x0F)<<3) | (str[4]>>5);
	key[5] = ((str[4]&0x1F)<<2) | (str[5]>>6);
	key[6] = ((str[5]&0x3F)<<1) | (str[6]>>7);
	key[7] = str[6]&0x7F;
	for (i=0;i<8;i++) {
		key[i] = (key[i]<<1);
	}
	DES_set_odd_parity((DES_cblock *)key);
}

/*
* Function to convert the RID to the first decrypt key.
*/

void sid_to_key1(unsigned long sid,unsigned char deskey[8])
{
	unsigned char s[7];

	s[0] = (unsigned char)(sid & 0xFF);
	s[1] = (unsigned char)((sid>>8) & 0xFF);
	s[2] = (unsigned char)((sid>>16) & 0xFF);
	s[3] = (unsigned char)((sid>>24) & 0xFF);
	s[4] = s[0];
	s[5] = s[1];
	s[6] = s[2];

	str_to_key(s,deskey);
}

/*
* Function to convert the RID to the second decrypt key.
*/

void sid_to_key2(unsigned long sid,unsigned char deskey[8])
{
	unsigned char s[7];

	s[0] = (unsigned char)((sid>>24) & 0xFF);
	s[1] = (unsigned char)(sid & 0xFF);
	s[2] = (unsigned char)((sid>>8) & 0xFF);
	s[3] = (unsigned char)((sid>>16) & 0xFF);
	s[4] = s[0];
	s[5] = s[1];
	s[6] = s[2];

	str_to_key(s,deskey);
}


/*
 * Convert UTF-16 to UTF-8
 */

unsigned char* utf16_to_utf8 (unsigned char *dest, unsigned short int *src, size_t size) {
  unsigned int code_high = 0;

  while (size--)
    {
      unsigned int code = *src++;

      if (code_high)
	{
	  if (code >= 0xDC00 && code <= 0xDFFF)
	    {
	      /* Surrogate pair.  */
	      code = ((code_high - 0xD800) << 12) + (code - 0xDC00) + 0x10000;
	      
	      *dest++ = (code >> 18) | 0xF0;
	      *dest++ = ((code >> 12) & 0x3F) | 0x80;
	      *dest++ = ((code >> 6) & 0x3F) | 0x80;
	      *dest++ = (code & 0x3F) | 0x80;
	    }
	  else
	    {
	      /* Error...  */
	      *dest++ = '?';
	    }

	  code_high = 0;
	}
      else
	{
	  if (code <= 0x007F)
	    *dest++ = code;
	  else if (code <= 0x07FF)
	    {
	      *dest++ = (code >> 6) | 0xC0;
	      *dest++ = (code & 0x3F) | 0x80;
	    }
	  else if (code >= 0xD800 && code <= 0xDBFF)
	    {
	      code_high = code;
	      continue;
	    }
	  else if (code >= 0xDC00 && code <= 0xDFFF)
	    {
	      /* Error... */
	      *dest++ = '?';
	    }
	  else
	    {
	      *dest++ = (code >> 12) | 0xE0;
	      *dest++ = ((code >> 6) & 0x3F) | 0x80;
	      *dest++ = (code & 0x3F) | 0x80;
	    }
	}
    }

  return dest;
}


//---

  int samdump2(unsigned char *sam, list_t *list, unsigned char *bootkey, char *error, int debug, int live, unsigned int size) {

  /* const */
  char *regaccountkey, *reguserskey;
  unsigned char aqwerty[] = "!@#$%^&*()qwertyUIOPAzxcvbnmQQQQQQQQQQQQ)(*@&%";
  unsigned char anum[] = "0123456789012345678901234567890123456789";
  unsigned char antpassword[] = "NTPASSWORD";
  unsigned char almpassword[] = "LMPASSWORD";

  char *buff; int buff_len;

  char *root_key;

  /* hive */
  struct hive h;
  nk_hdr *n = NULL;
  
  /* hive buffer */
  unsigned char *b = NULL;
  int blen;   
  unsigned char regkeyname[50];
  int regkeynamelen;
  char  *keyname;
  
  /* md5 contex, hash, rc4 key, hashed bootkey */
  MD5_CTX md5c;
  unsigned char md5hash[0x10];
  RC4_KEY rc4k;
  unsigned char hbootkey[0x20];
  
  /* Des */
  DES_key_schedule ks1, ks2;
  DES_cblock deskey1, deskey2;
  
  int i, j;
  
  char *username_utf8;
  int rid;
  unsigned short int disabled = 0;
  int usernameoffset, usernamelen;
  int lm_hashesoffset, nt_hashesoffset;
  int lm_size, nt_size;
  
  unsigned char obfkey[0x10];
  unsigned char fb[0x10];
    
  /* Initialize registry access function */
  _InitHive( &h );
  
  if (!live) {
    /* Open sam hive */
    if( _RegOpenHive(sam, &h ) ) {
      sprintf( error, "Error opening sam hive or not valid file(\"%s\")\n", sam );
      return -1;
    }
  } else {
    if( _RegOpenHiveBuffer(sam, (unsigned long) size, &h ) ) {
      sprintf( error, "Error opening sam hive, hive not valid\n");
      return -1;
    }
  }    

  /* Get Root key name 
     SAM for 2k/XP,
     CMI-CreateHive{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxx} for Vista */
  if( _RegGetRootKey( &h, &root_key)) {
    sprintf( error, "Error reading hive root key\n");
    return -1;
  }
  if (debug)
    printf("Root Key : %s\n", root_key);

  regaccountkey = (char *) malloc(strlen(root_key)+30);
  reguserskey = (char *) malloc(strlen(root_key)+30);

  sprintf(regaccountkey, "%s\\SAM\\Domains\\Account", root_key);
  sprintf(reguserskey, "%s\\SAM\\Domains\\Account\\Users", root_key);

  n = (nk_hdr*) malloc(sizeof(nk_hdr));

  /* Open SAM\\SAM\\Domains\\Account key*/
  if( _RegOpenKey( &h, regaccountkey, &n ) ) {
    _RegCloseHive( &h );
    sprintf( error, "%s key!\n", regaccountkey );
    return -1;
  }
  
  if( _RegQueryValue( &h, "F", n, &b, &blen ) ) {       
    _RegCloseHive( &h );
    sprintf( error, "No F!\n" );
    return -1;
  }
  
  /* hash the bootkey */
  MD5_Init( &md5c );
  MD5_Update( &md5c, &b[0x70], 0x10 );
  MD5_Update( &md5c, aqwerty, 0x2f );
  MD5_Update( &md5c, bootkey, 0x10 );
  MD5_Update( &md5c, anum, 0x29 );
  MD5_Final( md5hash, &md5c );
  RC4_set_key( &rc4k, 0x10, md5hash );
  RC4( &rc4k, 0x20, &b[0x80], hbootkey );
  
  j = 0;
  
  /* Enumerate user */
  while( j != -1 ) {
    /* Open  SAM\\SAM\\Domains\\Account\\Users */
    if( _RegOpenKey( &h, reguserskey, &n ) ) {
      _RegCloseHive( &h );
      sprintf( error, "No Users key!\n" );
      return -1;
    }
    
    regkeynamelen = sizeof( regkeyname );
    
    j = _RegEnumKey( &h, n, j, (char*)regkeyname, &regkeynamelen );

    if (debug)
    printf("******************** %d ********************\n", j);
    
    /* Skip Names key */
    if( !memcmp( regkeyname, "Names", regkeynamelen ) )
      continue;
    
    keyname = (char*) malloc( strlen( reguserskey ) + regkeynamelen + 2 );
    
    /* Open SAM\\SAM\\Domains\\Account\\Users\\userrid */
    strcpy( keyname, reguserskey );
    strcat( keyname, "\\" ) ;
    strcat( keyname, (char*)regkeyname ) ;

    if (debug)
      printf("keyname = %s\n", keyname);

    if( _RegOpenKey( &h, keyname, &n ) ) {
      _RegCloseHive( &h );
      
      sprintf( error, "Asd -_- _RegEnumKey fail!\n" );
      return -1;
    }
    
    if( _RegQueryValue( &h, "F", n, &b, &blen ) ) {
      _RegCloseHive( &h );
      
      sprintf( error, "No F value!\n" );
      return -1;
    }

    disabled = (unsigned short) *(b+56) & 0x0001;
    if (debug)
      printf("disabled = %d\n", disabled);


    if( _RegQueryValue( &h, "V", n, &b, &blen ) ) {
      _RegCloseHive( &h );
      
      sprintf( error, "No V value!\n" );
      return -1;
    }
    
    /* rid */
    rid = strtoul( (char*)regkeyname, NULL, 16 );
    
    /* get the username */
    /* 0x10 username size 0xc username offset */
#if BYTE_ORDER == LITTLE_ENDIAN
    usernamelen = *(int*)(b + 0x10) >> 1;
#elif BYTE_ORDER == BIG_ENDIAN
    usernamelen = __bswap_32(*(int*)(b + 0x10) >> 1);
#endif 
    usernameoffset = b[0xc] + 0xcc;

    if (debug)
      printf("\nusername len=%d, off=%x\n", usernamelen, usernameoffset);
    
    username_utf8 = (char *) malloc (usernamelen*2 + 1);
    memset(username_utf8, 0, usernamelen*2 + 1);
    utf16_to_utf8 ((unsigned char *)username_utf8, (unsigned short int*) &b[usernameoffset], usernamelen);

#if BYTE_ORDER == LITTLE_ENDIAN
    lm_hashesoffset = *(int *)(b + 0x9c ) + 0xcc;
    lm_size = *(int *)(b + 0xa0 );
    nt_hashesoffset = *(int *)(b + 0xa8 ) + 0xcc;
    nt_size = *(int *)(b + 0xac );
#elif BYTE_ORDER == BIG_ENDIAN
    lm_hashesoffset = __bswap_32(*(int *)(b + 0x9c )) + 0xcc;
    lm_size = __bswap_32(*(int *)(b + 0xa0 ));
    nt_hashesoffset = __bswap_32(*(int *)(b + 0xa8 )) + 0xcc;
    nt_size = __bswap_32(*(int *)(b + 0xac ));
#endif

    if (debug) {
      printf("lm_hashoffset = %x, lm_size = %x\n", lm_hashesoffset, lm_size);
      printf("nt_hashoffset = %x, nt_size = %x\n", nt_hashesoffset, nt_size);
    }

    buff = (char*)malloc(512);
    /* Print the user hash */
    if (disabled)
      buff_len = sprintf(buff, "*disabled* %s:%d:", username_utf8, rid );
    else
      buff_len = sprintf(buff, "%s:%d:", username_utf8, rid );


    if( lm_size == 0x14 ) {
      if (debug) {
	printf("\n");
	for( i = 0; i < 0x10; i++ )
	  printf( "%.2x", b[lm_hashesoffset+4+i] );
      }

      /* LANMAN */
      /* hash the hbootkey and decode lanman password hashes */
      MD5_Init( &md5c );
      MD5_Update( &md5c, hbootkey, 0x10 );
#if BYTE_ORDER == LITTLE_ENDIAN
      MD5_Update( &md5c, &rid, 0x4 );
#elif BYTE_ORDER == BIG_ENDIAN
      rid = __bswap_32(rid);
      MD5_Update( &md5c, &rid, 0x4 );
      rid = __bswap_32(rid);
#endif
      MD5_Update( &md5c, almpassword, 0xb );
      MD5_Final( md5hash, &md5c );        
      
      RC4_set_key( &rc4k, 0x10, md5hash );
      RC4( &rc4k, 0x10, &b[ lm_hashesoffset + 4 ], obfkey );
      if (debug) {
	printf("\nobfkey: ");
	for( i = 0; i < 0x10; i++ )
	  printf( "%.2x", (unsigned char)obfkey[i] );
	printf("\n");
      }

      
      /* From Pwdump */
      
      /* Get the two decrpt keys. */
      sid_to_key1(rid,(unsigned char *)deskey1);
      DES_set_key_checked((DES_cblock *)deskey1, &ks1);
      sid_to_key2(rid,(unsigned char *)deskey2);
      DES_set_key_unchecked((DES_cblock *)deskey2, &ks2);
      
      /* Decrypt the lanman password hash as two 8 byte blocks. */
      DES_ecb_encrypt((DES_cblock *)obfkey,
		      (DES_cblock *)fb, &ks1, DES_DECRYPT);
      DES_ecb_encrypt((DES_cblock *)(obfkey + 8),
		      (DES_cblock *)&fb[8], &ks2, DES_DECRYPT);
      
      
      
      // sf25( obfkey, (int*)&rid, fb );
      
      for( i = 0; i < 0x10; i++ ) {
	sprintf(buff+buff_len, "%.2x", fb[i] );
	buff_len +=2;
      }
    } else {
      sprintf(buff+buff_len, "aad3b435b51404eeaad3b435b51404ee" );
      buff_len+=32;
    }
    sprintf(buff+buff_len, ":" );
    buff_len++;
    
    if( nt_size == 0x14 ) {
      if (debug) {
	printf("\n");
	for( i = 0; i < 0x10; i++ )
	  printf( "%.2x", b[nt_hashesoffset+4+i] );
	printf("\n");
      }
      
      /* NT */
      /* hash the hbootkey and decode the nt password hashes */
      MD5_Init( &md5c );
      MD5_Update( &md5c, hbootkey, 0x10 );
#if BYTE_ORDER == LITTLE_ENDIAN
      MD5_Update( &md5c, &rid, 0x4 );
#elif BYTE_ORDER == BIG_ENDIAN
      rid = __bswap_32(rid);
      MD5_Update( &md5c, &rid, 0x4 );
      rid = __bswap_32(rid);
#endif
      MD5_Update( &md5c, antpassword, 0xb );
      MD5_Final( md5hash, &md5c );        
      
      RC4_set_key( &rc4k, 0x10, md5hash );
      RC4( &rc4k, 0x10, &b[ nt_hashesoffset + 4], obfkey );

      if (lm_size != 0x14) {
	/* Get the two decrpt keys. */
	sid_to_key1(rid,(unsigned char *)deskey1);
	DES_set_key((DES_cblock *)deskey1, &ks1);
	sid_to_key2(rid,(unsigned char *)deskey2);
	DES_set_key((DES_cblock *)deskey2, &ks2);
      }

      /* Decrypt the NT md4 password hash as two 8 byte blocks. */
      DES_ecb_encrypt((DES_cblock *)obfkey,
		      (DES_cblock *)fb, &ks1, DES_DECRYPT);
      DES_ecb_encrypt((DES_cblock *)(obfkey + 8),
		      (DES_cblock *)&fb[8], &ks2, DES_DECRYPT);
      
      /* sf27 wrap to sf25 */
      //sf27( obfkey, (int*)&rid, fb );
      
      for( i = 0; i < 0x10; i++ ) {
	sprintf(buff+buff_len, "%.2x", fb[i] );
	buff_len +=2;
      }
    } else {
      sprintf(buff+buff_len, "31d6cfe0d16ae931b73c59d7e0c089c0");
      buff_len+=32;
    }
    sprintf(buff+buff_len, ":::" );
    buff_len+=3;

    /* add the hash to the list */
    list_add_tail(list, buff);
    
    free( username_utf8 );
    free( keyname );
  }
  
  _RegCloseHive( &h );
  free(n);
  free(b);
  
  return 0;
}
