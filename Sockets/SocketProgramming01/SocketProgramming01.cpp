#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <winsock.h>
#include <stdio.h>

#include <iostream>

#pragma comment(lib, "ws2_32.lib")

int main()
{
    std::cout << "-----CLIENT OUTPUT-----\n" << std::endl;

    WSADATA wsaData;
    int err;

    err = WSAStartup(MAKEWORD(1,1), &wsaData);
    if (err != 0)
    {
        std::cout << "WSAStartup failed with error: " << err << std::endl;
    }

    /*
        Create a socket.
        SOCK_STREAM or SOCK_DGRAM (TCP or UDP)
        IPPROTO_TCP or IPPROTO_UDP
    */

    std::cout << "Creating socket..." << std::endl;

    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (s == INVALID_SOCKET)
    {
        int iSocketError = WSAGetLastError();
        std::cout << "Socket error: " << iSocketError << std::endl;
        return FALSE;
    }

    std::cout << "Establishing connection..." << std::endl;

    closesocket(s);
    WSACleanup();
    std::cout << "\n\n-----END OF CLIENT OUTPUT-----\n\n" << std::endl;
}