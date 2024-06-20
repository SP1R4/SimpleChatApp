Client-Server Chat Application

This is a simple client-server chat application implemented in C using sockets and pthreads for multi-threading. The application allows multiple clients to connect to a server and communicate in a chatroom-like environment.

Features
Server (server_ergasia2.c):

Listens for incoming client connections.
Handles multiple clients concurrently using pthreads.
Broadcasts messages to all connected clients.
Supports termination of the server by sending a "terminate" command from a client or using CTRL + C.
Client (client_ergasia2.c):

Connects to the server using TCP/IP sockets.
Sends messages to the server.
Receives messages from the server and displays them.
Allows the user to enter their name and join the chatroom.
Requirements
Operating System: Linux (preferred) or other Unix-like systems
Compiler: GCC (GNU Compiler Collection) or compatible C compiler
Usage
Server
Compile the server code:

bash
Copy code
gcc server_ergasia2.c -o server -pthread
Run the server:

bash
Copy code
./server
The server will start listening for client connections on port 12345 by default.
To terminate the server:

Send a "terminate" command from any connected client.
Or press CTRL + C in the terminal running the server.
Client
Compile the client code:

bash
Copy code
gcc client_ergasia2.c -o client -pthread
Run the client:

bash
Copy code
./client
Enter your name when prompted.
Start sending and receiving messages in the chatroom.
To disconnect from the server:

Type "DISCONNECT" to exit the client application.
Implementation Details
Server (server_ergasia2.c):

Uses pthreads for handling multiple clients concurrently.
Utilizes mutex locks to synchronize access to shared resources (e.g., client list).
Handles termination signals (SIGINT) for graceful shutdown.
Client (client_ergasia2.c):

Connects to the server using TCP/IP sockets.
Implements separate threads for sending and receiving messages.
Handles user inputs and displays received messages.
Notes
This application demonstrates basic client-server communication using sockets in C.
It provides a framework for building more complex chat applications with additional features like user authentication, message encryption, or file sharing.
