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
#include <openssl/md4.h>

#include "hashToPwd.h"
#include "aliasPattern.h"
#include "aliasMarkov.h"
#include "patternDescriptor.h"

uint64_t getPatternFirstRound(patternArray* array, uint64_t index, unsigned int* patIndex){
  unsigned int i = 0;
  uint64_t cumul = 0;
  unsigned char found = 0; 

  while(found == 0 && i < array->size){
    if (cumul + array->sizeArray[i] > index){
      found = 1;
      *patIndex = i;
    }
    else{
      cumul += array->sizeArray[i];
      i ++;
    }
  }
  return cumul;
}

unsigned int selectCharacterUTFFirstRound(markovDynProgWrapper* mdpw, markovImprobaStruct* mis, uint64_t patternDescriptor, uint64_t patternLength, uint64_t subIndex, char* result){
  unsigned int i = 0;
  unsigned int j = 0;
  unsigned int k = 0;
  char spe[] = " !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
  unsigned int prevChar1 = 0;
  unsigned int prevChar2 = 0;
  unsigned int markov_length = 0;
  unsigned int markov_level = 0;
  unsigned int* pointerImproba = NULL;
  uint64_t markovIndex = 0;
  uint64_t sum = 0;
  uint64_t size = 0;
	
	
  for (i = 0; i < PATTERN_ARRAY_MAX_LENGTH; i++){
    if ((unsigned int)(patternDescriptor >> (2+4*i))&0x00000001){
      switch((unsigned int)(patternDescriptor >> (4*i))&0x00000003){
      case MARKOV_FIRST 	: {
	markov_length = ((unsigned int)(patternLength >> (4*i))&0x0000000f)+1;
	markov_level = (MAX_MARKOV_SIZE-markov_length)*mis->meanImprobaMark0;
	switch (mdpw->type){
	case MARK0 		: {
	  markovIndex = subIndex%mdpw->m0dp->rootValue[markov_length-1];
	  subIndex = subIndex/mdpw->m0dp->rootValue[markov_length-1];
	  break;
	}
	case MARK1		: {
	  markovIndex = subIndex%mdpw->m1dp->rootValue[markov_length-1];
	  subIndex = subIndex/mdpw->m1dp->rootValue[markov_length-1];
	  break;
	}
	case MARK2 		: {
	  markovIndex = subIndex%mdpw->m2dp->rootValue[markov_length-1];
	  subIndex = subIndex/mdpw->m2dp->rootValue[markov_length-1];
	  break;
	}
	case NO_MARK 	: {break;}
	}
	pointerImproba = mis->improbaFirst;
	break;
      }
      case MARKOV_SECOND 	: {
	switch (mdpw->type){
	case MARK0 		: {pointerImproba = mis->improbaMark0; break;}
	case MARK1		: {pointerImproba = mis->improbaMark1[prevChar1]; break;}
	case MARK2 		: {pointerImproba = mis->improbaMark1[prevChar1]; break;}
	case NO_MARK 	: {break;}
	}
	markov_length = MAX_MARKOV_SIZE-markov_length+2;
	prevChar2 = prevChar1;
	break;
      }
      case MARKOV_MAIN 	: {
	switch(mdpw->type){
	case MARK0 : {pointerImproba = mis->improbaMark0; break;}
	case MARK1 : {pointerImproba = mis->improbaMark1[prevChar1]; break;}
	case MARK2 : {pointerImproba = mis->improbaMark2[prevChar2][prevChar1]; break;}
	case NO_MARK : {break;}
	}
	markov_length++;
	prevChar2 = prevChar1;
	break;
      }
      }
      sum = 0;
      j = 0;
      while (sum <= markovIndex){
	switch((unsigned int)(patternDescriptor >> (4*i))&0x00000003){
	case MARKOV_FIRST : {
	  switch(mdpw->type){
	  case MARK0 		: {size = getMarkov0DynProgFirstLine(mdpw->m0dp, mis, j, markov_length); break;}
	  case MARK1 		: {size = getMarkov1DynProgFirstLine(mdpw->m1dp, mis, j, markov_length); break;}
	  case MARK2 		: {size = getMarkov2DynProgFirstLine(mdpw->m2dp, mis, j, markov_length); break;}
	  case NO_MARK 	: {break;}
	  }
	  break;
	}
	case MARKOV_SECOND : {
	  switch(mdpw->type){
	  case MARK0 		: {size = getMarkov0DynProgSpace(mdpw->m0dp, mis, markov_length, markov_level+pointerImproba[j]); break;}
	  case MARK1 		: {size = getMarkov1DynProgSpace(mdpw->m1dp, mis, markov_length, markov_level+pointerImproba[j], j); break;}
	  case MARK2 		: {size = getMarkov2DynProgSpace(mdpw->m2dp, mis, markov_length, markov_level+pointerImproba[j], j, prevChar1); break;}
	  case NO_MARK 	: {break;}
	  }
	  break;
	}
	case MARKOV_MAIN : {
	  switch(mdpw->type){
	  case MARK0 		: {size = getMarkov0DynProgSpace(mdpw->m0dp, mis, markov_length, markov_level+pointerImproba[j]); break;}
	  case MARK1 		: {size = getMarkov1DynProgSpace(mdpw->m1dp, mis, markov_length, markov_level+pointerImproba[j], j); break;}
	  case MARK2 		: {size = getMarkov2DynProgSpace(mdpw->m2dp, mis, markov_length, markov_level+pointerImproba[j], j, prevChar1); break;}
	  case NO_MARK 	: {break;}
	  }
	  break;
	}
	}
	if (sum + size > markovIndex){
	  markovIndex -= sum;
	  sum += size;
	  prevChar1 = j;
	}
	else{
	  sum += size;
	  j++;
	}
      }
      result[k] = 97 - 32*((unsigned int)(patternDescriptor >> (3+4*i)) &0x00000001) +  prevChar1;
      markov_level += pointerImproba[prevChar1];
      k += 2;
    }
    else{
      switch((unsigned int)(patternDescriptor >> (4*i))&0x00000003){
      case NO_MARK_IGNORED		: {
	result[k] = 0; 
	break;
      }
      case NO_MARK_SPECIAL		: {
	result[k] =  spe[subIndex%33];
	subIndex = subIndex / 33;
	k += 2;
	break;
      }
      case NO_MARK_NUMERAL		: {
	result[k] = 48 + subIndex%10;
	subIndex = subIndex / 10;
	k += 2;
	break;
      }
      case NO_MARK_LETTER			: {
	result[k] = 97 - 32*((unsigned int)(patternDescriptor >> (3+4*i)) &0x00000001) + subIndex%26;
	subIndex = subIndex / 26;
	k += 2;
	break;
      }
      }
    }
  }
  return k;
}

