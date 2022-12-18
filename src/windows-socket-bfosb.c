#include <winsock2.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

#define MAX_CLIENTS 30
#define PORT 7777
#define BUFFER_SIZE 1536
#define OLD_BUFFER_SIZE 512

struct server_fd
{
	SOCKET sock;
    SOCKADDR_IN sin;
}


static void create_serverfd(void)
{
    
}
int __cdecl main(int argc, char *argv[])
{
    
}
