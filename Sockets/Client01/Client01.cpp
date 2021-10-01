#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <winsock.h>
#include <stdio.h>

#include <iostream>

#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_PORT "27015"
#define SERVER "localhost"

int main(int argc, char* argv[])
{
    std::cout << "\n\n-----CLIENT OUTPUT-----\n\n" << std::endl;

    /*
        Initializing WinSock.
    */
    WSADATA wsaData;
    int err;

    err = WSAStartup(MAKEWORD(1, 1), &wsaData);
    if (err != 0)
    {
        std::cout << "WSAStartup failed with error: " << err << std::endl;
    }

    std::cout << "Creating socket..." << std::endl;

    /*
        getaddrinfo (function): fills result struct
    */

    struct addrinfo* result = NULL, * ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    err = getaddrinfo(SERVER, DEFAULT_PORT, &hints, &result);
    if (err != 0) {
        std::cout << "getaddrinfo failed: " << err << std::endl;
        WSACleanup();
        return 1;
    }

    /*
        Create a socket, check for errors.
    */

    SOCKET ConnectSocket = INVALID_SOCKET;

    ptr = result;

    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

    if (ConnectSocket == INVALID_SOCKET) {
        std::cout << "Error at socket(): " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    /*
        Binding a socket.
        freeaddrinfo to free memory.

        TODO: Try next address if connection call failed.
    */

    err = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (err == SOCKET_ERROR)
    {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET)
    {
        std::cout << "Unable to connect to server. \n" << err << std::endl;
        WSACleanup();
        return 1;
    }

    /*
        TODO:
        Message Loop.
    */

    std::cout << "Connected?..." << std::endl;

    /*
        Cleanup
    */
    WSACleanup();

    std::cout << "\n\n-----END OF CLIENT OUTPUT-----\n\n" << std::endl;
}