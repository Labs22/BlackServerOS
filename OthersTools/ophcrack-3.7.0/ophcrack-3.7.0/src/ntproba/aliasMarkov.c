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

#include <stdio.h>

#include "aliasMarkov.h"

void initAliasMarkov32Element(aliasMarkov32Element* am32e, uint64_t space, uint64_t* subSpace);
void initAliasMarkov64Element(aliasMarkov64Element* am64e, uint64_t space, uint64_t* subSpace);

void initAliasMarkov32Element(aliasMarkov32Element* am32e, uint64_t space, uint64_t* subSpace){
  unsigned int i;
  uint64_t temp;
  am32e->space = space;
	
  uint64_t large[ALPHABET_SIZE];
  uint64_t small[ALPHABET_SIZE];
  unsigned char numberLarge[ALPHABET_SIZE];
  unsigned char numberSmall[ALPHABET_SIZE];
  unsigned int pointerLarge = 0;
  unsigned int pointerSmall = 0;
	
  for (i = 0; i < ALPHABET_SIZE; i++){
    temp = subSpace[i]*ALPHABET_SIZE;
    if (temp < space){
      small[pointerSmall] = temp;
      numberSmall[pointerSmall] = i;
      pointerSmall++;
    }
    else{
      large[pointerLarge] = temp;
      numberLarge[pointerLarge] = i;
      pointerLarge++;
    }
  }
	
  while(pointerLarge > 0 && pointerSmall > 0){
    am32e->proba[numberSmall[pointerSmall-1]] = (unsigned int)small[pointerSmall -1];
    am32e->alias[numberSmall[pointerSmall-1]] = numberLarge[pointerLarge-1];
    temp = large[pointerLarge-1] + small[pointerSmall-1] - space;
    if (temp < space){
      small[pointerSmall-1] = temp;
      numberSmall[pointerSmall-1] = numberLarge[pointerLarge-1];
      pointerLarge--;
    }
    else{
      large[pointerLarge-1] = temp;
      pointerSmall--;
    }
  }
  while (pointerLarge > 0){
    am32e->proba[numberLarge[pointerLarge-1]] = (unsigned int)space;
    am32e->alias[numberLarge[pointerLarge-1]] = numberLarge[pointerLarge-1];
    pointerLarge --;
  }
}

void initAliasMarkov64Element(aliasMarkov64Element* am64e, uint64_t space, uint64_t* subSpace){
  unsigned int i;
  uint64_t temp;
  am64e->space = space;
	
  uint64_t large[ALPHABET_SIZE];
  uint64_t small[ALPHABET_SIZE];
  unsigned char numberLarge[ALPHABET_SIZE];
  unsigned char numberSmall[ALPHABET_SIZE];
  unsigned int pointerLarge = 0;
  unsigned int pointerSmall = 0;
	
  for (i = 0; i < ALPHABET_SIZE; i++){
    temp = subSpace[i]*ALPHABET_SIZE;
    if (temp < space){
      small[pointerSmall] = temp;
      numberSmall[pointerSmall] = i;
      pointerSmall++;
    }
    else{
      large[pointerLarge] = temp;
      numberLarge[pointerLarge] = i;
      pointerLarge++;
    }
  }
	
  while(pointerLarge > 0 && pointerSmall > 0){
    am64e->proba[numberSmall[pointerSmall-1]] = small[pointerSmall -1];
    am64e->alias[numberSmall[pointerSmall-1]] = numberLarge[pointerLarge-1];
    temp = large[pointerLarge-1] + small[pointerSmall-1] - space;
    if (temp < space){
      small[pointerSmall-1] = temp;
      numberSmall[pointerSmall-1] = numberLarge[pointerLarge-1];
      pointerLarge--;
    }
    else{
      large[pointerLarge-1] = temp;
      pointerSmall--;
    }
  }
  while (pointerLarge > 0){
    am64e->proba[numberLarge[pointerLarge-1]] = space;
    am64e->alias[numberLarge[pointerLarge-1]] = numberLarge[pointerLarge-1];
    pointerLarge --;
  }
}

uint64_t getMark0BufferSizeRoot(markov0DynProg* m0dp){
  unsigned int i;
  uint64_t result = 0;
	
  for (i = 0; i < MAX_MARKOV_SIZE; i++){
    if (m0dp->rootValue[i] != 0 && m0dp->rootValue[i] != (uint64_t)0xffffffffffffffff){
      if (m0dp->rootValue[i] & (uint64_t)0xffffffff00000000){
	result += sizeof(aliasMarkov64Element);
      }
      else{
	result += sizeof(aliasMarkov32Element);
      }
    }
  }

  return result;
}

