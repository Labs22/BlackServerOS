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
#include <sys/mman.h>
#include <ctype.h>

#include "markov.h"
#include "aliasMarkov.h"

#define IMPROBA_PENALTY 100
#define IMPROBA_MIN_VALUE 1

void initAMarkov0DynProg(AmarkovDynProg* amdp, markov0DynProg* m0dp, markovImprobaStruct* mis);
void deleteMarkov0DynProg(markov0DynProg* m0dp);

void initMarkov1DynProg(AmarkovDynProg* amdp, markov1DynProg* m1dp, markovImprobaStruct* mis);
void deleteMarkov1DynProg(markov1DynProg* m1dp);

void initMarkov2DynProg(AmarkovDynProg* amdp, markov2DynProg* m2dp, markovImprobaStruct* mis);
void deleteMarkov2DynProg(markov2DynProg* m2dp);

void deleteAMarkovDynProg(AmarkovDynProg* amdp);


void deleteImprobaStruct(markovImprobaStruct* mis){
  if (mis != NULL){
    munmap(mis, sizeof(markovImprobaStruct));
  }
}

void initAMarkov0DynProg(AmarkovDynProg* amdp, markov0DynProg* m0dp, markovImprobaStruct* mis){
  unsigned int minImproba = mis->improbaMark0[0];
  unsigned int i;
	
  for (i = 0; i < ALPHABET_SIZE; i++){
    if (mis->improbaMark0[i] < minImproba){
      minImproba = mis->improbaMark0[i];
    }
    if (mis->improbaFirst[i] < minImproba){
      minImproba = mis->improbaFirst[i];
    }
  }
	
  m0dp->rootValue 	= (uint64_t*)malloc(sizeof(uint64_t)*MAX_MARKOV_SIZE);
  m0dp->firstLine 	= (uint64_t*)malloc(sizeof(uint64_t)*ALPHABET_SIZE*(MAX_MARKOV_SIZE-1));
  m0dp->buffer 		= (uint64_t*)malloc(sizeof(uint64_t)*(1+MAX_MARKOV_SIZE*(mis->meanImprobaMark0-minImproba))*(MAX_MARKOV_SIZE-2));
  m0dp->minImproba 	= minImproba;
  m0dp->bufferWidth 	= 1+MAX_MARKOV_SIZE*(mis->meanImprobaMark0-minImproba);
  m0dp->threshold 	= mis->meanImprobaMark0*MAX_MARKOV_SIZE;
	
  amdp->rootValue 	= (unsigned int*)malloc(sizeof(unsigned int)*MAX_MARKOV_SIZE);
  amdp->firstLine 	= (unsigned int*)malloc(sizeof(unsigned int)*ALPHABET_SIZE*(MAX_MARKOV_SIZE-1));
  amdp->buffer 		= (unsigned int*)malloc(sizeof(unsigned int)*(1+MAX_MARKOV_SIZE*(mis->meanImprobaMark0-minImproba))*(MAX_MARKOV_SIZE-2));
  amdp->minImproba 	= minImproba;
  amdp->bufferWidth 	= 1+MAX_MARKOV_SIZE*(mis->meanImprobaMark0-minImproba);
  amdp->threshold 	= mis->meanImprobaMark0*MAX_MARKOV_SIZE;
  amdp->amb		= (aliasMarkovBuffer*)malloc(sizeof(aliasMarkovBuffer));
	
  for(i = 0; i < MAX_MARKOV_SIZE; i++)
    m0dp->rootValue[i] = (uint64_t)0xffffffffffffffff;
  
  for (i = 0; i < ALPHABET_SIZE*(MAX_MARKOV_SIZE-1); i++)
    m0dp->firstLine[i] = (uint64_t)0xffffffffffffffff;
  
  for (i = 0; i < m0dp->bufferWidth*(MAX_MARKOV_SIZE-2); i++)
    m0dp->buffer[i] = (uint64_t)0xffffffffffffffff;
  
	
  for (i = 1; i <= MAX_MARKOV_SIZE; i++)
    getMarkov0DynProgRootValue(m0dp, mis, i);
  
	
  amdp->amb->size_bufferRoot =  getMark0BufferSizeRoot(m0dp);
  amdp->amb->size_bufferFLine = getMark0BufferSizeFLine(m0dp);
  amdp->amb->size_bufferMain = getMark0BufferSizeMain(m0dp);
	
  amdp->amb->bufferRoot = malloc(amdp->amb->size_bufferRoot);
  amdp->amb->bufferFLine = malloc(amdp->amb->size_bufferFLine);
  amdp->amb->bufferMain = malloc(amdp->amb->size_bufferMain);
	
  fillAliasMarkov0BufferRoot(m0dp, amdp, mis, amdp->amb->bufferRoot);
  fillAliasMarkov0BufferFLine(m0dp, amdp, mis, amdp->amb->bufferFLine);
  fillAliasMarkov0BufferMain(m0dp, amdp, mis, amdp->amb->bufferMain);
}