unsigned int selectCharacterUTF(markovDynProgWrapper* mdpw, markovImprobaStruct* mis, uint64_t patternDescriptor, uint64_t patternLength, uint64_t biasedCoin, uint64_t fairDice, char* result){
  unsigned int i = 0;
  unsigned int k = 0;
  char spe[] = " !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
	
  unsigned int markov_length = 0;
  unsigned int markov_level = 0;
  char* pointerAlias = NULL;
  unsigned int* pointerImproba = NULL;
  uint64_t space = 0;
  unsigned int prevChar1 = 0;
  unsigned int prevChar2 = 0;
  unsigned int predicate = 0;
	
  for (i = 0; i < PATTERN_ARRAY_MAX_LENGTH; i++){
    if ((unsigned int)(patternDescriptor >> (2+4*i))&0x00000001){
      switch((unsigned int)(patternDescriptor >> (4*i))&0x00000003){
      case MARKOV_FIRST 	: {
	markov_length = (unsigned int)(patternLength >> (4*i))&0x0000000f;
	markov_level = (MAX_MARKOV_SIZE-markov_length-1)*mis->meanImprobaMark0;
	pointerAlias = mdpw->amdp->amb->bufferRoot + mdpw->amdp->rootValue[markov_length];
	pointerImproba = mis->improbaFirst;
	break;
      }
      case MARKOV_SECOND 	: {
	markov_length --;
	pointerAlias = mdpw->amdp->amb->bufferFLine + mdpw->amdp->firstLine[prevChar1 + markov_length*ALPHABET_SIZE];
	switch (mdpw->type){
	case MARK0: {pointerImproba = mis->improbaMark0; break;}
	case MARK1: {pointerImproba = mis->improbaMark1[prevChar1]; break;}
	case MARK2: {pointerImproba = mis->improbaMark1[prevChar1]; break;}
	case NO_MARK: {break;}
	}
	markov_length = MAX_MARKOV_SIZE-markov_length;
	prevChar2 = prevChar1;
	break;
      }
      case MARKOV_MAIN 	: {
	switch(mdpw->type){
	case MARK0 : {
	  pointerAlias = mdpw->amdp->amb->bufferMain + mdpw->amdp->buffer[(markov_length-2)*mdpw->amdp->bufferWidth+markov_level-markov_length*mdpw->amdp->minImproba];
	  pointerImproba = mis->improbaMark0;
	  break;
	}
	case MARK1 : {
	  pointerAlias = mdpw->amdp->amb->bufferMain + mdpw->amdp->buffer[((markov_length-2)*mdpw->amdp->bufferWidth+markov_level-markov_length*mdpw->amdp->minImproba)*ALPHABET_SIZE+prevChar1];
	  pointerImproba = mis->improbaMark1[prevChar1];
	  break;
	}
	case MARK2 : {
	  pointerAlias = mdpw->amdp->amb->bufferMain + mdpw->amdp->buffer[(((markov_length-2)*mdpw->amdp->bufferWidth+markov_level-markov_length*mdpw->amdp->minImproba)*ALPHABET_SIZE+prevChar1)*ALPHABET_SIZE+prevChar2];
	  pointerImproba = mis->improbaMark2[prevChar2][prevChar1]; 
	  break;
	}
	case NO_MARK : {break;}
	}
	markov_length++;
	prevChar2 = prevChar1;
	break;
      }
      }
      space = *(uint64_t*)pointerAlias;
      predicate = ((space & (uint64_t)0xffffffff00000000) != 0);
      prevChar1 = fairDice%26;
      pointerAlias += 8;
      if (biasedCoin%space >= *(unsigned int*)(pointerAlias + (prevChar1 << (2+predicate)))){
	pointerAlias += (26<<(2+predicate));
	prevChar1 = *(unsigned char*)(pointerAlias + prevChar1);
      }
      result[k] = 97 - 32*((unsigned int)(patternDescriptor >> (3+4*i)) &0x00000001) +  prevChar1;
      markov_level += pointerImproba[prevChar1];
      fairDice = fairDice/26;
      k += 2;
    }
    else{
      switch((unsigned int)(patternDescriptor >> (4*i))&0x00000003){
      case NO_MARK_IGNORED		: {
	result[k] = 0; 
	break;
      }
      case NO_MARK_SPECIAL		: {
	result[k] =  spe[biasedCoin%33];
	biasedCoin = biasedCoin / 33;
	k += 2;
	break;
      }
      case NO_MARK_NUMERAL		: {
	result[k] = 48 + biasedCoin%10;
	biasedCoin = biasedCoin / 10;
	k += 2;
	break;
      }
      case NO_MARK_LETTER			: {
	result[k] = 97 - 32*((unsigned int)(patternDescriptor >> (3+4*i)) &0x00000001) + biasedCoin%26;
	biasedCoin = biasedCoin / 26;
	k += 2;
	break;
      }
      }
    }
  }
  return k;
}