uint64_t getMark0BufferSizeFLine(markov0DynProg* m0dp){
  unsigned int i;
  uint64_t result = 0;
	
  for (i = 0; i < (MAX_MARKOV_SIZE-1)*ALPHABET_SIZE; i++){
    if (m0dp->firstLine[i] != 0 && m0dp->firstLine[i] != (uint64_t)0xffffffffffffffff){
      if (m0dp->firstLine[i] & (uint64_t)0xffffffff00000000){
	result += sizeof(aliasMarkov64Element);
      }
      else{
	result += sizeof(aliasMarkov32Element);
      }
    }
  }
	
  return result;
}

uint64_t getMark0BufferSizeMain(markov0DynProg* m0dp){
  unsigned int i;
  uint64_t result = 0;
	
  for (i = 0; i < m0dp->bufferWidth*(MAX_MARKOV_SIZE-2); i++){
    if (m0dp->buffer[i] != 0 && m0dp->buffer[i] != (uint64_t)0xffffffffffffffff){
      if (m0dp->buffer[i] & (uint64_t)0xffffffff00000000){
	result += sizeof(aliasMarkov64Element);
      }
      else{
	result += sizeof(aliasMarkov32Element);
      }
    }
  }
	
  return result;
}

void fillAliasMarkov0BufferRoot(markov0DynProg* m0dp, AmarkovDynProg* amdp, markovImprobaStruct* mis, char* bufferRoot){
  unsigned int length;
  unsigned int j;
  unsigned int bufferPointer = 0;
  aliasMarkov32Element* am32e = NULL;
  aliasMarkov64Element* am64e = NULL;
  uint64_t subSpace[ALPHABET_SIZE];
	
	
  for (length = 1; length <= MAX_MARKOV_SIZE; length++){
    if (m0dp->rootValue[length-1] != 0 && m0dp->rootValue[length-1] != (uint64_t)0xffffffffffffffff){
      for (j = 0; j < ALPHABET_SIZE; j++){
	subSpace[j] = getMarkov0DynProgFirstLine(m0dp, mis, j, length);
      }
      if (m0dp->rootValue[length-1] & (uint64_t)0xffffffff00000000){
	am64e = (aliasMarkov64Element*)(bufferRoot + bufferPointer);
	initAliasMarkov64Element(am64e, m0dp->rootValue[length-1], subSpace);
	amdp->rootValue[length-1] = bufferPointer;
	bufferPointer += sizeof(aliasMarkov64Element);
      }
      else{
	am32e = (aliasMarkov32Element*)(bufferRoot + bufferPointer);
	initAliasMarkov32Element(am32e, m0dp->rootValue[length-1], subSpace);
	amdp->rootValue[length-1] = bufferPointer;
	bufferPointer += sizeof(aliasMarkov32Element);
      }
    }
    else{
      amdp->rootValue[length-1] = (unsigned int)0xffffffff;
    }
  }
}

void fillAliasMarkov0BufferFLine(markov0DynProg* m0dp, AmarkovDynProg* amdp, markovImprobaStruct* mis, char* bufferFLine){
  unsigned int length;
  unsigned int c;
  unsigned int j;
  unsigned int bufferPointer = 0;
  aliasMarkov32Element* am32e = NULL;
  aliasMarkov64Element* am64e = NULL;
  uint64_t subSpace[ALPHABET_SIZE];
	
	
  for (length = 2; length <= MAX_MARKOV_SIZE; length++){
    for (c = 0; c < ALPHABET_SIZE; c++){
      if (m0dp->firstLine[c + (length-2)*ALPHABET_SIZE] != 0 && m0dp->firstLine[c + (length-2)*ALPHABET_SIZE] != (uint64_t)0xffffffffffffffff){
	for (j = 0; j < ALPHABET_SIZE; j++){
	  subSpace[j] = getMarkov0DynProgSpace(m0dp, mis, 2+(MAX_MARKOV_SIZE-length), mis->improbaFirst[c]+mis->improbaMark0[j]+(MAX_MARKOV_SIZE-length)*mis->meanImprobaMark0);
	}
	if (m0dp->firstLine[c + (length-2)*ALPHABET_SIZE] & (uint64_t)0xffffffff00000000){
	  am64e = (aliasMarkov64Element*)(bufferFLine + bufferPointer);
	  initAliasMarkov64Element(am64e, m0dp->firstLine[c + (length-2)*ALPHABET_SIZE], subSpace);
	  amdp->firstLine[c + (length-2)*ALPHABET_SIZE] = bufferPointer;
	  bufferPointer += sizeof(aliasMarkov64Element);
	}
	else{
	  am32e = (aliasMarkov32Element*)(bufferFLine + bufferPointer);
	  initAliasMarkov32Element(am32e, m0dp->firstLine[c + (length-2)*ALPHABET_SIZE], subSpace);
	  amdp->firstLine[c + (length-2)*ALPHABET_SIZE] = bufferPointer;
	  bufferPointer += sizeof(aliasMarkov32Element);
	}
      }
      else{
	amdp->firstLine[c + (length-2)*ALPHABET_SIZE] = (unsigned int)0xffffffff;
      }
    }
  }
}