uint64_t getMarkov0DynProgRootValue(markov0DynProg* m0dp, markovImprobaStruct* mis, unsigned int requiredLength){
  if (requiredLength == 0 || requiredLength > MAX_MARKOV_SIZE){
    return 0;
  }
  if (m0dp->rootValue[requiredLength-1] == (uint64_t)0xffffffffffffffff){
    unsigned int i;
    uint64_t result = 0;
		
    for (i = 0; i < ALPHABET_SIZE; i++)
      result += getMarkov0DynProgFirstLine(m0dp, mis, i, requiredLength);
    
			
    m0dp->rootValue[requiredLength-1] = result;
    return result;
  }
  else
    return m0dp->rootValue[requiredLength-1];
}

uint64_t getMarkov0DynProgFirstLine(markov0DynProg* m0dp, markovImprobaStruct* mis, unsigned int index, unsigned int requiredLength){
  if (requiredLength == 1){
    if (mis->improbaFirst[index] > mis->meanImprobaMark0)
      return 0;
    else
      return 1;
  }
  if (m0dp->firstLine[index + (requiredLength-2)*ALPHABET_SIZE] == (uint64_t)0xffffffffffffffff){
    unsigned int i;
    uint64_t result = 0;
		
    for (i = 0; i < ALPHABET_SIZE; i++)
      result += getMarkov0DynProgSpace(m0dp, mis, 2+(MAX_MARKOV_SIZE-requiredLength), mis->improbaFirst[index]+mis->improbaMark0[i]+(MAX_MARKOV_SIZE-requiredLength)*mis->meanImprobaMark0);
    
		
    m0dp->firstLine[index + (requiredLength-2)*ALPHABET_SIZE] = result;
    return result;
  }
  else 
    return m0dp->firstLine[index + (requiredLength-2)*ALPHABET_SIZE];
}

uint64_t getMarkov0DynProgSpace(markov0DynProg* m0dp, markovImprobaStruct* mis, unsigned int length, unsigned int level){
  if (level > m0dp->threshold)
    return 0;
  if (length == MAX_MARKOV_SIZE)
    return 1;
  if (level > m0dp->minImproba*(length-(int)MAX_MARKOV_SIZE)+m0dp->threshold)
    return 0;

  if (m0dp->buffer[(length-2)*m0dp->bufferWidth+level-length*m0dp->minImproba] == (uint64_t)0xffffffffffffffff){
    unsigned int i;
    uint64_t result = 0;
		
    for (i = 0; i < ALPHABET_SIZE; i++)
      result += getMarkov0DynProgSpace(m0dp, mis, length+1, level+mis->improbaMark0[i]);
		
    m0dp->buffer[(length-2)*m0dp->bufferWidth+level-length*m0dp->minImproba] = result;
    return result;
  }
  else
    return m0dp->buffer[(length-2)*m0dp->bufferWidth+level-length*m0dp->minImproba];
	
  return 0;
}

