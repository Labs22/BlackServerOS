#include "MD4ex.h"

MD4ex::MD4ex()
{
}

MD4ex::~MD4ex()
{
}

int MD4ex::GenerateSignature(vector<unsigned char> key, vector<unsigned char> message, unsigned char ** signature)
{
	*signature = new unsigned char[16];
	MD4_CTX original;
	MD4_Init(&original);
	int totalLen = key.size() + message.size();
	unsigned char * tohash = new unsigned char[totalLen];
	for(unsigned int x = 0; x < key.size(); x++)
	{
		tohash[x] = key[x];
	}
	for(unsigned int x = 0; x < message.size(); x++)
	{
		tohash[x + key.size()] = message[x];
	}
	MD4_Update(&original, tohash, totalLen);
	delete [] tohash;
	MD4_Final(*signature, &original);
	return 1;
}

bool MD4ex::ValidateSignature(vector<unsigned char> key, vector<unsigned char> message, unsigned char * signature)
{
	MD4_CTX original;
	MD4_Init(&original);
	int totalLen = key.size() + message.size();
	unsigned char * tohash = new unsigned char[totalLen];
	for(unsigned int x = 0; x < key.size(); x++)
	{
		tohash[x] = key[x];
	}
	for(unsigned int x = 0; x < message.size(); x++)
	{
		tohash[x + key.size()] = message[x];
	}
	MD4_Update(&original, tohash, totalLen);
	delete [] tohash;
	unsigned char hash[16];
	MD4_Final(hash, &original);
	if(memcmp(hash, signature, 16) == 0)
	{
		return true;
	}
	return false;
}

vector<unsigned char> * MD4ex::GenerateStretchedData(vector<unsigned char> originalMessage, int keylength, unsigned char * hash, vector<unsigned char> added, unsigned char ** newSig)
{
	vector<unsigned char> * ret = new vector<unsigned char>();
	for(unsigned int x = 0; x < originalMessage.size(); x++)
		ret->push_back(originalMessage[x]);
	int tailLength = ret->size() + keylength;
	tailLength *= 8;
	ret->push_back(0x80);
	while((ret->size() + keylength) % 64 != 56)
	{
		ret->push_back(0x00);
	}

	ret->push_back((tailLength) & 0xFF);
	ret->push_back((tailLength >> 8) & 0xFF);
	ret->push_back((tailLength >> 16) & 0xFF);
	ret->push_back((tailLength >> 24) & 0xFF);
	ret->push_back(0x00);
	ret->push_back(0x00);
	ret->push_back(0x00);
	ret->push_back(0x00);

	MD4_CTX stretch;
	MD4_Init(&stretch);
	stretch.Nl = (ret->size() + keylength) * 8;
	stretch.A = hash[0] | (hash[1] << 8) | (hash[2] << 16) | (hash[3] << 24);
	stretch.B = hash[4] | (hash[5] << 8) | (hash[6] << 16) | (hash[7] << 24);
	stretch.C = hash[8] | (hash[9] << 8) | (hash[10] << 16) | (hash[11] << 24);
	stretch.D = hash[12] | (hash[13] << 8) | (hash[14] << 16) | (hash[15] << 24);
	char * toadd = new char[added.size()];
	for(unsigned int x = 0; x < added.size(); x++)
	{
		toadd[x] = added[x];
	}
	MD4_Update(&stretch, toadd, added.size());
	*newSig = new unsigned char[16];
	MD4_Final(*newSig, &stretch);
	delete [] toadd;
	for(unsigned int x = 0; x < added.size(); x++)
	{
		ret->push_back(added.at(x));
	}
	return ret;
}
