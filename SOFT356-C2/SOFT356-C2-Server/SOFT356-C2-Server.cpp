// SOFT356-C2-Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define DEFAULT_PORT "27015"

#include <windows.h>

#include <iostream>
#include <conio.h>

#include <winsock2.h>
#include <ws2tcpip.h>

//Links the building enviroment to the libary
#pragma comment(lib, "Ws2_32.lib")

int main()
{
	std::cout << "Starting Server!" << std::endl;


	WSADATA wsaData;
	int iResult;

	//Initialize Winsock
	std::cout << "Initializing Winsock: ";
	// MAKEWORD requests version 2.2 of winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed: " << iResult << std::endl;
		
		//return 1;
	}

	struct addrinfo
		*result = NULL,
		*ptr = NULL,
		hints;

	ZeroMemory(&hints, sizeof(hints));
	// AF_INET specifies IPv4
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		std::cout << "getaddrinfo failed: " << iResult << std::endl;
		WSACleanup();
		//return 1;
	}
	std::cout << "done!" << std::endl;

	// Creates the socket
	std::cout << "Creating Socket: ";
	SOCKET listenSocket = INVALID_SOCKET;

	listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	// Check for errors
	if (listenSocket == INVALID_SOCKET) {
		std::cout << "Error at socket:" << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		WSACleanup();
		//return 1;
	}
	std::cout << "done!" << std::endl;


	//Setup the TCP listening socket
	std::cout << "Binding socket: ";
	iResult = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		std::cout << "bind failed with error: " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		closesocket(listenSocket);
		WSACleanup();
		//return 1;
	}

	// Frees the addrinfo as it is no longer needed
	freeaddrinfo(result);

	std::cout << "done!" << std::endl;
	
	//Listen for a connection
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		std::cout << "Listen failed with error:" << WSAGetLastError() << std::endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}


	//TODO: Removed when server is functional
	//Stops the server from auto closing
	std::string test;
	std::cin >> test;
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