void deleteMarkov0DynProg(markov0DynProg* m0dp){
  if (m0dp != NULL){
    if (m0dp->rootValue != NULL)
      free(m0dp->rootValue);
    if (m0dp->firstLine != NULL)
      free(m0dp->firstLine);
    if (m0dp->buffer != NULL)
      free(m0dp->buffer);
  }
}

void deleteAMarkovDynProg(AmarkovDynProg* amdp){
  if (amdp != NULL){
    if (amdp->rootValue != NULL)
      free(amdp->rootValue);
     if (amdp->firstLine != NULL)
      free(amdp->firstLine);
     if (amdp->buffer != NULL)
      free(amdp->buffer);
     if (amdp->amb != NULL){
      free(amdp->amb->bufferRoot);
      free(amdp->amb->bufferFLine);
      free(amdp->amb->bufferMain);
      free(amdp->amb);
    }
  }
}

void initMarkov1DynProg(AmarkovDynProg* amdp, markov1DynProg* m1dp, markovImprobaStruct* mis){	
  unsigned int minImproba = mis->improbaMark1[0][0];
  unsigned int i;
  unsigned int c;
	
  m1dp->rootValue = (uint64_t*)malloc(sizeof(uint64_t)*MAX_MARKOV_SIZE);
  m1dp->firstLine = (uint64_t*)malloc(sizeof(uint64_t)*ALPHABET_SIZE*(MAX_MARKOV_SIZE-1));
  m1dp->threshold = mis->meanImprobaMark0*MAX_MARKOV_SIZE;
	
  amdp->rootValue = (unsigned int*)malloc(sizeof(unsigned int)*MAX_MARKOV_SIZE);
  amdp->firstLine = (unsigned int*)malloc(sizeof(unsigned int)*ALPHABET_SIZE*(MAX_MARKOV_SIZE-1));
  amdp->threshold = mis->meanImprobaMark0*MAX_MARKOV_SIZE;
  amdp->amb = (aliasMarkovBuffer*)malloc(sizeof(aliasMarkovBuffer));
	
  for(i = 0; i < MAX_MARKOV_SIZE; i++)
    m1dp->rootValue[i] = (uint64_t)0xffffffffffffffff;

  for (i = 0; i < ALPHABET_SIZE*(MAX_MARKOV_SIZE-1); i++)
    m1dp->firstLine[i] = (uint64_t)0xffffffffffffffff;
	
  for (c = 0; c < ALPHABET_SIZE; c++)
    for (i = 0; i < ALPHABET_SIZE; i++)
      if (mis->improbaMark1[c][i] < minImproba)
	minImproba = mis->improbaMark1[c][i];

  for (i = 0; i < ALPHABET_SIZE; i++)
    if (mis->improbaFirst[i] < minImproba)
      minImproba = mis->improbaFirst[i];
	
  m1dp->minImproba = minImproba;
  m1dp->bufferWidth = 1+MAX_MARKOV_SIZE*(mis->meanImprobaMark0-minImproba);
  amdp->minImproba = minImproba;
  amdp->bufferWidth = 1+MAX_MARKOV_SIZE*(mis->meanImprobaMark0-minImproba);
  amdp->buffer = (unsigned int*)malloc(sizeof(unsigned int)*ALPHABET_SIZE*m1dp->bufferWidth*(MAX_MARKOV_SIZE-2));
	
  for (c = 0; c <ALPHABET_SIZE; c++){
    m1dp->buffer[c] = (uint64_t*)malloc(sizeof(uint64_t)*m1dp->bufferWidth*(MAX_MARKOV_SIZE-2));
    for (i = 0; i < m1dp->bufferWidth*(MAX_MARKOV_SIZE-2); i++)
      m1dp->buffer[c][i] = (uint64_t)0xffffffffffffffff;
  }
	
  for (i = 1; i <= MAX_MARKOV_SIZE; i++)
    getMarkov1DynProgRootValue(m1dp, mis, i);
	
  amdp->amb->size_bufferRoot =  getMark1BufferSizeRoot(m1dp);
  amdp->amb->size_bufferFLine = getMark1BufferSizeFLine(m1dp);
  amdp->amb->size_bufferMain = getMark1BufferSizeMain(m1dp);
	
  amdp->amb->bufferRoot = malloc(amdp->amb->size_bufferRoot);
  amdp->amb->bufferFLine = malloc(amdp->amb->size_bufferFLine);
  amdp->amb->bufferMain = malloc(amdp->amb->size_bufferMain);
	
  fillAliasMarkov1BufferRoot(m1dp, amdp, mis, amdp->amb->bufferRoot);
  fillAliasMarkov1BufferFLine(m1dp, amdp, mis, amdp->amb->bufferFLine);
  fillAliasMarkov1BufferMain(m1dp, amdp, mis, amdp->amb->bufferMain);
}

