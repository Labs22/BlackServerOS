#ifndef MD5EX_H_
#define MD5EX_H_

#include "Extender.h"
#include <openssl/md5.h>

class MD5ex : public Extender
{
public:
	MD5ex();
	int GenerateSignature(vector<unsigned char> key, vector<unsigned char> message, unsigned char ** signature);
	bool ValidateSignature(vector<unsigned char> key, vector<unsigned char> message, unsigned char * signature);
	vector<unsigned char> * GenerateStretchedData(vector<unsigned char> originalMessage, int keylength, unsigned char * hash, vector<unsigned char> added, unsigned char ** newSig);
};

#endif
