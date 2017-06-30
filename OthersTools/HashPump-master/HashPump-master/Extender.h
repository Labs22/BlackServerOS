/*
 * Extender.h
 *
 *  Created on: Aug 29, 2012
 *      Author: bwall
 */

#ifndef EXTENDER_H_
#define EXTENDER_H_

#include <algorithm>
#include <string.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <vector>


using namespace std;

class Extender
{
public:
	Extender();
	virtual ~Extender();
	virtual int GenerateSignature(vector<unsigned char> key, vector<unsigned char> message, unsigned char ** signature) = 0;
	virtual bool ValidateSignature(vector<unsigned char> key, vector<unsigned char> message, unsigned char * signature) = 0;
	virtual vector<unsigned char> * GenerateStretchedData(vector<unsigned char> originalMessage, int keylength, unsigned char * hash, vector<unsigned char> added, unsigned char ** newSig) = 0;
};

#endif /* EXTENDER_H_ */
