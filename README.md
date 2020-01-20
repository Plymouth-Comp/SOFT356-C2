# SOFT356 Interactive Network

## How to Use

The project is split into 2 parts, the client and the server.

### The Client

The client displays 2 models using information given by the server.

Using the client, the user connects the server that is running either on the local machine or another networked system running the server.

Once the client is loaded the user is prompted to use the default IP which will attempt to connect to the server running on the local machine. Selecting a custom IP allows to the client connect to a networked machine with the given IP running the server.

If the client successfully connects to the server then the a message will be sent to the server which will reply with the initial position, rotation and scale of the 2 models. With the initial model information, the client will draw the models at the correct area.

The user is able to navigate around the models using ‘WASD’ keys to move the camera Forward, Left, Down and Right respectively. Moving the mouse will allow the user to turn the camera and using the scroll wheel will zoom the camera in and out. Pressing 'ESC' will close the OpenGL window and terminate the client.

### The Server

If the server is not run on the local machine or on the local network then machine hosting the server will need to have its IP port forwarded which will be different depending on the router which can be found at [https://portforward.com/](https://portforward.com/). Once the hosting machine is port forwarded the client will need to connect using the hosts external IP which can be found at [https://www.yougetsignal.com/tools/open-ports/](https://www.yougetsignal.com/tools/open-ports/).

To start the server the user need to run the exe which will start the server listening for a client to connect. Only one client can connect to a server at a time and once the connection has been closed the server will need to be restarted.

Once a client has connected to the server the game objects can be changed. To change a game object the object to be edited needs to be chosen, to do this press ’1' or '2' to select which object to edit (object 1 is the car, object 2 is the gun). Selecting an object will show you its current properties. Once an object has been selected, pressing 'P', 'R' or 'S' will edit the Position, Rotation and scale respectively. Selecting to edit a property will prompt the user for three values (x, y, z) separated by the user pressing the enter key. The values must be a number otherwise the server will reject the input and will not allow anything else to be changed until three valid inputs are entered. Once all three inputs have been correctly entered the updated game object is sent to the connected client.