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

#ifndef HASHTOPWD_H
#define HASHTOPWD_H

#include <stdint.h>

#include "pattern.h"

#define PWD_UTF_SIZE 32

uint64_t getPatternFirstRound(patternArray* array, uint64_t index, unsigned int* patIndex);
unsigned int selectCharacterUTFFirstRound(markovDynProgWrapper* mdpw, markovImprobaStruct* mis, uint64_t patternDescriptor, uint64_t patternLength, uint64_t subIndex, char* result);
unsigned int selectCharacterUTF(markovDynProgWrapper* mdpw, markovImprobaStruct* mis, uint64_t patternDescriptor, uint64_t patternLength, uint64_t biasedCoin, uint64_t fairDice, char* result);

char checkFoundPwd(char* pwdUTF, unsigned int taille, unsigned int* hash, char* pwd);
uint64_t hashToIndex(patternArray* array, markovDynProgWrapper* mdpw, markovImprobaStruct* mis, unsigned int* hashsrc, const unsigned int startC, const unsigned int endC, const unsigned int threshold, const unsigned int currentTable, const unsigned int chainLength);
unsigned int indexToPwdUTF(patternArray* array, markovDynProgWrapper* mdpw, markovImprobaStruct* mis, const uint64_t start, const unsigned int threshold, const unsigned int endC, const unsigned int currentTable, const unsigned int chainLength, char* pwdUTF);
void hashNTH(char* plainText, unsigned char* hash);

#endif
