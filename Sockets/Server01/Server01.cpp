#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <winsock.h>
#include <stdio.h>

#include <iostream>

#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_PORT "27015"

int main(int argc, char* argv[])
{
	std::cout << "\n\n-----SERVER OUTPUT-----\n\n" << std::endl;

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
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    err = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (err != 0) {
        std::cout << "getaddrinfo failed: " << err << std::endl;
        WSACleanup();
        return 1;
    }

    /*
        Create a socket, check for errors.
    */

    SOCKET ListenSocket = INVALID_SOCKET;

    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (ListenSocket == INVALID_SOCKET) {
        std::cout << "Error at socket(): " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
    
    /*
        Binding a socket.
        freeaddrinfo to free memory.
    */

    std::cout << "Binding socket..." << std::endl;

    err = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (err == SOCKET_ERROR) {
        std::cout << "bind failed with error: " << WSAGetLastError();
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    char* str = new char[256];
    gethostname(str, 256);

    //std::cout << "Address: " << str << std::endl;

    /*
        Listen function: Takes socket and backlog.
        Backlog: max length of queue - of pending connections to accept.
        SOMAXCONN - Max number of pending connections in queue.
    */

    std::cout << "Listening for connections..." << std::endl;

    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cout << "Listen failed, error: " << WSAGetLastError() << std::endl;
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    /*
        Accept Client connection.
    */

    SOCKET ClientSocket = INVALID_SOCKET;

    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET)
    {
        std::cout << "Accept failed: " << WSAGetLastError() << std::endl;
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    std::cout << "Accepted..." << std::endl;
    /*
        TODO:
        Message Loop.
    */



    /*
        Cleanup
    */
    WSACleanup();

	std::cout << "\n\n-----END OF SERVER OUTPUT-----\n\n" << std::endl;
}