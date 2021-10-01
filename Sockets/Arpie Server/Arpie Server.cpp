#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

int main ()
{
	WSADATA wsaData;
	SOCKET SendingSocket;

	// Address
	SOCKADDR_IN ServerAddr, ThisSenderInfo;

	unsigned int Port = 7171;

	int ret;

	char sendbuf[1024];
	int BytesSent, nlen;

	// Initialize Winsock v2.2
	WSAStartup(MAKEWORD(2,2), &wsaData);

	printf("Winsock DLL status is %s.\n", wsaData.szSystemStatus);

	// Create a new socket to make a client connection.
	// AF_INET = 2, The Internet Protocol version 4 (IPv4) address family, TCP protocol
	SendingSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (SendingSocket == INVALID_SOCKET)
	{
		printf("Client: socket() failed! Error code: %ld\n", WSAGetLastError());

		// Do the clean up
		WSACleanup();

		// Exit with error
		return -1;
	}
	else
	{
		printf("Client: socket() is OK!\n");
	}


	// Set up a SOCKADDR_IN structure that will be used to connect
	// to a listening server on port 5150. For demonstration
	// purposes, let's assume our server's IP address is 127.0.0.1 or localhost

	// IPv4
	ServerAddr.sin_family = AF_INET;

	// Port no.
	ServerAddr.sin_port = htons(Port);

	// The IP address
	inet_pton(AF_INET,"127.0.0.1", &ServerAddr.sin_addr.s_addr);

	// Make a connection to the server with socket SendingSocket.
	ret = connect(SendingSocket, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr));

	if (ret != 0)
	{
		printf("Client: connect() failed! Error code: %ld\n", WSAGetLastError());

		// Close the socket
		closesocket(SendingSocket);

		// Do the clean up
		WSACleanup();

		// Exit with error
		return -1;
	}
	else
	{
		printf("Client: connect() is OK, got connected...\n");

		printf("Client: Ready for sending and/or receiving data...\n");
	}
}