uint64_t getMarkov1DynProgRootValue(markov1DynProg* m1dp, markovImprobaStruct* mis, unsigned int requiredLength){
  if (requiredLength == 0 || requiredLength > MAX_MARKOV_SIZE)
    return 0;

  if (m1dp->rootValue[requiredLength-1] == (uint64_t)0xffffffffffffffff){
    unsigned int i;
    uint64_t result = 0;
		
    for (i = 0; i < ALPHABET_SIZE; i++)
      result += getMarkov1DynProgFirstLine(m1dp, mis, i, requiredLength);
			
    m1dp->rootValue[requiredLength-1] = result;
    return result;
  }
  else
    return m1dp->rootValue[requiredLength-1];
}

uint64_t getMarkov1DynProgFirstLine(markov1DynProg* m1dp, markovImprobaStruct* mis, unsigned int index, unsigned int requiredLength){
  if (requiredLength == 1) {
    if ((mis->improbaFirst[index]+(MAX_MARKOV_SIZE-1)*mis->meanImprobaMark0) > m1dp->threshold)
      return 0;
    else
      return 1;
  }
  if (m1dp->firstLine[index + (requiredLength-2)*ALPHABET_SIZE] == (uint64_t)0xffffffffffffffff){
    unsigned int i;
    uint64_t result = 0;
		
    for (i = 0; i < ALPHABET_SIZE; i++)
      result += getMarkov1DynProgSpace(m1dp, mis, 2+(MAX_MARKOV_SIZE-requiredLength), mis->improbaFirst[index]+mis->improbaMark1[index][i]+(MAX_MARKOV_SIZE-requiredLength)*mis->meanImprobaMark0, i);
		
    m1dp->firstLine[index + (requiredLength-2)*ALPHABET_SIZE] = result;
    return result;
  }
  else 
    return m1dp->firstLine[index + (requiredLength-2)*ALPHABET_SIZE];
}

uint64_t getMarkov1DynProgSpace(markov1DynProg* m1dp, markovImprobaStruct* mis, unsigned int length, unsigned int level, unsigned int prevChar){
  if (level > m1dp->threshold)
    return 0;

  if (length == MAX_MARKOV_SIZE)
    return 1;

  if (level > m1dp->minImproba*(length-(int)MAX_MARKOV_SIZE)+m1dp->threshold)
    return 0;

  if (m1dp->buffer[prevChar][(length-2)*m1dp->bufferWidth+level-length*m1dp->minImproba] == (uint64_t)0xffffffffffffffff){
    unsigned int i;
    uint64_t result = 0;
		
    for (i = 0; i < ALPHABET_SIZE; i++)
      result += getMarkov1DynProgSpace(m1dp, mis, length+1, level+mis->improbaMark1[prevChar][i], i);
		
    m1dp->buffer[prevChar][(length-2)*m1dp->bufferWidth+level-length*m1dp->minImproba] = result;
    return result;
  }
  else
    return m1dp->buffer[prevChar][(length-2)*m1dp->bufferWidth+level-length*m1dp->minImproba];
  
  return 0;
}

