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

    /* 
        Initializing Winsock by calling WSAStartup function.
        Returns WSADATA struct.

        typedef struct WSAData {
           WORD     wVersion;                               - Version expected to use.
           WORD     wHighVersion;                           - Highest supported version.
           char     szDescription[WSADESCRIPTION_LEN+1];    - ASCII string (NULL-terminated) description of Windows Socket implementaion. (up to 256 chars).
           char     szSystemStatus[WSASYS_STATUS_LEN+1];    - String containing relevant status or configuration information.
           unsigned short iMaxSockets;                      - Maximum number of sockets that may be opened. (Depricated >= v2.0)
           unsigned short iMaxUdpDg;                        - Maximum datagram message size. (Depricated >= v2.0)
           char FAR *lpVendorInfo;                          - Vendor specific information. (Depricated >= v2.0)
        } WSADATA;
        https://docs.microsoft.com/en-us/windows/win32/api/winsock/ns-winsock-wsadata#requirements

    */
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