#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>

#define PORT 11427

void parse_data    (char * data);
void demangle_data (char *recvbuf);

int main(int argc, char * argv[])
{
    WSADATA wsaData;
    int ret = 0;
    SOCKET s;
    sockaddr_in server;
    int bytesRecv = SOCKET_ERROR;
    SOCKET aSocket;
    char recvbuf[1024];
    
   ret = WSAStartup(MAKEWORD(2,2), &wsaData);

   if (ret != NO_ERROR)
   {
        printf("error: WSAStartup()\n");
        return 0;
   }
   
   s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   
   if (s == INVALID_SOCKET) 
   {
        printf("error: socket(0x%08x)\n", WSAGetLastError());
        WSACleanup();
        return 0;
   }
   
   server.sin_family = AF_INET;
   server.sin_addr.s_addr = inet_addr("0.0.0.0");
   server.sin_port = htons(PORT);
   
   if (bind(s, (SOCKADDR*) &server, sizeof(server)) == SOCKET_ERROR) 
   {
        printf("error: bind(0x%08x)\n", WSAGetLastError());
        closesocket(s);
        return 0;
    }
    
    // listen on the socket.
    if (listen(s, 1) == SOCKET_ERROR)
    {
        printf("error: listen(0x%08x)\n", WSAGetLastError());
        return 0;
    }

    printf("Listening and waiting for client to connect... ");
   
    while (1)
    {
        aSocket = SOCKET_ERROR;

        while (aSocket == SOCKET_ERROR)
            aSocket = accept(s, NULL, NULL);

        printf("client connected.\n");
        s = aSocket; 
        break;
    }
    
    memset(recvbuf, 0, sizeof(recvbuf));
    
    ret = recv(s, recvbuf, 1024, 0);
    
    printf("received %ld bytes.\n", ret);
    
    demangle_data(recvbuf);
        
    parse_data(recvbuf);

    printf("exiting...\n\n");

    return 0;
}


void demangle_data (char *recvbuf)
{
    unsigned int i = 0;
    unsigned int length;

    //printf("demangling: %s)\n", recvbuf);

    length = strlen(recvbuf);

    for(i = 0; i < length; i++)
    {
        recvbuf[i] = recvbuf[i] ^ 0x01;   
    }

    return;
}


void parse_data (char *data)
{
    char cmd[64];
    char parameter[64];
    unsigned int i = 0;
    unsigned int t = 0;
    unsigned int length;
   
    printf("parsing: %s\n", data);

    memset(cmd,       0, sizeof(cmd));
    memset(parameter, 0, sizeof(parameter));

	// loop through recv'd data looking for the character ':', which indicates the command.
    length = strlen(data);

    for(i = 0; i <= length; i++)
    {
        if(data[i] == ':')
            break;
    }

    // only parse the data further if the character ':' was found.
    if (i < length)
    {
        strncpy(parameter, data+i+1, 16);
        strncpy(cmd,       data,     i);

        printf("\n");
        printf("PARSED CMD:       %s\n", cmd);
        printf("PARSED PARAMETER: %s\n", parameter);
    }
}