void fillAliasMarkov0BufferMain(markov0DynProg* m0dp, AmarkovDynProg* amdp, markovImprobaStruct* mis, char* bufferMain){
  unsigned int length;
  unsigned int level;
  unsigned int j;
  unsigned int bufferPointer = 0;
  aliasMarkov32Element* am32e = NULL;
  aliasMarkov64Element* am64e = NULL;
  uint64_t subSpace[ALPHABET_SIZE];
	
	
  for (length = 2; length < MAX_MARKOV_SIZE; length++){
    for (level = 2*m0dp->minImproba; level < m0dp->threshold; level++){
      if (!(level > m0dp->minImproba*(length-(int)MAX_MARKOV_SIZE)+m0dp->threshold) && !(level <= length*m0dp->minImproba -1)){
	if (m0dp->buffer[(length-2)*m0dp->bufferWidth+level-length*m0dp->minImproba] != 0 && m0dp->buffer[(length-2)*m0dp->bufferWidth+level-length*m0dp->minImproba] != (uint64_t)0xffffffffffffffff){
	  for (j = 0; j < ALPHABET_SIZE; j++){
	    subSpace[j] = getMarkov0DynProgSpace(m0dp, mis, length+1, level+mis->improbaMark0[j]);
	  }
	  if (m0dp->buffer[(length-2)*m0dp->bufferWidth+level-length*m0dp->minImproba] & (uint64_t)0xffffffff00000000){
	    am64e = (aliasMarkov64Element*)(bufferMain + bufferPointer);
	    initAliasMarkov64Element(am64e, m0dp->buffer[(length-2)*m0dp->bufferWidth+level-length*m0dp->minImproba], subSpace);
	    amdp->buffer[(length-2)*m0dp->bufferWidth+level-length*m0dp->minImproba] = bufferPointer;
	    bufferPointer += sizeof(aliasMarkov64Element);
	  }
	  else{
	    am32e = (aliasMarkov32Element*)(bufferMain + bufferPointer);
	    initAliasMarkov32Element(am32e, m0dp->buffer[(length-2)*m0dp->bufferWidth+level-length*m0dp->minImproba], subSpace);
	    amdp->buffer[(length-2)*m0dp->bufferWidth+level-length*m0dp->minImproba] = bufferPointer;
	    bufferPointer += sizeof(aliasMarkov32Element);
	  }
	}
	else{
	  amdp->buffer[(length-2)*m0dp->bufferWidth+level-length*m0dp->minImproba] = (unsigned int)0xffffffff;
	}
      }
    }
  }
}

uint64_t getMark1BufferSizeRoot(markov1DynProg* m1dp){
  unsigned int i;
  uint64_t result = 0;
	
  for (i = 0; i < MAX_MARKOV_SIZE; i++){
    if (m1dp->rootValue[i] != 0 && m1dp->rootValue[i] != (uint64_t)0xffffffffffffffff){
      if (m1dp->rootValue[i] & (uint64_t)0xffffffff00000000){
	result += sizeof(aliasMarkov64Element);
      }
      else{
	result += sizeof(aliasMarkov32Element);
      }
    }
  }
	
  return result;
}