char checkFoundPwd(char* pwdUTF, unsigned int taille, unsigned int* hash, char* pwd){
  unsigned int i = 0;
  unsigned int hashTest[4];
  char result = 1;
	
  MD4((unsigned char*)pwdUTF, (size_t)(taille), (unsigned char*)hashTest);
	
  for (i = 0; i < 4; i++){
    if (hash[i] != hashTest[i]){
      result = 0;
    }
  }
	
  if (result){
    for (i = 0; i < (taille >> 1); i++){
      pwd[i] = pwdUTF[i << 1];
    }
    pwd[taille >> 1] = '\0';
  }

  return result;
}

uint64_t hashToIndex(patternArray* array, markovDynProgWrapper* mdpw, markovImprobaStruct* mis, unsigned int* hashsrc, const unsigned int startC, const unsigned int endC, const unsigned int threshold, const unsigned int currentTable, const unsigned int chainLength){
  uint64_t indexLetterBiased;
  uint64_t indexLetterFair;
  uint64_t indexPatternBiased;
  uint16_t indexPatternFair;
  unsigned int taille = 0;
  unsigned int patIndex = 0;
  unsigned int i = startC;
  char pwdUTF[PWD_UTF_SIZE] = {0};
  uint32_t hash[4];

  indexLetterBiased = *(uint64_t*)((char*)hashsrc + 5) ^ (uint64_t)(i + currentTable*chainLength);
  indexLetterFair = *(uint64_t*)hashsrc ^ (uint64_t)(i + currentTable*chainLength);
  indexPatternBiased = *(uint64_t*)(hashsrc + 2) >> (64 - threshold);
  indexPatternFair = *(uint16_t*)hashsrc ;

  for (i = startC + 1; i < endC; i++){
    patIndex = selectPatternAlias(array->lut, indexPatternBiased, indexPatternFair);
    taille = selectCharacterUTF(mdpw, mis, array->patternDescriptor[patIndex], array->patternLength[patIndex], indexLetterBiased, indexLetterFair, pwdUTF);

    MD4((unsigned char *)pwdUTF, (size_t)(taille), (unsigned char*)hash);

    memcpy(&indexLetterBiased, (char*)hash + 5, sizeof(uint64_t));
    indexLetterBiased ^= (uint64_t)(i + currentTable*chainLength);

    memcpy(&indexLetterFair, hash, sizeof(uint64_t));
    indexLetterFair ^= (uint64_t)(i + currentTable*chainLength);

    memcpy(&indexPatternBiased, hash + 2, sizeof(uint64_t));
    indexPatternBiased >>= (64 - threshold);

    memcpy(&indexPatternFair, hash, sizeof(uint16_t));
  }

  return (indexLetterBiased & ((uint64_t)0xffffffffffffffff >> (64 - threshold)));
}

