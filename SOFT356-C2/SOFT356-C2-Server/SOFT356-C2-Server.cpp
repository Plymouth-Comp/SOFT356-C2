// SOFT356-C2-Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
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

struct addrinfo
	* result = NULL,
	* ptr = NULL,
	hints;


int InitializeWinsock() {
	WSADATA wsaData;
	int iResult;

	//Initialize Winsock
	// MAKEWORD requests version 2.2 of winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed: " << iResult << std::endl;

		return 1;
	}

	return 0;
}

int CreateSocket(SOCKET& listenSocket) {
	int iResult;

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
		return 1;
	}
	std::cout << "done!" << std::endl;

	// Creates the socket
	std::cout << "Creating Socket: ";

	listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	// Check for errors
	if (listenSocket == INVALID_SOCKET) {
		std::cout << "Error at socket:" << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	std::cout << "done!" << std::endl;

	return 0;
}

int BindSocket(SOCKET& listenSocket) {
	int iResult;

	//Setup the TCP listening socket
	std::cout << "Binding socket: ";
	iResult = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		std::cout << "bind failed with error: " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	// Frees the addrinfo as it is no longer needed
	freeaddrinfo(result);

	return 0;
}

int main()
{
	std::cout << "Starting Server!" << std::endl;

	//Start winsock
	InitializeWinsock();

	//Create the socket
	SOCKET listenSocket = INVALID_SOCKET;
	CreateSocket(listenSocket);
	
	//Bind the socket
	BindSocket(listenSocket);


	int iResult;

	std::cout << "done!" << std::endl;
	
	//Listen for a connection
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		std::cout << "Listen failed with error: " << WSAGetLastError() << std::endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}


	//Listens for a single connection 
	// Accept a client socket
	SOCKET clientSocket = INVALID_SOCKET;

	std::cout << "Listening for client" << std::endl;
	
	clientSocket = accept(listenSocket, NULL, NULL);
	if (clientSocket == INVALID_SOCKET) {
		std::cout << "accept failed: " << WSAGetLastError() << std::endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	std::cout << "Client connected!" << std::endl;



	//Recieve and send data
	char recvbuf[DEFAULT_BUFLEN];
	int iSendResult;
	int recvbuflen = DEFAULT_BUFLEN;

	// Receive until the peer shuts down the connection
	do {
		iResult = recv(clientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			std::cout << "Bytes received: " << std::endl;

			std::cout << "Message: " << recvbuf << std::endl;

			// Echo the buffer back to the sender
			iSendResult = send(clientSocket, recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				std::cout << "send failed: " << WSAGetLastError() << std::endl;
				closesocket(clientSocket);
				WSACleanup();
				//return 1;
			}

			std::cout << "Bytes sent: " << std::endl;
		}
		else if (iResult == 0)
			std::cout << "Connection closing..." << std::endl;
		else {
			std::cout << "recv failed: " << WSAGetLastError() << std::endl;
			closesocket(clientSocket);
			WSACleanup();
			//return 1;
		}
	} while (iResult > 0);

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