uint64_t getMark1BufferSizeFLine(markov1DynProg* m1dp){
  unsigned int i;
  uint64_t result = 0;
	
  for (i = 0; i < (MAX_MARKOV_SIZE-1)*ALPHABET_SIZE; i++){
    if (m1dp->firstLine[i] != 0 && m1dp->firstLine[i] != (uint64_t)0xffffffffffffffff){
      if (m1dp->firstLine[i] & (uint64_t)0xffffffff00000000){
	result += sizeof(aliasMarkov64Element);
      }
      else{
	result += sizeof(aliasMarkov32Element);
      }
    }
  }
	
  return result;
}

uint64_t getMark1BufferSizeMain(markov1DynProg* m1dp){
  unsigned int i;
  unsigned int c;
  uint64_t result = 0;
	
  for (c = 0; c < ALPHABET_SIZE; c++){
    for (i = 0; i < m1dp->bufferWidth*(MAX_MARKOV_SIZE-2); i++){
      if (m1dp->buffer[c][i] != 0 && m1dp->buffer[c][i] != (uint64_t)0xffffffffffffffff){
	if (m1dp->buffer[c][i] & (uint64_t)0xffffffff00000000){
	  result += sizeof(aliasMarkov64Element);
	}
	else{
	  result += sizeof(aliasMarkov32Element);
	}
      }
    }
  }
  return result;
}

void fillAliasMarkov1BufferRoot(markov1DynProg* m1dp, AmarkovDynProg* amdp, markovImprobaStruct* mis, char* bufferRoot){
  unsigned int length;
  unsigned int j;
  unsigned int bufferPointer = 0;
  aliasMarkov32Element* am32e = NULL;
  aliasMarkov64Element* am64e = NULL;
  uint64_t subSpace[ALPHABET_SIZE];
	
  for (length = 1; length <= MAX_MARKOV_SIZE; length++){
    if (m1dp->rootValue[length-1] != 0 && m1dp->rootValue[length-1] != (uint64_t)0xffffffffffffffff){
      for (j = 0; j < ALPHABET_SIZE; j++){
	subSpace[j] = getMarkov1DynProgFirstLine(m1dp, mis, j, length);
      }
      if (m1dp->rootValue[length-1] & (uint64_t)0xffffffff00000000){
	am64e = (aliasMarkov64Element*)(bufferRoot + bufferPointer);
	initAliasMarkov64Element(am64e, m1dp->rootValue[length-1], subSpace);
	amdp->rootValue[length-1] = bufferPointer;
	bufferPointer += sizeof(aliasMarkov64Element);
      }
      else{
	am32e = (aliasMarkov32Element*)(bufferRoot + bufferPointer);
	initAliasMarkov32Element(am32e, m1dp->rootValue[length-1], subSpace);
	amdp->rootValue[length-1] = bufferPointer;
	bufferPointer += sizeof(aliasMarkov32Element);
      }
    }
    else{
      amdp->rootValue[length-1] = (unsigned int)0xffffffff;
    }
  }
}

void fillAliasMarkov1BufferFLine(markov1DynProg* m1dp, AmarkovDynProg* amdp, markovImprobaStruct* mis, char* bufferFLine){
  unsigned int length;
  unsigned int c;
  unsigned int j;
  unsigned int bufferPointer = 0;
  aliasMarkov32Element* am32e = NULL;
  aliasMarkov64Element* am64e = NULL;
  uint64_t subSpace[ALPHABET_SIZE];
	
	
  for (length = 2; length <= MAX_MARKOV_SIZE; length++){
    for (c = 0; c < ALPHABET_SIZE; c++){
      if (m1dp->firstLine[c + (length-2)*ALPHABET_SIZE] != 0 && m1dp->firstLine[c + (length-2)*ALPHABET_SIZE] != (uint64_t)0xffffffffffffffff){
	for (j = 0; j < ALPHABET_SIZE; j++){
	  subSpace[j] = getMarkov1DynProgSpace(m1dp, mis, 2+(MAX_MARKOV_SIZE-length), mis->improbaFirst[c]+mis->improbaMark1[c][j]+(MAX_MARKOV_SIZE-length)*mis->meanImprobaMark0, j);
	}
	if (m1dp->firstLine[c + (length-2)*ALPHABET_SIZE] & (uint64_t)0xffffffff00000000){
	  am64e = (aliasMarkov64Element*)(bufferFLine + bufferPointer);
	  initAliasMarkov64Element(am64e, m1dp->firstLine[c + (length-2)*ALPHABET_SIZE], subSpace);
	  amdp->firstLine[c + (length-2)*ALPHABET_SIZE] = bufferPointer;
	  bufferPointer += sizeof(aliasMarkov64Element);
	}
	else{
	  am32e = (aliasMarkov32Element*)(bufferFLine + bufferPointer);
	  initAliasMarkov32Element(am32e, m1dp->firstLine[c + (length-2)*ALPHABET_SIZE], subSpace);
	  amdp->firstLine[c + (length-2)*ALPHABET_SIZE] = bufferPointer;
	  bufferPointer += sizeof(aliasMarkov32Element);
	}
      }
      else{
	amdp->firstLine[c + (length-2)*ALPHABET_SIZE] = (unsigned int)0xffffffff;
      }
    }
  }
}

