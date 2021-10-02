#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <iphlpapi.h>

#include <iostream>
#include <string>
#include <WinInet.h>
#include "network/NetworkHandler.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "WinInet.lib")
#pragma comment(lib, "Iphlpapi.lib")

#define DEFAULT_PORT "2522"
#define DATA_BUFSIZE 8192



class ServerManager
{
public:
	ServerManager();
	~ServerManager();

	int Run();

private:
    typedef struct _SOCKET_INFORMATION {

        CHAR Buffer[DATA_BUFSIZE];
        WSABUF DataBuf;
        SOCKET Socket;
        DWORD BytesSEND;
        DWORD BytesRECV;

    } SOCKET_INFORMATION, * LPSOCKET_INFORMATION;

    NetworkHandler dbhandler;

    DWORD EventTotal = 0;
    WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
    LPSOCKET_INFORMATION SocketArray[WSA_MAXIMUM_WAIT_EVENTS];

    SOCKET ListenSocket = NULL;
    SOCKET AcceptSocket;
    SOCKADDR_IN InternetAddr;
    LPSOCKET_INFORMATION SocketInfo;
    DWORD Event;
    WSANETWORKEVENTS NetworkEvents;
    WSADATA wsaData;
    DWORD Flags;
    DWORD RecvBytes;
    DWORD SendBytes;

    
    BOOL CreateSocketInformation(SOCKET s);
    void FreeSocketInformation(DWORD Event);
    void displayAddress(const SOCKET_ADDRESS& Address);
    void ListAdapterInfo();

    int init();
};
