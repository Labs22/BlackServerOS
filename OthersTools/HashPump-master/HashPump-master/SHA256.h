#ifndef SHA256_H_
#define SHA256_H_

#include "Extender.h"
#include <openssl/sha.h>

class SHA256ex: public Extender
{
public:
	SHA256ex();
	virtual ~SHA256ex();
	int GenerateSignature(vector<unsigned char> key, vector<unsigned char> message, unsigned char ** signature);
	bool ValidateSignature(vector<unsigned char> key, vector<unsigned char> message, unsigned char * signature);
	vector<unsigned char> * GenerateStretchedData(vector<unsigned char> originalMessage, int keylength, unsigned char * hash, vector<unsigned char> added, unsigned char ** newSig);
};

#endif