void fillAliasMarkov1BufferMain(markov1DynProg* m1dp, AmarkovDynProg* amdp, markovImprobaStruct* mis, char* bufferMain){
  unsigned int length;
  unsigned int level;
  unsigned int prevChar1;
  unsigned int j;
  unsigned int bufferPointer = 0;
  aliasMarkov32Element* am32e = NULL;
  aliasMarkov64Element* am64e = NULL;
  uint64_t subSpace[ALPHABET_SIZE];
	
	
  for (length = 2; length < MAX_MARKOV_SIZE; length++){
    for (level = 2*m1dp->minImproba; level < m1dp->threshold; level++){
      if (!(level > m1dp->minImproba*(length-(int)MAX_MARKOV_SIZE)+m1dp->threshold) && !(level <= length*m1dp->minImproba -1)){
	for (prevChar1 = 0; prevChar1 < ALPHABET_SIZE; prevChar1++){
	  if (m1dp->buffer[prevChar1][(length-2)*m1dp->bufferWidth+level-length*m1dp->minImproba] != 0 && m1dp->buffer[prevChar1][(length-2)*m1dp->bufferWidth+level-length*m1dp->minImproba] != (uint64_t)0xffffffffffffffff){
	    for (j = 0; j < ALPHABET_SIZE; j++){
	      subSpace[j] = getMarkov1DynProgSpace(m1dp, mis, length+1, level+mis->improbaMark1[prevChar1][j], j);
	    }
	    if (m1dp->buffer[prevChar1][(length-2)*m1dp->bufferWidth+level-length*m1dp->minImproba] & (uint64_t)0xffffffff00000000){
	      am64e = (aliasMarkov64Element*)(bufferMain + bufferPointer);
	      initAliasMarkov64Element(am64e, m1dp->buffer[prevChar1][(length-2)*m1dp->bufferWidth+level-length*m1dp->minImproba], subSpace);
	      amdp->buffer[((length-2)*m1dp->bufferWidth+level-length*m1dp->minImproba)*ALPHABET_SIZE+prevChar1] = bufferPointer;
	      bufferPointer += sizeof(aliasMarkov64Element);
	    }
	    else{
	      am32e = (aliasMarkov32Element*)(bufferMain + bufferPointer);
	      initAliasMarkov32Element(am32e, m1dp->buffer[prevChar1][(length-2)*m1dp->bufferWidth+level-length*m1dp->minImproba], subSpace);
	      amdp->buffer[((length-2)*m1dp->bufferWidth+level-length*m1dp->minImproba)*ALPHABET_SIZE+prevChar1] = bufferPointer;
	      bufferPointer += sizeof(aliasMarkov32Element);
	    }
	  }
	  else{
	    amdp->buffer[((length-2)*m1dp->bufferWidth+level-length*m1dp->minImproba)*ALPHABET_SIZE+prevChar1] = (unsigned int)0xffffffff;
	  }
	}
      }
    }
  }
}

uint64_t getMark2BufferSizeRoot(markov2DynProg* m2dp){
  unsigned int i;
  uint64_t result = 0;
	
  for (i = 0; i < MAX_MARKOV_SIZE; i++){
    if (m2dp->rootValue[i] != 0 && m2dp->rootValue[i] != (uint64_t)0xffffffffffffffff){
      if (m2dp->rootValue[i] & (uint64_t)0xffffffff00000000){
	result += sizeof(aliasMarkov64Element);
      }
      else{
	result += sizeof(aliasMarkov32Element);
      }
    }
  }
	
  return result;
}

