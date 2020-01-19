// SOFT356-C2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512
#define DEFAULT_IP "127.0.0.1"

#include <windows.h>

#include <iostream>
#include <conio.h>

#include <winsock2.h>
#include <ws2tcpip.h>

//OpenGL
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GLFW/glfw3.h"

//Assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//Links the building enviroment to the libary
#pragma comment(lib, "Ws2_32.lib")

// Declares an object wich contains the sockaddr and initializes the values
struct addrinfo
	* result = NULL,
	* ptr = NULL,
	hints;



void OpenGL() {
	glfwInit();

	GLFWwindow* window = glfwCreateWindow(800, 600, "Model Viewer", NULL, NULL);

	glfwMakeContextCurrent(window);
	glewInit();

	while (!glfwWindowShouldClose(window)) {
		//display(delta, *object, objectScale);
		glfwSwapBuffers(window);
		glfwPollEvents();

		//Incriment time
		//delta += 0.1f;

		//Key controls
		// CLoses the window when 'Q' is pressed
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
			std::string input;

			std::cout << "Are you sure? (Y/N)" << std::endl;
			char quitConfirmation = ' ';

			while (quitConfirmation != 'y' && quitConfirmation != 'n') {
				quitConfirmation = _getch();

				if (quitConfirmation == 'y') {
					std::cout << "Closing Model Viewer" << std::endl;
					glfwSetWindowShouldClose(window, true);
				}
			}
		}

	}

	std::cout << "Closed Window" << std::endl;


	glfwDestroyWindow(window);

	glfwTerminate();

}



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

int CreateSocket(SOCKET& connectSocket) {
	int iResult;

	//Creates a socket
	ZeroMemory(&hints, sizeof(hints));
	//  AF_UNSPEC allows for either IPv4 or IPv6
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Gets the ip from the user
	char ipSelection = 'u';
	std::string hostIPAddress;

	//  Loops until valid input
	while (ipSelection != 'n' && ipSelection != 'N' && ipSelection != 'y' && ipSelection != 'Y') {
		std::cout << "Used default IP (127.0.0.1)? Y/N" << std::endl;
		ipSelection = _getch();

	}

	//  Sets the ip depending on the users selection
	if (ipSelection == 'n' || ipSelection == 'N') {
		// Gets the host ip from the user
		std::cout << "Enter host ip address: ";
		std::cin >> hostIPAddress;
	}
	else {
		hostIPAddress = DEFAULT_IP;
	}

	// Resolve the server address and port
	iResult = getaddrinfo(hostIPAddress.c_str(), DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		std::cout << "getaddrinfo failed: " << iResult << std::endl;
		WSACleanup();
		//return 1;
	}

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
		return 1;
	}

	return 0;
}

int ConnectToServer(SOCKET& connectSocket) {
	int iResult;

	//Connect to server.
	iResult = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(connectSocket);
		connectSocket = INVALID_SOCKET;
	}

	freeaddrinfo(result);

	if (connectSocket == INVALID_SOCKET) {
		std::cout << "Unable to connect to server" << std::endl;
		WSACleanup();
		return 1;
	}

	std::cout << "Client connected to server!" << std::endl;

	return 0;
}

int SendDataToServer(SOCKET& connectSocket, int recvbuflen, char* recvbuf, const char* message) {

	int iResult;

	// Send an initial buffer
	iResult = send(connectSocket, message, (int)strlen(message), 0);
	if (iResult == SOCKET_ERROR) {
		std::cout << "send failed: " << iResult << std::endl;
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	std::cout << "Bytes Sent: " << iResult << std::endl;

	return 0;
}

int ShutdownOutgoingConnection(SOCKET& connectSocket) {
	int iResult;

	//Shutdown the connection for sending since no more data will be sent
	// the client can still use the ConnectSocket for receiving data
	iResult = shutdown(connectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		std::cout << "shutdown failed: " << WSAGetLastError() << std::endl;
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	return 0;
}

int ReciveData(SOCKET& connectSocket, int recvbuflen, char* recvbuf) {
	int iResult;

	// Receive data until the server closes the connection
	do {
		iResult = recv(connectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
			std::cout << "Bytes received:" << iResult << std::endl;
		else if (iResult == 0)
			std::cout << "Connection closed" << std::endl;
		else
			if (WSAGetLastError() == 10054) {
				std::cout << "Server closed the connection" << std::endl;
			}
			else {
				std::cout << "recv failed: " << WSAGetLastError() << std::endl;
			}
	} while (iResult > 0);

	return 0;
}

int main()
{
	std::cout << "Starting Client!" << std::endl;

	std::cout << "Starting OpenGL" << std::endl;
	OpenGL();

	_getch();

	//Start winsock
	InitializeWinsock();

	//Create the socket
	SOCKET connectSocket = INVALID_SOCKET;
	CreateSocket(connectSocket);

	//Connect to the server using the socket
	ConnectToServer(connectSocket);

	//Send a message to the server
	const char* message = "this is a test";
	int recvbuflen = DEFAULT_BUFLEN;
	char recvbuf[DEFAULT_BUFLEN];
	//SendDataToServer(connectSocket, recvbuflen, recvbuf, message);

	std::string msg = "Object";
	recvbuflen = sizeof(msg);
	recvbuf[sizeof(msg)];

	SendDataToServer(connectSocket, recvbuflen, recvbuf, msg.c_str());

	//No more data needs to be send so the outgoing connection is stopped
	ShutdownOutgoingConnection(connectSocket);

	//Keeps reciving data untill server closes connection
	ReciveData(connectSocket, recvbuflen, recvbuf);
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
