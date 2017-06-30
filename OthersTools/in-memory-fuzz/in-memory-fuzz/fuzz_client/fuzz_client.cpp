#include <stdio.h>
#include "winsock2.h"

#define PORT 11427

void mangle_data (char * sendbuf);

int main (int argc, char * argv[])
{
	char sendbuf[1024];
    WSADATA wsaData;
	SOCKET s;
	sockaddr_in client;

	if(argc <= 2)
	{
		printf("USAGE: fuzz_client [server] [data to send]\n");
	}

    int ret = WSAStartup(MAKEWORD(2,2), &wsaData);

    if (ret != NO_ERROR)
	{
        printf("Error: WSAStartup()\n");
		return 0;
	}

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (s == INVALID_SOCKET)
    {
        printf("Error: socket(0x%08x)\n", WSAGetLastError());
        WSACleanup();
        return 0;
    }

    // connect to a server.
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr(argv[1]);
    client.sin_port = htons(PORT);
	
    printf("connecting....\n");
    
    if (connect(s, (SOCKADDR*)&client, sizeof(client))== SOCKET_ERROR)
    {
        printf("Failed to connect.\n");
        WSACleanup();
        return 0;
    }

    strncpy(sendbuf, argv[2], 1023);
	mangle_data(sendbuf);

    printf("sending...\n");
    send(s, sendbuf, strlen(sendbuf), 0);
	
    printf("sent...\n");
    closesocket(s);
    return 0;
}


void mangle_data (char * sendbuf)
{
	unsigned int i = 0;

	// don't xor the null character
    for(i = 0; i <= strlen(sendbuf)-1; i++)
    {
        sendbuf[i] = sendbuf[i] ^ 0x01;        
    }
}