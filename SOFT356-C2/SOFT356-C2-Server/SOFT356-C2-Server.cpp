// SOFT356-C2-Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

#include <windows.h>

#include <iostream>
#include <vector>
#include <string>
#include <conio.h>
#include <thread>

#include <winsock2.h>
#include <ws2tcpip.h>

#include <glm.hpp> //includes GML

//Links the building enviroment to the libary
#pragma comment(lib, "Ws2_32.lib")

struct addrinfo
	* result = NULL,
	* ptr = NULL,
	hints;


struct GameObject {
	int id;
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
};

//ID,Pos,Rotation,Scale
GameObject objectOne{
	0,
	glm::vec3(1,-1.7,1),
	glm::vec3(0,0,0),
	glm::vec3(0.02,0.02,0.02),
};

GameObject objectTwo{
	1,
	glm::vec3(1,-1.7,1),
	glm::vec3(180,180,180),
	glm::vec3(0.2,0.2,0.2),
};


SOCKET clientSocket = INVALID_SOCKET;

bool terminateInput = false;


int SendToClient(char* message) {
	int iSendResult, iResult;

	//Send First Object
	iSendResult = send(clientSocket, message, DEFAULT_BUFLEN, 0);
	if (iSendResult == SOCKET_ERROR) {
		std::cout << "send failed: " << WSAGetLastError() << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}

	return 0;
}

//Converts the message from an object into a char string
char* SerializeGameObject(GameObject object) {
	
	std::string message;

	message += "{GameObject;" +
		std::to_string(object.id) + "," +
		std::to_string(object.position.x) + "," +
		std::to_string(object.position.y) + "," +
		std::to_string(object.position.z) + "," +
		std::to_string(object.rotation.x) + "," +
		std::to_string(object.rotation.y) + "," +
		std::to_string(object.rotation.z) + "," +
		std::to_string(object.scale.x) + "," +
		std::to_string(object.scale.y) + "," +
		std::to_string(object.scale.z) + "}";

	char* outMessage = new char[message.length() + 1];

	for (int i = 0; i < message.length(); i++) {
		outMessage[i] = message[i];
	}
	outMessage[message.length()] = '\0';

	return outMessage;
}


int DecodeMessage(char* string, std::vector<std::string>& values) {
	int count = 0;

	//Checks for the start of the message
	if (string[0] == '{') {
		count++;
		//Loops until the end of the string
		while (string[count] != '}') {
			std::string messageType;

			//Loops till the end of the message type
			while (string[count] != ';') {
				messageType.push_back(string[count]);
				count++;
			}
			count++;

			std::string newValue = "";

			//Loops until the end of the string
			while (string[count] != '}') {
				//Loops until the end of each value
				while (string[count] != ',' && string[count] != '}') {
					newValue.push_back(string[count]);
					count++;
				}
				//Adds the new value to the list of values
				values.push_back(newValue);

				//Resets the new vairable
				newValue = "";

				//Prevents count increasing at the end of the string
				if (string[count] != '}') {
					count++;
				}
			}

			if (messageType == "GameObject") {
				return 1;
			}
		}
	}

	//Returns 0 to show an error
	return 0;
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

	std::cout << "done!" << std::endl;

	return 0;
}

int ListenOnSocket(SOCKET& listenSocket) {
	//Listen for a connection
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		std::cout << "Listen failed with error: " << WSAGetLastError() << std::endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	return 0;
}

int AcceptConnection(SOCKET& listenSocket, SOCKET& clientSocket) {
	std::cout << "Listening for client" << std::endl;

	clientSocket = accept(listenSocket, NULL, NULL);
	if (clientSocket == INVALID_SOCKET) {
		std::cout << "accept failed: " << WSAGetLastError() << std::endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	std::cout << "Client connected!" << std::endl;

	return 0;
}

void Input() {
	GameObject* editGameObject = &objectOne;
	std::string valueInput;
	bool validInput = false;
	char* editSerialize = nullptr;

	//Loops until the terminate bool is fliped
	while (!terminateInput) {
		char input = _getch();

		switch (input)
		{
		case '1':
			std::cout << "Now Editing Object One" << std::endl;
			editGameObject = &objectOne;
			break;
		case '2':
			std::cout << "Now Editing Object Two" << std::endl;
			editGameObject = &objectOne;
			break;
		case 'p':
			std::cout << "Editing Object Position" << std::endl;

			for (int i = 0; i < 3; i++) {
				while (!validInput) {
					std::cout << "Enter Value " << i << std::endl;
					std::cin >> valueInput;

					try {
						float value = std::stof(valueInput.c_str());
						editGameObject->position[i] = value;
						std::cout << "Valid" << std::endl;
						validInput = true;
					}
					catch (std::exception e) {
						std::cout << "Invalid Value" << std::endl;
					}
				}
				validInput = false;
			}
			std::cout << "Position Succesfuly Edited" << std::endl;

			editSerialize = SerializeGameObject(*editGameObject);
			SendToClient(editSerialize);
			break;
		default:
			break;
		}

		std::cout << "Position: " << editGameObject->position.x << ", " << editGameObject->position.y << ", " << editGameObject->position.z << std::endl;
		std::cout << "Rotation: " << editGameObject->rotation.x << ", " << editGameObject->rotation.y << ", " << editGameObject->rotation.z << std::endl;
		std::cout << "Scale: " << editGameObject->scale.x << ", " << editGameObject->scale.y << ", " << editGameObject->scale.z << std::endl;
	}

	
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

	//Listens on the socket untill a client conects
	//For multiple clients, this should be run on a seperate thread
	ListenOnSocket(listenSocket);

	// Accept a client socket

	AcceptConnection(listenSocket, clientSocket);


	int iResult;

	//Recieve and send data
	char recvbuf[DEFAULT_BUFLEN];
	int iSendResult;
	int recvbuflen = DEFAULT_BUFLEN;

	std::thread inputThread(Input);

	// Receive until the peer shuts down the connection
	do {
		iResult = recv(clientSocket, recvbuf, recvbuflen, 0);

		if (iResult > 0) {
			std::cout << "Bytes received: " << std::endl;

			// Echo the buffer back to the sender

			char* objectOneSerialized = SerializeGameObject(objectOne);
			SendToClient(objectOneSerialized);
		

			char* objectTwoSerialized = SerializeGameObject(objectTwo);
			SendToClient(objectTwoSerialized);

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

	terminateInput = true;
	
	inputThread.join();
}
