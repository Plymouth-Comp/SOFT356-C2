// SOFT356-C2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

#include <windows.h>

#include <iostream>
#include <conio.h>

#include <winsock2.h>
#include <ws2tcpip.h>

//Links the building enviroment to the libary
#pragma comment(lib, "Ws2_32.lib")

int main()
{
    std::cout << "Starting Client!" << std::endl;


	WSADATA wsaData;
	int iResult;

	//Initialize Winsock
	// MAKEWORD requests version 2.2 of winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed: " << iResult << std::endl;

		//return 1;
	}

	//Creates a socket
	// Declares an object wich contains the sockaddr and initializes the values
	struct addrinfo
		* result = NULL,
		* ptr = NULL,
		hints;

	ZeroMemory(&hints, sizeof(hints));
	//  AF_UNSPEC allows for either IPv4 or IPv6
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Gets the host ip from the user
	std::string hostIPAddress;
	std::cout << "Enter host ip address: ";
	std::cin >> hostIPAddress;



	// Resolve the server address and port
	iResult = getaddrinfo(hostIPAddress.c_str(), DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		std::cout << "getaddrinfo failed: " << iResult << std::endl;
		WSACleanup();
		//return 1;
	}

	// Create a socket object
	SOCKET connectSocket = INVALID_SOCKET;

	// Attempt to connect to the first address returned by the call to getaddrinfo
	ptr = result;

	// Create a SOCKET for connecting to server
	connectSocket = socket(ptr->ai_family, ptr->ai_socktype,
		ptr->ai_protocol);

	// Check for errors
	if (connectSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		//return 1;
	}


	// Connect to server.
	iResult = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(connectSocket);
		connectSocket = INVALID_SOCKET;
	}

	freeaddrinfo(result);

	if (connectSocket == INVALID_SOCKET) {
		std::cout << "Unable to connect to server" << std::endl;
		WSACleanup();
		//return 1;
	}

	std::cout << "Client connected to server at: " << hostIPAddress << std::endl;



	int recvbuflen = DEFAULT_BUFLEN;

	const char* sendbuf = "this is a test";
	char recvbuf[DEFAULT_BUFLEN];

	// Send an initial buffer
	iResult = send(connectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		std::cout << "send failed: " << iResult << std::endl;
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	std::cout << "Bytes Sent: " << iResult << std::endl;

	// shutdown the connection for sending since no more data will be sent
	// the client can still use the ConnectSocket for receiving data
	iResult = shutdown(connectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		std::cout << "shutdown failed: " << WSAGetLastError() << std::endl;
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	// Receive data until the server closes the connection
	do {
		iResult = recv(connectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
			std::cout << "Bytes received:" << iResult << std::endl;
		else if (iResult == 0)
			std::cout << "Connection closed" << std::endl;
		else
			std::cout << "recv failed: " << WSAGetLastError() << std::endl;
	} while (iResult > 0);
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
