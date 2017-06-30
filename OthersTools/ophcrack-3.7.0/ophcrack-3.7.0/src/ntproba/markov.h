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

#ifndef MARKOV_H
#define MARKOV_H

#include <stdint.h>

#include "aliasMarkovBuffer.h"

#define ALPHABET_SIZE 26
#define MAX_MARKOV_SIZE 12


enum markovType{
  NO_MARK, MARK0, MARK1, MARK2
};
typedef enum markovType markovType;

typedef struct markovImprobaStruct markovImprobaStruct;
struct markovImprobaStruct{
  unsigned int 	improbaFirst[ALPHABET_SIZE];
  unsigned int	improbaMark0[ALPHABET_SIZE];
  unsigned int	improbaMark1[ALPHABET_SIZE][ALPHABET_SIZE];
  unsigned int 	improbaMark2[ALPHABET_SIZE][ALPHABET_SIZE][ALPHABET_SIZE];
  unsigned int	meanImprobaMark0;
};

typedef struct markov0DynProg markov0DynProg;
struct markov0DynProg{
  uint64_t* 			rootValue;
  uint64_t* 			firstLine;
  uint64_t* 			buffer;
  unsigned int 		minImproba;
  unsigned int 		bufferWidth;
  unsigned int 		threshold;
};

typedef struct markov1DynProg markov1DynProg;
struct markov1DynProg{
  uint64_t* 		rootValue;
  uint64_t*		firstLine;
  uint64_t*		buffer[ALPHABET_SIZE];
  unsigned int	minImproba;
  unsigned int	bufferWidth;
  unsigned int 	threshold;
};

typedef struct markov2DynProg markov2DynProg;
struct markov2DynProg{
  uint64_t* 		rootValue;
  uint64_t*		firstLine;
  uint64_t*		buffer[ALPHABET_SIZE][ALPHABET_SIZE];
  unsigned int	minImproba;
  unsigned int	bufferWidth;
  unsigned int 	threshold;
};

typedef struct AmarkovDynProg AmarkovDynProg;
struct AmarkovDynProg{
  unsigned int* 		rootValue;
  unsigned int*		firstLine;
  unsigned int*		buffer;
  unsigned int 		minImproba;
  unsigned int		bufferWidth;
  unsigned int 		threshold;
  aliasMarkovBuffer*	amb;
};

typedef struct markovDynProgWrapper markovDynProgWrapper;
struct markovDynProgWrapper{
  markovType type;
	
  markov0DynProg* m0dp;
  markov1DynProg* m1dp;
  markov2DynProg* m2dp;
	
  AmarkovDynProg* amdp;
};

#include "pattern.h"

void deleteImprobaStruct(markovImprobaStruct* mis);

int initMarkovDynProgWrapper(markovDynProgWrapper* mdpw, markovImprobaStruct* mis, markovType type);
uint64_t getMarkovDynProgRootValue(markovDynProgWrapper* mdpw, markovImprobaStruct* mis, unsigned int requiredLength);
void deleteMarkovDynProgWrapper(markovDynProgWrapper* mdpw);

uint64_t getMarkov0DynProgRootValue(markov0DynProg* m0dp, markovImprobaStruct* mis, unsigned int requiredLength);
uint64_t getMarkov0DynProgFirstLine(markov0DynProg* m0dp, markovImprobaStruct* mis, unsigned int index, unsigned int requiredLength);
uint64_t getMarkov0DynProgSpace(markov0DynProg* m0dp, markovImprobaStruct* mis, unsigned int length, unsigned int level);

uint64_t getMarkov1DynProgRootValue(markov1DynProg* m1dp, markovImprobaStruct* mis, unsigned int requiredLength);
uint64_t getMarkov1DynProgFirstLine(markov1DynProg* m1dp, markovImprobaStruct* mis, unsigned int index, unsigned int requiredLength);
uint64_t getMarkov1DynProgSpace(markov1DynProg* m1dp, markovImprobaStruct* mis, unsigned int length, unsigned int level, unsigned int prevChar);

uint64_t getMarkov2DynProgRootValue(markov2DynProg* m2dp, markovImprobaStruct* mis, unsigned int requiredLength);
uint64_t getMarkov2DynProgFirstLine(markov2DynProg* m2dp, markovImprobaStruct* mis, unsigned int index, unsigned int requiredLength);
uint64_t getMarkov2DynProgSpace(markov2DynProg* m2dp, markovImprobaStruct* mis, unsigned int length, unsigned int level, unsigned int prevChar1, unsigned int prevChar2);

unsigned int getLetterSequenceImprobaMark0(markovImprobaStruct *mis, char* pwd, unsigned int length);
unsigned int getLetterSequenceImprobaMark1(markovImprobaStruct *mis, char* pwd, unsigned int length);
unsigned int getLetterSequenceImprobaMark2(markovImprobaStruct *mis, char* pwd, unsigned int length);

#endif
