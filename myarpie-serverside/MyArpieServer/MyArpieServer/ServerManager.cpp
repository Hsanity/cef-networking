#include "ServerManager.h"


ServerManager::ServerManager()
{
    dbhandler = NetworkHandler();
}

ServerManager::~ServerManager()
{

}

int ServerManager::init()
{
    std::cout << "\n\n-----SERVER OUTPUT-----\n\n" << std::endl;

    /*
        Initializing WinSock.
    */
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

    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (ListenSocket == INVALID_SOCKET) {
        std::cout << "Error at socket(): " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    unsigned long ul = 1;

    int nRet = ioctlsocket(ListenSocket, FIONBIO, (unsigned long*)&ul);

    if (nRet == SOCKET_ERROR)
    {
        // Failed to put the socket into non-blocking mode

        std::cout << "Error at ioctlsocket(): " << WSAGetLastError() << std::endl;
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

    if (CreateSocketInformation(ListenSocket) == FALSE)
        printf("CreateSocketInformation() failed!\n");

    if (WSAEventSelect(ListenSocket, EventArray[EventTotal - 1], FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR)
    {
        printf("WSAEventSelect() failed with error %d\n", WSAGetLastError());
        return 1;
    }


    ListAdapterInfo();

    std::cout << "\n\n" << std::endl;
    std::cout << "Listening for connections..." << std::endl;

    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cout << "Listen failed, error: " << WSAGetLastError() << std::endl;
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
}

int ServerManager::Run()
{
    if(init() == 1)
    {
        return 1;
    }


    std::cout << "\n\n-----BEGIN SERVER LOOP-----\n\n" << std::endl;

    while (true)
    {
        // Wait for one of the sockets to receive I/O notification and
        if ((Event = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE, WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
        {
            printf("WSAWaitForMultipleEvents() failed with error %d\n", WSAGetLastError());
            return 1;
        }

        if (WSAEnumNetworkEvents(SocketArray[Event - WSA_WAIT_EVENT_0]->Socket,
            EventArray[Event - WSA_WAIT_EVENT_0], &NetworkEvents) == SOCKET_ERROR)
        {
            printf("WSAEnumNetworkEvents() failed with error %d\n", WSAGetLastError());
            return 1;
        }

        if (NetworkEvents.lNetworkEvents & FD_ACCEPT)
        {
            if (NetworkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
            {
                printf("FD_ACCEPT failed with error %d\n", NetworkEvents.iErrorCode[FD_ACCEPT_BIT]);
                break;
            }

            if ((AcceptSocket = accept(SocketArray[Event - WSA_WAIT_EVENT_0]->Socket, NULL, NULL)) == INVALID_SOCKET)
            {
                printf("accept() failed with error %d\n", WSAGetLastError());
                break;
            }

            if (EventTotal > WSA_MAXIMUM_WAIT_EVENTS)
            {
                printf("Too many connections - closing socket...\n");
                closesocket(AcceptSocket);
                break;
            }

            CreateSocketInformation(AcceptSocket);

            if (WSAEventSelect(AcceptSocket, EventArray[EventTotal - 1], FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
            {
                printf("WSAEventSelect() failed with error %d\n", WSAGetLastError());
                return 1;
            }

            printf("Socket %d got connected...\n", AcceptSocket);
        }

        // Try to read and write data to and from the data buffer if read and write events occur
        if (NetworkEvents.lNetworkEvents & FD_READ || NetworkEvents.lNetworkEvents & FD_WRITE)
        {
            if (NetworkEvents.lNetworkEvents & FD_READ && NetworkEvents.iErrorCode[FD_READ_BIT] != 0)
            {
                printf("FD_READ failed with error %d\n", NetworkEvents.iErrorCode[FD_READ_BIT]);
                break;
            }

            if (NetworkEvents.lNetworkEvents & FD_WRITE && NetworkEvents.iErrorCode[FD_WRITE_BIT] != 0)
            {
                printf("FD_WRITE failed with error %d\n", NetworkEvents.iErrorCode[FD_WRITE_BIT]);
                break;
            }

            SocketInfo = SocketArray[Event - WSA_WAIT_EVENT_0];

            // Read data only if the receive buffer is empty
            if (SocketInfo->BytesRECV == 0)
            {
                SocketInfo->DataBuf.buf = SocketInfo->Buffer;
                SocketInfo->DataBuf.len = DATA_BUFSIZE;

                Flags = 0;

                if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags, NULL, NULL) == SOCKET_ERROR)
                {
                    if (WSAGetLastError() != WSAEWOULDBLOCK)
                    {
                        printf("WSARecv() failed with error %d\n", WSAGetLastError());
                        FreeSocketInformation(Event - WSA_WAIT_EVENT_0);
                        return 1;
                    }
                }
                else
                {
                    SocketInfo->BytesRECV = RecvBytes;
                    char* data = SocketInfo->DataBuf.buf;
                    std::cout << data << std::endl;
                    
                }
            }

            // Write buffer data if it is available
            if (SocketInfo->BytesRECV > SocketInfo->BytesSEND)
            {
                SocketInfo->DataBuf.buf = SocketInfo->Buffer + SocketInfo->BytesSEND;
                SocketInfo->DataBuf.len = SocketInfo->BytesRECV - SocketInfo->BytesSEND;

                if (WSASend(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &SendBytes, 0, NULL, NULL) == SOCKET_ERROR)
                {
                    if (WSAGetLastError() != WSAEWOULDBLOCK)
                    {
                        printf("WSASend() failed with error %d\n", WSAGetLastError());
                        FreeSocketInformation(Event - WSA_WAIT_EVENT_0);
                        return 1;
                    }

                    // A WSAEWOULDBLOCK error has occurred. An FD_WRITE event will be posted
                    // when more buffer space becomes available
                }
                else
                {
                    SocketInfo->BytesSEND += SendBytes;

                    if (SocketInfo->BytesSEND == SocketInfo->BytesRECV)
                    {
                        SocketInfo->BytesSEND = 0;
                        SocketInfo->BytesRECV = 0;
                    }
                }
            }
        }

        if (NetworkEvents.lNetworkEvents & FD_CLOSE)
        {
            if (NetworkEvents.iErrorCode[FD_CLOSE_BIT] != 0)
            {
                printf("FD_CLOSE failed with error %d\n", NetworkEvents.iErrorCode[FD_CLOSE_BIT]);
                break;
            }

            printf("Closing socket information %d\n", SocketArray[Event - WSA_WAIT_EVENT_0]->Socket);
            FreeSocketInformation(Event - WSA_WAIT_EVENT_0);
        }
    }

    /*
        Cleanup
    */
    WSACleanup();

    std::cout << "\n\n-----END OF SERVER OUTPUT-----\n\n" << std::endl;
}

BOOL ServerManager::CreateSocketInformation(SOCKET s)
{
    LPSOCKET_INFORMATION SI;

    if ((EventArray[EventTotal] = WSACreateEvent()) == WSA_INVALID_EVENT)
    {
        printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());

        return FALSE;
    }

    if ((SI = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL)
    {
        printf("GlobalAlloc() failed with error %d\n", GetLastError());

        return FALSE;
    }
    else
        printf("GlobalAlloc() for LPSOCKET_INFORMATION is OK!\n");

    // Prepare SocketInfo structure for use
    SI->Socket = s;
    SI->BytesSEND = 0;
    SI->BytesRECV = 0;

    SocketArray[EventTotal] = SI;
    EventTotal++;

    return(TRUE);
}

void ServerManager::FreeSocketInformation(DWORD Event)
{
    LPSOCKET_INFORMATION SI = SocketArray[Event];
    DWORD i;

    closesocket(SI->Socket);
    GlobalFree(SI);

    if (WSACloseEvent(EventArray[Event]) == TRUE)
        printf("WSACloseEvent() is OK!\n\n");
    else
        printf("WSACloseEvent() failed miserably!\n\n");

    // Squash the socket and event arrays
    for (i = Event; i < EventTotal; i++)
    {
        EventArray[i] = EventArray[i + 1];
        SocketArray[i] = SocketArray[i + 1];
    }

    EventTotal--;
}

void ServerManager::displayAddress(const SOCKET_ADDRESS& Address)
{
    std::cout << "\n  Length of sockaddr: " << Address.iSockaddrLength;
    if (Address.lpSockaddr->sa_family == AF_INET)
    {
        sockaddr_in* si = (sockaddr_in*)(Address.lpSockaddr);
        char a[INET_ADDRSTRLEN] = {};
        if (inet_ntop(AF_INET, &(si->sin_addr), a, sizeof(a)))
            std::cout << "\n   IPv4 address: " << a;
    }
    else if (Address.lpSockaddr->sa_family == AF_INET6)
    {
        sockaddr_in6* si = (sockaddr_in6*)(Address.lpSockaddr);
        char a[INET6_ADDRSTRLEN] = {};
        if (inet_ntop(AF_INET6, &(si->sin6_addr), a, sizeof(a)))
            std::cout << "\n   IPv6 address: " << a;
    }
}

void ServerManager::ListAdapterInfo()
{
    std::cout << "\nUsing GetAdaptersAddresses";

    ULONG size = 1024 * 15;
    PIP_ADAPTER_ADDRESSES p = (IP_ADAPTER_ADDRESSES*)HeapAlloc(GetProcessHeap(), 0, size);
    if (!p)
    {
        std::cout << "\nCannot allocate memory";
        std::cin.get();
        return;
    }

    ULONG ret;
    do
    {
        ret = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, p, &size);
        if (ret != ERROR_BUFFER_OVERFLOW)
            break;

        PIP_ADAPTER_ADDRESSES newp = (IP_ADAPTER_ADDRESSES*)HeapReAlloc(GetProcessHeap(), 0, p, size);
        if (!newp)
        {
            std::cout << "\nCannot reallocate memory";
            HeapFree(GetProcessHeap(), 0, p);
            std::cin.get();
            return;
        }

        p = newp;
    } while (true);

    if (ret != NO_ERROR)
    {
        std::cout << "\nSomething went wrong. Error: " << ret;
        HeapFree(GetProcessHeap(), 0, p);
        std::cin.get();
        return;
    }

    int i = 0;
    for (PIP_ADAPTER_ADDRESSES tp = p; tp != NULL; tp = tp->Next)
    {
        ++i;
        std::cout << "\nLength of IP Adapter info: " << tp->Length;
        std::cout << "\n IPv4 IfIndex: " << tp->IfIndex;
        std::cout << "\n Adapter name: " << tp->AdapterName;
        std::cout << "\n Unicast addresses:";
        int j = 0;
        for (PIP_ADAPTER_UNICAST_ADDRESS pu = tp->FirstUnicastAddress; pu != NULL; pu = pu->Next)
        {
            ++j;
            displayAddress(pu->Address);
        }
        std::cout << "\n # of Unicast addresses: " << j;
        std::cout << "\n Anycast addresses:";
        j = 0;
        for (PIP_ADAPTER_ANYCAST_ADDRESS pa = tp->FirstAnycastAddress; pa != NULL; pa = pa->Next)
        {
            ++j;
            displayAddress(pa->Address);
        }
        std::cout << "\n # of Anycast addresses: " << j;
        std::cout << "\n Multicast addresses:";
        j = 0;
        for (PIP_ADAPTER_MULTICAST_ADDRESS pm = tp->FirstMulticastAddress; pm != NULL; pm = pm->Next)
        {
            ++j;
            displayAddress(pm->Address);
        }
        std::cout << "\n # of Multicast addresses: " << j;
        std::cout << "\n DNS server addresses:";
        j = 0;
        for (PIP_ADAPTER_DNS_SERVER_ADDRESS pd = tp->FirstDnsServerAddress; pd != NULL; pd = pd->Next)
        {
            ++j;
            displayAddress(pd->Address);
        }
        std::cout << "\n # of DNS server addresses: " << j;
        std::cout << "\n Gateway addresses:";
        j = 0;
        for (PIP_ADAPTER_GATEWAY_ADDRESS_LH pg = tp->FirstGatewayAddress; pg != NULL; pg = pg->Next)
        {
            ++j;
            displayAddress(pg->Address);
        }
        std::cout << "\n # of Gateway addresses: " << j;
        std::cout << "\n DNS suffix" << tp->DnsSuffix;
        std::cout << "\n Description" << tp->Description;
        std::cout << "\n Friendly name" << tp->FriendlyName;
        if (tp->PhysicalAddressLength != 0)
        {
            std::cout << "\n Physical address: ";
            std::cout << std::hex << (int)tp->PhysicalAddress[0];
            for (UINT i = 1; i < tp->PhysicalAddressLength; i++)
                std::cout << "-" << std::hex << (int)tp->PhysicalAddress[i];
        }
        std::cout << "\n Flags" << tp->Flags;
        std::cout << "\n MTU" << tp->Mtu;
        std::cout << "\n IfType" << tp->IfType;
        std::cout << "\n OperStatus" << tp->OperStatus;
        std::cout << "\n IPv6 IfIndex :" << tp->Ipv6IfIndex;
        std::cout << "\n and more...";
    }
    std::cout << "\n# of IP Adapters: " << i;

    HeapFree(GetProcessHeap(), 0, p);
}