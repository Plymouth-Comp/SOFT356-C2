// SOFT356-C2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512
#define DEFAULT_IP "127.0.0.1"

#define STB_IMAGE_IMPLEMENTATION

#include <windows.h>

#include <iostream>
#include <conio.h>
#include <vector>
#include <thread>

#include <winsock2.h>
#include <ws2tcpip.h>

//OpenGL
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GLFW/glfw3.h"

//GLM
#include <glm.hpp> //includes GML
#include <ext/matrix_transform.hpp> // GLM: translate, rotate
#include <ext/matrix_clip_space.hpp> // GLM: perspective and ortho 
#include <gtc/type_ptr.hpp> // GLM: access to the value_ptr

#include "Model.h"
#include "Camera.h"
#include "Shader.h"

//Links the building enviroment to the libary
#pragma comment(lib, "Ws2_32.lib")


//Socket
SOCKET connectSocket = INVALID_SOCKET;
int recvbuflen = DEFAULT_BUFLEN;
char recvbuf[DEFAULT_BUFLEN];

//Settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

//Camera
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

//Time
float deltaTime = 0.0f;
float lastFrame = 0.0f;


// Declares an object wich contains the sockaddr and initializes the values
struct addrinfo
	* result = NULL,
	* ptr = NULL,
	hints;

struct GameObject {
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	Model model;
};

std::vector<GameObject> gameObjects;

//Messages
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

int UpdateGameObject(std::vector<std::string>& values) {
	//"{GameObject;1,3,4,5,8,9,10}";

	try {
		std::cout << "Recived Message" << std::endl;

		//Gets the id from the message
		int objectID = std::stoi(values[0].c_str());

		//Checks if the objectID is out of range
		if (gameObjects.size() < 10 && gameObjects.size() > 0) {
			gameObjects[objectID].position.x = std::stof(values[1].c_str());
			gameObjects[objectID].position.y = std::stof(values[2].c_str());
			gameObjects[objectID].position.z = std::stof(values[3].c_str());

			gameObjects[objectID].rotation.x = std::stof(values[4].c_str());
			gameObjects[objectID].rotation.y = std::stof(values[5].c_str());
			gameObjects[objectID].rotation.z = std::stof(values[6].c_str());

			gameObjects[objectID].scale.x = std::stof(values[7].c_str());
			gameObjects[objectID].scale.y = std::stof(values[8].c_str());
			gameObjects[objectID].scale.z = std::stof(values[9].c_str());
		}
	}
	catch (std::exception e) {
		std::cout << "ERROR::UPDATE GAMEOBJECT" << std::endl;
	}


	return 0;
}

//OpenGL
// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}



//Sockets
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
	iResult = send(connectSocket, message, DEFAULT_BUFLEN, 0);
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


int ReciveData() {
	int iResult;

	// Receive data until the server closes the connection
	do {
		iResult = recv(connectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			std::vector<std::string> values;

			int messageType = DecodeMessage(recvbuf, values);

			if (messageType == 1) {
				UpdateGameObject(values);
			}
		}
		else if (iResult == 0) {
			std::cout << "Connection closed" << std::endl;
		}
		else {
			if (WSAGetLastError() == 10054) {
				std::cout << "Server closed the connection" << std::endl;
			}
			else {
				std::cout << "recv failed: " << WSAGetLastError() << std::endl;
			}
		}
	} while (iResult > 0);

	return 0;
}

void MainLoop() {
	glfwInit();


	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Model Viewer", NULL, NULL);

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewInit();

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	//Shader
	Shader shader("shaders/mesh.vert", "shaders/mesh.frag");

	//Model
	char modelPath[] = { "C:/Store/Repo/SOFT356-C2/SOFT356-C2/SOFT356-C2/models/Lamborginhi Aventador OBJ/Lamborghini_Aventador.obj" };
	Model newModel(modelPath);

	//Adds the first test object
	GameObject newGameObject = {
		glm::vec3(0,0,0),
		glm::vec3(0,0,0),
		glm::vec3(0,0,0),
		newModel
	};

	gameObjects.push_back(newGameObject);

	//Adds the second test object
	char modelPathTwo[] = { "C:/Store/Repo/SOFT356-C2/SOFT356-C2/SOFT356-C2/models/47-obj/obj/Handgun_obj.obj" };
	Model newModelTwo(modelPathTwo);

	GameObject newGameObjectTwo = {
		glm::vec3(0,0,0),
		glm::vec3(0,0,0),
		glm::vec3(0,0,0),
		newModelTwo
	};
	
	gameObjects.push_back(newGameObjectTwo);


	//Keeps reciving data untill server closes connection
	std::thread dataThread(ReciveData);


	//Main loop
	while (!glfwWindowShouldClose(window)) {
		// per-frame time logic
	   // --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// don't forget to enable shader before setting uniforms
		shader.use();

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		shader.setMat4("projection", projection);
		shader.setMat4("view", view);

		

		//Draws all models
		for (int i = 0; i < gameObjects.size(); i++) {
			// render the loaded model
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::rotate(model, gameObjects[i].rotation.x, glm::vec3(1, 0, 0));
			model = glm::rotate(model, gameObjects[i].rotation.y, glm::vec3(0, 1, 0));
			model = glm::rotate(model, gameObjects[i].rotation.z, glm::vec3(0, 0, 1));

			model = glm::translate(model, gameObjects[i].position); // translate it down so it's at the center of the scene
			model = glm::scale(model, gameObjects[i].scale);	// it's a bit too big for our scene, so scale it down
			shader.setMat4("model", model);

			gameObjects[i].model.Draw(shader);
		}


		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//No more data needs to be send so the outgoing connection is stopped
	ShutdownOutgoingConnection(connectSocket);

	std::cout << "Closed Window" << std::endl;

	glfwTerminate();

}

int main()
{
	std::cout << "Starting Client!" << std::endl;

	//Start winsock
	InitializeWinsock();

	//Create the socket
	CreateSocket(connectSocket);

	//Connect to the server using the socket
	ConnectToServer(connectSocket);

	std::string msg = "Object";
	recvbuflen = DEFAULT_BUFLEN;
	recvbuf[DEFAULT_BUFLEN];

	SendDataToServer(connectSocket, recvbuflen, recvbuf, msg.c_str());

	std::cout << "Starting OpenGL" << std::endl;
	MainLoop();
}