uint64_t getMark2BufferSizeFLine(markov2DynProg* m2dp){
  unsigned int i;
  uint64_t result = 0;
	
  for (i = 0; i < (MAX_MARKOV_SIZE-1)*ALPHABET_SIZE; i++){
    if (m2dp->firstLine[i] != 0 && m2dp->firstLine[i] != (uint64_t)0xffffffffffffffff){
      if (m2dp->firstLine[i] & (uint64_t)0xffffffff00000000){
	result += sizeof(aliasMarkov64Element);
      }
      else{
	result += sizeof(aliasMarkov32Element);
      }
    }
  }
	
  return result;
}

uint64_t getMark2BufferSizeMain(markov2DynProg* m2dp){
  unsigned int i;
  unsigned int c1;
  unsigned int c2;
  uint64_t result = 0;
	
	
  for (c2 = 0; c2 < ALPHABET_SIZE; c2++){
    for (c1 = 0; c1 < ALPHABET_SIZE; c1++){
      for (i = 0; i < m2dp->bufferWidth*(MAX_MARKOV_SIZE-2); i++){
	if (m2dp->buffer[c2][c1][i] != 0 && m2dp->buffer[c2][c1][i] != (uint64_t)0xffffffffffffffff){
	  if (m2dp->buffer[c2][c1][i] & (uint64_t)0xffffffff00000000){
	    result += sizeof(aliasMarkov64Element);
	  }
	  else{
	    result += sizeof(aliasMarkov32Element);
	  }
	}
      }
    }
  }
  return result;
}

void fillAliasMarkov2BufferRoot(markov2DynProg* m2dp, AmarkovDynProg* amdp, markovImprobaStruct* mis, char* bufferRoot){
  unsigned int length;
  unsigned int j;
  unsigned int bufferPointer = 0;
  aliasMarkov32Element* am32e = NULL;
  aliasMarkov64Element* am64e = NULL;
  uint64_t subSpace[ALPHABET_SIZE];
	
  for (length = 1; length <= MAX_MARKOV_SIZE; length++){
    if (m2dp->rootValue[length-1] != 0 && m2dp->rootValue[length-1] != (uint64_t)0xffffffffffffffff){
      for (j = 0; j < ALPHABET_SIZE; j++){
	subSpace[j] = getMarkov2DynProgFirstLine(m2dp, mis, j, length);
      }
      if (m2dp->rootValue[length-1] & (uint64_t)0xffffffff00000000){
	am64e = (aliasMarkov64Element*)(bufferRoot + bufferPointer);
	initAliasMarkov64Element(am64e, m2dp->rootValue[length-1], subSpace);
	amdp->rootValue[length-1] = bufferPointer;
	bufferPointer += sizeof(aliasMarkov64Element);
      }
      else{
	am32e = (aliasMarkov32Element*)(bufferRoot + bufferPointer);
	initAliasMarkov32Element(am32e, m2dp->rootValue[length-1], subSpace);
	amdp->rootValue[length-1] = bufferPointer;
	bufferPointer += sizeof(aliasMarkov32Element);
      }
    }
    else{
      amdp->rootValue[length-1] = (unsigned int)0xffffffff;
    }
  }
}

void fillAliasMarkov2BufferFLine(markov2DynProg* m2dp, AmarkovDynProg* amdp, markovImprobaStruct* mis, char* bufferFLine){
  unsigned int length;
  unsigned int c;
  unsigned int j;
  unsigned int bufferPointer = 0;
  aliasMarkov32Element* am32e = NULL;
  aliasMarkov64Element* am64e = NULL;
  uint64_t subSpace[ALPHABET_SIZE];
	
	
  for (length = 2; length <= MAX_MARKOV_SIZE; length++){
    for (c = 0; c < ALPHABET_SIZE; c++){
      if (m2dp->firstLine[c + (length-2)*ALPHABET_SIZE] != 0 && m2dp->firstLine[c + (length-2)*ALPHABET_SIZE] != (uint64_t)0xffffffffffffffff){
	for (j = 0; j < ALPHABET_SIZE; j++){
	  subSpace[j] = getMarkov2DynProgSpace(m2dp, mis, 2+(MAX_MARKOV_SIZE-length), mis->improbaFirst[c]+mis->improbaMark1[c][j]+(MAX_MARKOV_SIZE-length)*mis->meanImprobaMark0, j, c);
	}
	if (m2dp->firstLine[c + (length-2)*ALPHABET_SIZE] & (uint64_t)0xffffffff00000000){
	  am64e = (aliasMarkov64Element*)(bufferFLine + bufferPointer);
	  initAliasMarkov64Element(am64e, m2dp->firstLine[c + (length-2)*ALPHABET_SIZE], subSpace);
	  amdp->firstLine[c + (length-2)*ALPHABET_SIZE] = bufferPointer;
	  bufferPointer += sizeof(aliasMarkov64Element);
	}
	else{
	  am32e = (aliasMarkov32Element*)(bufferFLine + bufferPointer);
	  initAliasMarkov32Element(am32e, m2dp->firstLine[c + (length-2)*ALPHABET_SIZE], subSpace);
	  amdp->firstLine[c + (length-2)*ALPHABET_SIZE] = bufferPointer;
	  bufferPointer += sizeof(aliasMarkov32Element);
	}
      }
      else{
	amdp->firstLine[c + (length-2)*ALPHABET_SIZE] = (unsigned int)0xffffffff;
      }
    }
  }
}

