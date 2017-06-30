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
/* This file is a copy from implCPU/v6 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "aliasPattern.h"

/*attention on ne peut pas avoir plus de 2^16 patterns car la selection du pattern se fait sur uint16_t
  d'autre part l'espace d'adressage ne doit pas exéder 2^56 sinon certaine valeures ne seront plus indéxées*/

uint64_t getThresholdFromPatternArray(patternArray* array);

void initLookUpTable(lookUpTable* lut, patternArray* array){
  lut->proba = (uint64_t*)malloc(sizeof(uint64_t)*array->size);
  lut->alias = (unsigned int*)malloc(sizeof(unsigned int)*array->size);
  lut->length = (unsigned int)array->size;
	
  uint64_t* 	large = (uint64_t*)malloc(sizeof(uint64_t)*array->size);
  uint64_t* 	small = (uint64_t*)malloc(sizeof(uint64_t)*array->size);
  unsigned int*	patternNumberLarge = (unsigned int*)malloc(sizeof(unsigned int)*array->size);
  unsigned int*	patternNumberSmall = (unsigned int*)malloc(sizeof(unsigned int)*array->size);
	
  unsigned int	i;
  unsigned int 	pointerLarge = 0;
  unsigned int	pointerSmall = 0;
  uint64_t	 	temp;
  uint64_t		threshold	 = getThresholdFromPatternArray(array);

  for (i = 0; i < array->size; i++){
    temp = (uint64_t)array->size*array->sizeArray[i];
    if (temp < threshold){
      small[pointerSmall] = temp;
      patternNumberSmall[pointerSmall] = i;
      pointerSmall ++;
    }
    else{
      large[pointerLarge] = temp;
      patternNumberLarge[pointerLarge] = i;
      pointerLarge ++;
    }
  }
	
  while (pointerLarge >  0 && pointerSmall > 0){
    lut->proba[patternNumberSmall[pointerSmall-1]] = small[pointerSmall-1];
    lut->alias[patternNumberSmall[pointerSmall-1]] = patternNumberLarge[pointerLarge-1];
    temp = large[pointerLarge-1]+small[pointerSmall-1] - threshold;
    if (temp < threshold){
      small[pointerSmall-1] = temp;
      patternNumberSmall[pointerSmall-1] = patternNumberLarge[pointerLarge-1];
      pointerLarge --;
    }
    else{
      large[pointerLarge-1] = temp;
      pointerSmall --;
    }
  }
  while (pointerLarge > 0){
    lut->proba[patternNumberLarge[pointerLarge-1]] = threshold;
    lut->alias[patternNumberLarge[pointerLarge-1]] = patternNumberLarge[pointerLarge-1];
    pointerLarge --;
  }
	
  free(large);
  free(small);
  free(patternNumberLarge);
  free(patternNumberSmall);
}


void deleteLookUpTable(lookUpTable* lut){
  if (lut != NULL){
    if (lut->proba != NULL){
      free(lut->proba);
    }
    if (lut->alias != NULL){
      free (lut->alias);
    }
  }
}

unsigned int selectPatternAlias(lookUpTable* lut, uint64_t biasedCoin, uint16_t fairDice_){
  unsigned int fairDice = (unsigned int)(fairDice_%(lut->length));
	
  if(biasedCoin >= lut->proba[fairDice]){
    return lut->alias[fairDice];
  }
  else{
    return fairDice;
  }
}

uint64_t getThresholdFromPatternArray(patternArray* array){
  unsigned int 	i		= 0;
  uint64_t 		result 	= 0;
	
  for (i = 0; i < array->size; i++){
    result += array->sizeArray[i];
  }
	
  return result;
}
