/*
 *   
 *   Ophcrack is a Lanmanager/NTLM hash cracker based on the faster time-memory
 *   trade-off using rainbow tables. 
 *   
 *   Created with the help of: Maxime Mueller, Luca Wullschleger, Claude
 *   Hochreutiner, Andreas Huber and Etienne Dysli.
 *   
 *   Copyright (c) 2013 Philippe Oechslin, Cedric Tissieres, 
 *                      Bertrand Mesot, Pierre Lestringant
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
/* WARNING: this file is a NOT copy from implCPU/v6 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "ioMarkov.h"


char readImprobaStructFromBinFile(markovImprobaStruct** mis, char* path, char* fileName){
  struct stat sb;
  int binFile;
  char buffer[256];
	
  if (fileName != NULL){
    sprintf(buffer, "%s/%s", path, fileName);
    binFile = open(buffer, O_RDONLY);
  }
  else{
    sprintf(buffer, "%s/%s", path, DEFAULT_MARKOV_FILE_NAME);
    binFile = open(buffer, O_RDONLY);
  }
	
  if (binFile == -1){
    return 0;
  }
	
  if (fstat(binFile, &sb) == -1){
    close(binFile);
    return 0;
  }
	
	
  if (sb.st_size != sizeof(markovImprobaStruct)){
    close(binFile);
    return 0;
  }
	
  *mis = (markovImprobaStruct*)mmap(NULL, sizeof(markovImprobaStruct),  PROT_READ, MAP_PRIVATE, binFile, 0);
  if (*mis == MAP_FAILED){
    close(binFile);
    return 0;
  }
  close(binFile);
  return 1;
}

markovType fromStringMarkovType(const char* word, const int length){
  if (!memcmp(word, "NO_MARK", length)){
    return NO_MARK;
  }
  if (!memcmp(word, "MARK0", length)){
    return MARK0;
  }
  if (!memcmp(word, "MARK1", length)){
    return MARK1;
  }
  if (!memcmp(word, "MARK2", length)){
    return MARK2;
  }
  return NO_MARK;
}
