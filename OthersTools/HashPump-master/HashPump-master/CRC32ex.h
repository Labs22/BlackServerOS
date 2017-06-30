#ifndef CRC32EX_H_
#define CRC32EX_H_

#include "Extender.h"
#include <stdint.h>

class CRC32ex: public Extender
{
public:
	CRC32ex();
	virtual ~CRC32ex();
	int GenerateSignature(vector<unsigned char> key, vector<unsigned char> message, unsigned char ** signature);
	bool ValidateSignature(vector<unsigned char> key, vector<unsigned char> message, unsigned char * signature);
	static uint32_t Continue_CRC32(unsigned int state, vector<unsigned char> message);
	vector<unsigned char> * GenerateStretchedData(vector<unsigned char> originalMessage, int keylength, unsigned char * hash, vector<unsigned char> added, unsigned char ** newSig);
	static uint32_t crc32_lookup[];
};

#endif