void deleteMarkov1DynProg(markov1DynProg* m1dp){
  unsigned int c;

  if (m1dp != NULL){
    if (m1dp->rootValue != NULL)
      free(m1dp->rootValue);

    if (m1dp->firstLine != NULL)
      free(m1dp->firstLine);

    for (c = 0; c < ALPHABET_SIZE; c++)
      if (m1dp->buffer[c] != NULL)
	free(m1dp->buffer[c]);
  }
}

void initMarkov2DynProg(AmarkovDynProg* amdp, markov2DynProg* m2dp, markovImprobaStruct* mis){
  unsigned int minImproba = mis->improbaMark2[0][0][0];
  unsigned int i;
  unsigned int c1;
  unsigned int c2;
	
  m2dp->rootValue = (uint64_t*)malloc(sizeof(uint64_t)*MAX_MARKOV_SIZE);
  m2dp->firstLine = (uint64_t*)malloc(sizeof(uint64_t)*ALPHABET_SIZE*(MAX_MARKOV_SIZE-1));
  m2dp->threshold = mis->meanImprobaMark0*MAX_MARKOV_SIZE;
	
  amdp->rootValue = (unsigned int*)malloc(sizeof(unsigned int)*MAX_MARKOV_SIZE);
  amdp->firstLine = (unsigned int*)malloc(sizeof(unsigned int)*ALPHABET_SIZE*(MAX_MARKOV_SIZE-1));
  amdp->threshold = mis->meanImprobaMark0*MAX_MARKOV_SIZE;
  amdp->amb = (aliasMarkovBuffer*)malloc(sizeof(aliasMarkovBuffer));
	
  for(i = 0; i < MAX_MARKOV_SIZE; i++)
    m2dp->rootValue[i] = (uint64_t)0xffffffffffffffff;
  
  for (i = 0; i < ALPHABET_SIZE*(MAX_MARKOV_SIZE-1); i++)
    m2dp->firstLine[i] = (uint64_t)0xffffffffffffffff;
	
  for (c1 = 0; c1 < ALPHABET_SIZE; c1++){
    for (c2 = 0; c2 < ALPHABET_SIZE; c2++){
      for (i = 0; i < ALPHABET_SIZE; i++){
	if (mis->improbaMark2[c1][c2][i] < minImproba){
	  minImproba = mis->improbaMark2[c1][c2][i];
	}
      }
    }
    for (i = 0; i < ALPHABET_SIZE; i++){
      if (mis->improbaMark1[c1][i] < minImproba){
	minImproba = mis->improbaMark1[c1][i];
      }
    }
  }
  for (i = 0; i < ALPHABET_SIZE; i++){
    if (mis->improbaFirst[i] < minImproba){
      minImproba = mis->improbaFirst[i];
    }
  }
	
  m2dp->minImproba = minImproba;
  m2dp->bufferWidth = 1+MAX_MARKOV_SIZE*(mis->meanImprobaMark0-minImproba);
  amdp->minImproba = minImproba;
  amdp->bufferWidth = 1+MAX_MARKOV_SIZE*(mis->meanImprobaMark0-minImproba);
  amdp->buffer = (unsigned int*)malloc(sizeof(unsigned int)*ALPHABET_SIZE*ALPHABET_SIZE*m2dp->bufferWidth*(MAX_MARKOV_SIZE-2));
	
  for (c1 = 0; c1 < ALPHABET_SIZE; c1++){
    for (c2 = 0; c2 < ALPHABET_SIZE; c2++){
      m2dp->buffer[c1][c2] = (uint64_t*)malloc(sizeof(uint64_t)*m2dp->bufferWidth*(MAX_MARKOV_SIZE-2));
      for (i = 0; i < m2dp->bufferWidth*(MAX_MARKOV_SIZE-2); i++){
	m2dp->buffer[c1][c2][i] = (uint64_t)0xffffffffffffffff;
      }
    }
  }
	
  for (i = 1; i <= MAX_MARKOV_SIZE; i++)
    getMarkov2DynProgRootValue(m2dp, mis, i);
	
  amdp->amb->size_bufferRoot =  getMark2BufferSizeRoot(m2dp);
  amdp->amb->size_bufferFLine = getMark2BufferSizeFLine(m2dp);
  amdp->amb->size_bufferMain = getMark2BufferSizeMain(m2dp);
	
  amdp->amb->bufferRoot = malloc(amdp->amb->size_bufferRoot);
  amdp->amb->bufferFLine = malloc(amdp->amb->size_bufferFLine);
  amdp->amb->bufferMain = malloc(amdp->amb->size_bufferMain);
	
  fillAliasMarkov2BufferRoot(m2dp, amdp, mis, amdp->amb->bufferRoot);
  fillAliasMarkov2BufferFLine(m2dp, amdp, mis, amdp->amb->bufferFLine);
  fillAliasMarkov2BufferMain(m2dp, amdp, mis, amdp->amb->bufferMain);
}

