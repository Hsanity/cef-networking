#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <winsock.h>
#include <stdio.h>
#include <windows.h>

#include <iostream>
#include <string>
#include <WinInet.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "WinInet.lib")

#define DEFAULT_PORT "27015"

std::string real_ip()
{
    LPCWSTR word = L"IP retriever";
    LPCWSTR word2 = L"http://myexternalip.com/raw";

    HINTERNET net = InternetOpen(word,
        INTERNET_OPEN_TYPE_PRECONFIG,
        NULL,
        NULL,
        0);

    HINTERNET conn = InternetOpenUrl(net,
        word2,
        NULL,
        0,
        INTERNET_FLAG_RELOAD,
        0);

    char buffer[4096];
    DWORD read;

    InternetReadFile(conn, buffer, sizeof(buffer) / sizeof(buffer[0]), &read);
    InternetCloseHandle(net);

    return std::string(buffer, read);
}

int main(int argc, char* argv[])
{
	std::cout << "\n\n-----SERVER OUTPUT-----\n\n" << std::endl;

    /*
        Initializing WinSock.
    */
    WSADATA wsaData;
    int err;

    err = WSAStartup(MAKEWORD(2, 2), &wsaData);
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

    unsigned long ul = 1;

    //int nRet = ioctlsocket(ListenSocket, FIONBIO, (unsigned long*)&ul);

    //if (nRet == SOCKET_ERROR)
    //{
        // Failed to put the socket into non-blocking mode

       // std::cout << "Error at ioctlsocket(): " << WSAGetLastError() << std::endl;
       // freeaddrinfo(result);
        //WSACleanup();
        //return 1;
    //}

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

    /*
        Listen function: Takes socket and backlog.
        Backlog: max length of queue - of pending connections to accept.
        SOMAXCONN - Max number of pending connections in queue.
    */
    std::cout << "\n\n-----IP ADDRESS-----\n\n" << std::endl;
    std::cout << real_ip() << std::endl;

    std::cout << "\n" << std::endl;
    std::cout << "Listening for connections..." << std::endl;

    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cout << "Listen failed, error: " << WSAGetLastError() << std::endl;
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "\n\n-----BEGIN SERVER LOOP-----\n\n" << std::endl;
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
    
    std::cout << "\nConnected Client..\n" << std::endl;
    /*
        Message Loop.
    */
    fd_set  fdread;
    int     ret;

    // Manage I/O on the socket
    while (TRUE)
    {
        // Always clear the read set before calling select()
        FD_ZERO(&fdread);

        // Add socket s to the read set
        FD_SET(ClientSocket, &fdread);

        if ((ret = select(0, &fdread, NULL, NULL, NULL)) == SOCKET_ERROR)
        {
            // Error condition
        }

        if (ret > 0)
        {
            // For this simple case, select() should return
            // the value 1. An application dealing with
            // more than one socket could get a value
            // greater than 1. At this point, your
            // application should check to see whether the socket is part of a set.
            if (FD_ISSET(ClientSocket, &fdread))
            {
                // A read event has occurred on socket s
            }
        }
    }

    /*
        Cleanup
    */
    WSACleanup();

	std::cout << "\n\n-----END OF SERVER OUTPUT-----\n\n" << std::endl;
}