unsigned int indexToPwdUTF(patternArray* array, markovDynProgWrapper* mdpw, markovImprobaStruct* mis, const uint64_t start, const unsigned int threshold, const unsigned int endC, const unsigned int currentTable, const unsigned int chainLength, char* pwdUTF){
  uint64_t indexLetterBiased;
  uint64_t indexLetterFair;
  uint64_t indexPatternBiased;
  uint16_t indexPatternFair;
  unsigned int taille = 0;
  unsigned int patIndex = 0;
  unsigned int i = 0;
  uint32_t hash[4];
  uint64_t tailleCumul;

  tailleCumul = getPatternFirstRound(array, start, &patIndex);
  taille = selectCharacterUTFFirstRound(mdpw, mis, array->patternDescriptor[patIndex], array->patternLength[patIndex], start - tailleCumul, pwdUTF);

  for (i = 0; i < endC; i++){

    MD4((unsigned char *)pwdUTF, (size_t)(taille), (unsigned char*)hash);

    memcpy(&indexLetterBiased, (char*)hash + 5, sizeof(uint64_t));
    indexLetterBiased ^= (uint64_t)(i + currentTable*chainLength);

    memcpy(&indexLetterFair, hash, sizeof(uint64_t));
    indexLetterFair ^= (uint64_t)(i + currentTable*chainLength);

    memcpy(&indexPatternBiased, hash + 2, sizeof(uint64_t));
    indexPatternBiased >>= (64 - threshold);

    memcpy(&indexPatternFair, hash, sizeof(uint16_t));

    patIndex = selectPatternAlias(array->lut, indexPatternBiased, indexPatternFair);
    taille = selectCharacterUTF(mdpw, mis, array->patternDescriptor[patIndex], array->patternLength[patIndex], indexLetterBiased, indexLetterFair, pwdUTF);
  }

  return taille;
}


void hashNTH(char* plaintxt, unsigned char* hash){
  char pwdUTF[PWD_UTF_SIZE] = {0};
  unsigned int j;
	
  for (j = 0; j < strlen(plaintxt) && 2*j < PWD_UTF_SIZE; j++){
    pwdUTF[2*j] = plaintxt[j];
  }
  MD4((unsigned char*)pwdUTF, (size_t)(2*strlen(plaintxt)), hash);

}
