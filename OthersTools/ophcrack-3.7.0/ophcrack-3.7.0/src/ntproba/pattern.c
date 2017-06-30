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
#include <string.h>
#include <math.h>
#include <time.h>

#include "pattern.h"
#include "patternDescriptor.h"


uint64_t power(unsigned int a, unsigned int b){
  uint64_t result = 1;
  unsigned int i;
	
  for (i = 0; i < b; i++){
    result *= a;
  }
	
  return result;
}


uint64_t getPatternStructSpace(markovDynProgWrapper* mdpw, markovImprobaStruct* mis, uint64_t descriptor, uint64_t length){
  uint64_t result = 1;
  unsigned int i = 0;
  unsigned int l;
	
  for (i = 0; i < PATTERN_ARRAY_MAX_LENGTH; i++){
    switch ((char)(descriptor >> (i*4)) & 0x0f){
    case DESC_NUMERAL : {
      result = result*10;
      break;
    }
    case DESC_SPECIAL : {
      result = result*33;
      break;
    }
    case DESC_UPPER_NOMARK : {
      result = result*26;
      break;
    }
    case DESC_UPPER_MARKFIRST : {
      l = (unsigned int)((char)(length >> (i*4)) & 0x0f) + 1;
      result = result*getMarkovDynProgRootValue(mdpw, mis, l);
      break;
    }
    case DESC_LOWER_NOMARK : {
      result = result*26;
      break;
    }
    case DESC_LOWER_MARKFIRST : {
      l = (unsigned int)((char)(length >> (i*4)) & 0x0f) + 1;
      result = result*getMarkovDynProgRootValue(mdpw, mis, l);
      break;
    }
    default : {break;}
    }
  }
  return result;
}


char checkPatternArrayCrackMarkov(patternArray* array, markovDynProgWrapper* mdpw, markovImprobaStruct* mis){
  unsigned int i;
  char result = 1;
	
  for (i = 0; i < array->size-1; i++){
    if (array->sizeArray[i] != getPatternStructSpace(mdpw, mis, array->patternDescriptor[i], array->patternLength[i])){
      printf("Pattern %d: size error %llu vs %llu\n", i, 
	     (long long unsigned int)array->sizeArray[i], 
	     (long long unsigned int)getPatternStructSpace(mdpw, mis, array->patternDescriptor[i], array->patternLength[i]));
      result = 0;
    }
  }
  if (array->sizeArray[array->size-1] > getPatternStructSpace(mdpw, mis, array->patternDescriptor[array->size-1], array->patternLength[array->size-1])){
    printf("Pattern %d: size error %llu vs %llu\n", i, 
	   (long long unsigned int)array->sizeArray[array->size-1], 
	   (long long unsigned int)getPatternStructSpace(mdpw, mis, array->patternDescriptor[array->size-1], array->patternLength[array->size-1]));
    result = 0;
  }
  return result;
}

void deletePatternArray(patternArray* array){
  if (array->patternDescriptor != NULL){
    free(array->patternDescriptor);
  }
  if (array->patternLength != NULL){
    free(array->patternLength);
  }
  if (array->lut != NULL){
    deleteLookUpTable(array->lut);
    free(array->lut);
  }
  if (array->sizeArray != NULL){
    free(array->sizeArray);
  }
}



unsigned int getMaxPatternLength(patternArray* array){
  unsigned int i;
  unsigned int maxLength = 0;
  unsigned int currentLength;
  unsigned int c;
	
  for (i = 0; i < array->size; i++){
    currentLength = 0;
    for (c = 0; c < PATTERN_ARRAY_MAX_LENGTH; c++){
      if (((array->patternDescriptor[i] >> (4*c))&0x000000000000000f) != DESC_IGNORED){
	currentLength++;
      }
    }
    if (currentLength > maxLength){
      maxLength = currentLength;
    }
  }
	
  return maxLength;
}
