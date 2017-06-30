#include "SHA256.h"

SHA256ex::SHA256ex()
{

}

SHA256ex::~SHA256ex()
{

}

int SHA256ex::GenerateSignature(vector<unsigned char> key, vector<unsigned char> message, unsigned char ** signature)
{
	*signature = new unsigned char[32];
	SHA256_CTX original;
	SHA256_Init(&original);
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
	SHA256_Update(&original, tohash, totalLen);
	delete [] tohash;
	SHA256_Final(*signature, &original);
	return 1;
}

bool SHA256ex::ValidateSignature(vector<unsigned char> key, vector<unsigned char> message, unsigned char * signature)
{
	SHA256_CTX original;
	SHA256_Init(&original);
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
	SHA256_Update(&original, tohash, totalLen);
	delete [] tohash;
	unsigned char hash[32];
	SHA256_Final(hash, &original);
	if(memcmp(hash, signature, 32) == 0)
	{
		return true;
	}
	return false;
}

vector<unsigned char> * SHA256ex::GenerateStretchedData(vector<unsigned char> originalMessage, int keylength, unsigned char * hash, vector<unsigned char> added, unsigned char ** newSig)
{
	vector<unsigned char> * ret = new vector<unsigned char>();
	for(unsigned int x = 0; x < originalMessage.size(); x++)
		ret->push_back(originalMessage[x]);
	int tailLength = ret->size() + keylength;
	tailLength *= 8;
	ret->push_back(0x80);
	while((ret->size() + keylength + 8) % 64 != 0)
	{
		ret->push_back(0x00);
	}
	for (int i = 0; i < 4; ++i)
		ret->push_back(0x00);
	ret->push_back((tailLength >> 24) & 0xFF);
	ret->push_back((tailLength >> 16) & 0xFF);
	ret->push_back((tailLength >> 8) & 0xFF);
	ret->push_back((tailLength) & 0xFF);
	SHA256_CTX stretch;
	SHA256_Init(&stretch);
	stretch.Nl = (ret->size() + keylength) * 8;
	stretch.h[0] = hash[3] | (hash[2] << 8) | (hash[1] << 16) | (hash[0] << 24);
	stretch.h[1] = hash[7] | (hash[6] << 8) | (hash[5] << 16) | (hash[4] << 24);
	stretch.h[2] = hash[11] | (hash[10] << 8) | (hash[9] << 16) | (hash[8] << 24);
	stretch.h[3] = hash[15] | (hash[14] << 8) | (hash[13] << 16) | (hash[12] << 24);
	stretch.h[4] = hash[19] | (hash[18] << 8) | (hash[17] << 16) | (hash[16] << 24);
	stretch.h[5] = hash[23] | (hash[22] << 8) | (hash[21] << 16) | (hash[20] << 24);
	stretch.h[6] = hash[27] | (hash[26] << 8) | (hash[25] << 16) | (hash[24] << 24);
	stretch.h[7] = hash[31] | (hash[30] << 8) | (hash[29] << 16) | (hash[28] << 24);
	char * toadd = new char[added.size()];
	for(unsigned int x = 0; x < added.size(); x++)
	{
		toadd[x] = added[x];
	}
	SHA256_Update(&stretch, toadd, added.size());
	*newSig = new unsigned char[32];
	SHA256_Final(*newSig, &stretch);
	delete [] toadd;
	for(unsigned int x = 0; x < added.size(); x++)
	{
		ret->push_back(added.at(x));
	}
	return ret;
}

