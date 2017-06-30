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

#ifndef ALIASMARKOV_H
#define ALIASMARKOV_H

/*attention: ces structures ne doivent pas etre modifie sans modifier le kernel et les méthodes get UTF*/

#include <stdint.h>

typedef struct aliasMarkov64Element aliasMarkov64Element;
struct aliasMarkov64Element{
	uint64_t space;
	uint64_t proba[26];
	unsigned char alias[26];
	unsigned char align64Padding[6];
};

typedef struct aliasMarkov32Element aliasMarkov32Element;
struct aliasMarkov32Element{
	uint64_t space;
	unsigned int proba[26];
	unsigned char alias[26];
	unsigned char align64Padding[6];
};

#include "markov.h"

uint64_t getMark0BufferSizeRoot(markov0DynProg* m0dp);
uint64_t getMark0BufferSizeFLine(markov0DynProg* m0dp);
uint64_t getMark0BufferSizeMain(markov0DynProg* m0dp);

uint64_t getMark1BufferSizeRoot(markov1DynProg* m1dp);
uint64_t getMark1BufferSizeFLine(markov1DynProg* m1dp);
uint64_t getMark1BufferSizeMain(markov1DynProg* m1dp);

uint64_t getMark2BufferSizeRoot(markov2DynProg* m2dp);
uint64_t getMark2BufferSizeFLine(markov2DynProg* m2dp);
uint64_t getMark2BufferSizeMain(markov2DynProg* m2dp);

void fillAliasMarkov0BufferRoot(markov0DynProg* m0dp, AmarkovDynProg* amdp, markovImprobaStruct* mis, char* bufferRoot);
void fillAliasMarkov0BufferFLine(markov0DynProg* m0dp, AmarkovDynProg* amdp, markovImprobaStruct* mis, char* bufferFLine);
void fillAliasMarkov0BufferMain(markov0DynProg* m0dp, AmarkovDynProg* amdp, markovImprobaStruct* mis, char* bufferMain);

void fillAliasMarkov1BufferRoot(markov1DynProg* m1dp, AmarkovDynProg* amdp, markovImprobaStruct* mis, char* bufferRoot);
void fillAliasMarkov1BufferFLine(markov1DynProg* m1dp, AmarkovDynProg* amdp, markovImprobaStruct* mis, char* bufferFLine);
void fillAliasMarkov1BufferMain(markov1DynProg* m1dp, AmarkovDynProg* amdp, markovImprobaStruct* mis, char* bufferMain);

void fillAliasMarkov2BufferRoot(markov2DynProg* m2dp, AmarkovDynProg* amdp, markovImprobaStruct* mis, char* bufferRoot);
void fillAliasMarkov2BufferFLine(markov2DynProg* m2dp, AmarkovDynProg* amdp, markovImprobaStruct* mis, char* bufferFLine);
void fillAliasMarkov2BufferMain(markov2DynProg* m2dp, AmarkovDynProg* amdp, markovImprobaStruct* mis, char* bufferMain);


#endif
