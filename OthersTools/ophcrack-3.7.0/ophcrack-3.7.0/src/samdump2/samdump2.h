/** samdump2.h
    Declaration file for samdump2 3.x
    
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
    
    Copyright (C) 2008 Cedric Tissieres <http://www.objectif-securite.ch>
*/

#ifndef SAMDUMP2_H
#define SAMDUMP2_H

#ifdef  __cplusplus
extern "C" {
#endif

int bkhive(unsigned char *system, unsigned char *pkey, char *error, int debug);
  int samdump2(unsigned char *sam, list_t *list, unsigned char *bootkey, char *error, int debug, int live, unsigned int size );
#ifdef WIN32
int get_live_syskey(unsigned char *pkey, char *error, int debug);
int get_sam(unsigned char **buff_sam, int *size, char *error, int debug);
#endif
  
#ifdef __cplusplus
}
#endif
#endif
