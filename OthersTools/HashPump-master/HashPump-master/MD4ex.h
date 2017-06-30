#ifndef MD4EX_H_
#define MD4EX_H_

#include "Extender.h"
#include <openssl/md4.h>

class MD4ex: public Extender
{
public:
	MD4ex();
	virtual ~MD4ex();
	int GenerateSignature(vector<unsigned char> key, vector<unsigned char> message, unsigned char ** signature);
	bool ValidateSignature(vector<unsigned char> key, vector<unsigned char> message, unsigned char * signature);
	vector<unsigned char> * GenerateStretchedData(vector<unsigned char> originalMessage, int keylength, unsigned char * hash, vector<unsigned char> added, unsigned char ** newSig);
};

#endif
