#include "SHA1.h"

SHA1ex::SHA1ex()
{

}

int SHA1ex::GenerateSignature(vector<unsigned char> key, vector<unsigned char> message, unsigned char ** signature)
{
	*signature = new unsigned char[20];
	SHA_CTX original;
	SHA1_Init(&original);
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
	SHA1_Update(&original, tohash, totalLen);
	delete [] tohash;
	SHA1_Final(*signature, &original);
	return 1;
}

bool SHA1ex::ValidateSignature(vector<unsigned char> key, vector<unsigned char> message, unsigned char * signature)
{
	SHA_CTX original;
	SHA1_Init(&original);
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
	SHA1_Update(&original, tohash, totalLen);
	delete [] tohash;
	unsigned char hash[20];
	SHA1_Final(hash, &original);
	if(memcmp(hash, signature, 20) == 0)
	{
		return true;
	}
	return false;
}

vector<unsigned char> * SHA1ex::GenerateStretchedData(vector<unsigned char> originalMessage, int keylength, unsigned char * hash, vector<unsigned char> added, unsigned char ** newSig)
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
	SHA_CTX stretch;
	SHA1_Init(&stretch);
	stretch.Nl = (ret->size() + keylength) * 8;
	stretch.h0 = hash[3] | (hash[2] << 8) | (hash[1] << 16) | (hash[0] << 24);
	stretch.h1 = hash[7] | (hash[6] << 8) | (hash[5] << 16) | (hash[4] << 24);
	stretch.h2 = hash[11] | (hash[10] << 8) | (hash[9] << 16) | (hash[8] << 24);
	stretch.h3 = hash[15] | (hash[14] << 8) | (hash[13] << 16) | (hash[12] << 24);
	stretch.h4 = hash[19] | (hash[18] << 8) | (hash[17] << 16) | (hash[16] << 24);
	char * toadd = new char[added.size()];
	for(unsigned int x = 0; x < added.size(); x++)
	{
		toadd[x] = added[x];
	}
	SHA1_Update(&stretch, toadd, added.size());
	*newSig = new unsigned char[20];
	SHA1_Final(*newSig, &stretch);
	delete [] toadd;
	for(unsigned int x = 0; x < added.size(); x++)
	{
		ret->push_back(added.at(x));
	}
	return ret;
}