void fillAliasMarkov2BufferMain(markov2DynProg* m2dp, AmarkovDynProg* amdp, markovImprobaStruct* mis, char* bufferMain){
  unsigned int length;
  unsigned int level;
  unsigned int prevChar1;
  unsigned int prevChar2;
  unsigned int j;
  unsigned int bufferPointer = 0;
  aliasMarkov32Element* am32e = NULL;
  aliasMarkov64Element* am64e = NULL;
  uint64_t subSpace[ALPHABET_SIZE];
	
	
  for (length = 2; length < MAX_MARKOV_SIZE; length++){
    for (level = 2*m2dp->minImproba; level < m2dp->threshold; level++){
      if (!(level > m2dp->minImproba*(length-(int)MAX_MARKOV_SIZE)+m2dp->threshold) && !(level <= length*m2dp->minImproba -1)){
	for (prevChar1 = 0; prevChar1 < ALPHABET_SIZE; prevChar1++){
	  for (prevChar2 = 0; prevChar2 < ALPHABET_SIZE; prevChar2++){
	    if (m2dp->buffer[prevChar2][prevChar1][(length-2)*m2dp->bufferWidth+level-length*m2dp->minImproba] != 0 && m2dp->buffer[prevChar2][prevChar1][(length-2)*m2dp->bufferWidth+level-length*m2dp->minImproba] != (uint64_t)0xffffffffffffffff){
	      for (j = 0; j < ALPHABET_SIZE; j++){
		subSpace[j] = getMarkov2DynProgSpace(m2dp, mis, length+1, level+mis->improbaMark2[prevChar2][prevChar1][j], j, prevChar1);
	      }
	      if (m2dp->buffer[prevChar2][prevChar1][(length-2)*m2dp->bufferWidth+level-length*m2dp->minImproba] & (uint64_t)0xffffffff00000000){
		am64e = (aliasMarkov64Element*)(bufferMain + bufferPointer);
		initAliasMarkov64Element(am64e, m2dp->buffer[prevChar2][prevChar1][(length-2)*m2dp->bufferWidth+level-length*m2dp->minImproba], subSpace);
		amdp->buffer[(((length-2)*m2dp->bufferWidth+level-length*m2dp->minImproba)*ALPHABET_SIZE+prevChar1)*ALPHABET_SIZE+prevChar2] = bufferPointer;
		bufferPointer += sizeof(aliasMarkov64Element);
	      }
	      else{
		am32e = (aliasMarkov32Element*)(bufferMain + bufferPointer);
		initAliasMarkov32Element(am32e, m2dp->buffer[prevChar2][prevChar1][(length-2)*m2dp->bufferWidth+level-length*m2dp->minImproba], subSpace);
		amdp->buffer[(((length-2)*m2dp->bufferWidth+level-length*m2dp->minImproba)*ALPHABET_SIZE+prevChar1)*ALPHABET_SIZE+prevChar2] = bufferPointer;
		bufferPointer += sizeof(aliasMarkov32Element);
	      }
	    }
	    else{
	      amdp->buffer[(((length-2)*m2dp->bufferWidth+level-length*m2dp->minImproba)*ALPHABET_SIZE+prevChar1)*ALPHABET_SIZE+prevChar2] = (unsigned int)0xffffffff;
	    }
	  }
	}
      }
    }
  }
}
