#ifndef _SHA1_H_
#define _SHA1_H_

#include "Extender.h"
#include <openssl/sha.h>

class SHA1ex : public Extender
{
public:
	SHA1ex();
	int GenerateSignature(vector<unsigned char> key, vector<unsigned char> message, unsigned char ** signature);
	bool ValidateSignature(vector<unsigned char> key, vector<unsigned char> message, unsigned char * signature);
	vector<unsigned char> * GenerateStretchedData(vector<unsigned char> originalMessage, int keylength, unsigned char * hash, vector<unsigned char> added, unsigned char ** newSig);
};

#endif
