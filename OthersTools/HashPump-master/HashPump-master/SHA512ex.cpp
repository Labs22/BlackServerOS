#include "SHA512ex.h"

SHA512ex::SHA512ex()
{

}

SHA512ex::~SHA512ex()
{
}

int SHA512ex::GenerateSignature(vector<unsigned char> key, vector<unsigned char> message, unsigned char ** signature)
{
	*signature = new unsigned char[64];
	SHA512_CTX original;
	SHA512_Init(&original);
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
	SHA512_Update(&original, tohash, totalLen);
	delete [] tohash;
	SHA512_Final(*signature, &original);
	return 1;
}

bool SHA512ex::ValidateSignature(vector<unsigned char> key, vector<unsigned char> message, unsigned char * signature)
{
	SHA512_CTX original;
	SHA512_Init(&original);
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
	SHA512_Update(&original, tohash, totalLen);
	delete [] tohash;
	unsigned char hash[64];
	SHA512_Final(hash, &original);
	if(memcmp(hash, signature, 64) == 0)
	{
		return true;
	}
	return false;
}

vector<unsigned char> * SHA512ex::GenerateStretchedData(vector<unsigned char> originalMessage, int keylength, unsigned char * hash, vector<unsigned char> added, unsigned char ** newSig)
{
	vector<unsigned char> * ret = new vector<unsigned char>();
	for(unsigned int x = 0; x < originalMessage.size(); x++)
		ret->push_back(originalMessage[x]);
	int tailLength = ret->size() + keylength;
	tailLength *= 8;
	ret->push_back(0x80);
	while((ret->size() + keylength + 16) % 128 != 0)
	{
		ret->push_back(0x00);
	}
	for (int i = 0; i < 12; ++i)
		ret->push_back(0x00);
	ret->push_back((tailLength >> 24) & 0xFF);
	ret->push_back((tailLength >> 16) & 0xFF);
	ret->push_back((tailLength >> 8) & 0xFF);
	ret->push_back((tailLength) & 0xFF);
	SHA512_CTX stretch;
	SHA512_Init(&stretch);
	stretch.Nl = (ret->size() + keylength) * 8;
	for(int x = 0; x < 8; x++)
	{
		unsigned char * ptr = (unsigned char *)&(stretch.h[x]);
		*(ptr) = hash[(x * 8) + 7];
		*(ptr + 1) = hash[(x * 8) + 6];
		*(ptr + 2) = hash[(x * 8) + 5];
		*(ptr + 3) = hash[(x * 8) + 4];
		*(ptr + 4) = hash[(x * 8) + 3];
		*(ptr + 5) = hash[(x * 8) + 2];
		*(ptr + 6) = hash[(x * 8) + 1];
		*(ptr + 7) = hash[(x * 8) + 0];
	}

	char * toadd = new char[added.size()];
	for(unsigned int x = 0; x < added.size(); x++)
	{
		toadd[x] = added[x];
	}
	SHA512_Update(&stretch, toadd, added.size());
	*newSig = new unsigned char[64];
	SHA512_Final(*newSig, &stretch);
	delete [] toadd;
	for(unsigned int x = 0; x < added.size(); x++)
	{
		ret->push_back(added.at(x));
	}
	return ret;
}