uint64_t getMarkov2DynProgRootValue(markov2DynProg* m2dp, markovImprobaStruct* mis, unsigned int requiredLength){
  if (requiredLength == 0 || requiredLength > MAX_MARKOV_SIZE)
    return 0;
  
  if (m2dp->rootValue[requiredLength-1] == (uint64_t)0xffffffffffffffff){
    unsigned int i;
    uint64_t result = 0;
		
    for (i = 0; i < ALPHABET_SIZE; i++)
      result += getMarkov2DynProgFirstLine(m2dp, mis, i, requiredLength);
    
    m2dp->rootValue[requiredLength-1] = result;
    return result;
  }
  else
    return m2dp->rootValue[requiredLength-1];
}

uint64_t getMarkov2DynProgFirstLine(markov2DynProg* m2dp, markovImprobaStruct* mis, unsigned int index, unsigned int requiredLength){
  if (requiredLength == 1){
    if ((mis->improbaFirst[index]+(MAX_MARKOV_SIZE-1)*mis->meanImprobaMark0) > m2dp->threshold)
      return 0;
    else
      return 1;
  }
  if (m2dp->firstLine[index + (requiredLength-2)*ALPHABET_SIZE] == (uint64_t)0xffffffffffffffff){
    unsigned int i;
    uint64_t result = 0;
		
    for (i = 0; i < ALPHABET_SIZE; i++)
      result += getMarkov2DynProgSpace(m2dp, mis, 2+(MAX_MARKOV_SIZE-requiredLength), mis->improbaFirst[index]+mis->improbaMark1[index][i]+(MAX_MARKOV_SIZE-requiredLength)*mis->meanImprobaMark0, i, index);
		
    m2dp->firstLine[index + (requiredLength-2)*ALPHABET_SIZE] = result;
    return result;
  }
  else 
    return m2dp->firstLine[index + (requiredLength-2)*ALPHABET_SIZE];
}

uint64_t getMarkov2DynProgSpace(markov2DynProg* m2dp, markovImprobaStruct* mis, unsigned int length, unsigned int level, unsigned int prevChar1, unsigned int prevChar2){
  if (level > m2dp->threshold)
    return 0;

  if (length == MAX_MARKOV_SIZE)
    return 1;

  if (level > m2dp->minImproba*(length-(int)MAX_MARKOV_SIZE)+m2dp->threshold)
    return 0;

  if (m2dp->buffer[prevChar2][prevChar1][(length-2)*m2dp->bufferWidth+level-length*m2dp->minImproba] == (uint64_t)0xffffffffffffffff){
    unsigned int i;
    uint64_t result = 0;
		
    for (i = 0; i < ALPHABET_SIZE; i++)
      result += getMarkov2DynProgSpace(m2dp, mis, length+1, level+mis->improbaMark2[prevChar2][prevChar1][i], i, prevChar1);
		
    m2dp->buffer[prevChar2][prevChar1][(length-2)*m2dp->bufferWidth+level-length*m2dp->minImproba] = result;
    return result;
  }
  else
    return m2dp->buffer[prevChar2][prevChar1][(length-2)*m2dp->bufferWidth+level-length*m2dp->minImproba];
	
  return 1;
}

void deleteMarkov2DynProg(markov2DynProg* m2dp){
  unsigned int c1;
  unsigned int c2;

  if (m2dp != NULL){
    if (m2dp->rootValue != NULL)
      free(m2dp->rootValue);
    
    if (m2dp->firstLine != NULL)
      free(m2dp->firstLine);
    
    for (c1 = 0; c1 < ALPHABET_SIZE; c1++)
      for (c2 = 0; c2 < ALPHABET_SIZE; c2++)
	if (m2dp->buffer[c1][c2] != NULL)
	  free(m2dp->buffer[c1][c2]);
  }
}


int initMarkovDynProgWrapper(markovDynProgWrapper* mdpw, markovImprobaStruct* mis, markovType type){
  mdpw->type = type;
  mdpw->m0dp = NULL;
  mdpw->m1dp = NULL;
  mdpw->m2dp = NULL;
  
  mdpw->amdp = NULL;
  
  switch(mdpw->type){
  case NO_MARK	: {break;}
  case MARK0		: {
    mdpw->m0dp = (markov0DynProg*)malloc(sizeof(markov0DynProg));
    mdpw->amdp = (AmarkovDynProg*)malloc(sizeof(AmarkovDynProg)); 
    initAMarkov0DynProg(mdpw->amdp, mdpw->m0dp, mis);
    break;
  }
  case MARK1		: {
    mdpw->m1dp = (markov1DynProg*)malloc(sizeof(markov1DynProg));
    mdpw->amdp = (AmarkovDynProg*)malloc(sizeof(AmarkovDynProg)); 
    initMarkov1DynProg(mdpw->amdp, mdpw->m1dp, mis);
    break;
  }
  case MARK2		: {
    mdpw->m2dp = (markov2DynProg*)malloc(sizeof(markov2DynProg));
    mdpw->amdp = (AmarkovDynProg*)malloc(sizeof(AmarkovDynProg));
    initMarkov2DynProg(mdpw->amdp, mdpw->m2dp, mis);
    break;
  }
  }
  return 1;
}
uint64_t getMarkovDynProgRootValue(markovDynProgWrapper* mdpw, markovImprobaStruct* mis, unsigned int requiredLength){
  switch(mdpw->type){
  case NO_MARK: {return pow(ALPHABET_SIZE, requiredLength);}
  case MARK0: {return getMarkov0DynProgRootValue(mdpw->m0dp, mis, requiredLength);}
  case MARK1: {return getMarkov1DynProgRootValue(mdpw->m1dp, mis, requiredLength);}
  case MARK2: {return getMarkov2DynProgRootValue(mdpw->m2dp, mis, requiredLength);}
  }
  return 0;
}

void deleteMarkovDynProgWrapper(markovDynProgWrapper* mdpw){
  if (mdpw != NULL){
    if (mdpw->m0dp != NULL){
      deleteMarkov0DynProg(mdpw->m0dp);
      free(mdpw->m0dp);
    }
    if (mdpw->m1dp != NULL){
      deleteMarkov1DynProg(mdpw->m1dp);
      free(mdpw->m1dp);
    }
    if (mdpw->m2dp != NULL){
      deleteMarkov2DynProg(mdpw->m2dp);
      free(mdpw->m2dp);
    }
    if (mdpw->amdp != NULL){
      deleteAMarkovDynProg(mdpw->amdp);
      free(mdpw->amdp);
    }
  }
}
