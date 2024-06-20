Client-Server Chat Application
This is a simple client-server chat application implemented in C using TCP/IP sockets and pthreads for multi-threading. It allows multiple clients to connect to a server and communicate in a chatroom-like environment.

Features:
  Server (server.c):
    - Listens for incoming client connections.
    - Handles multiple clients concurrently using pthreads.
    - Broadcasts messages to all connected clients.
    - Supports termination of the server by sending a "terminate" command from a client or using CTRL + C.
  
  Client (client.c):
    - Connects to the server using TCP/IP sockets.
    - Sends messages to the server.
    - Receives messages from the server and displays them.
    - Allows the user to enter their name and join the chatroom.

Requirements
 - Operating System: Linux (preferred) or other Unix-like systems
 - Compiler: GCC (GNU Compiler Collection) or compatible C compiler

Usage

Server
1. Compile the server code:
  gcc server.c -o server -pthread

2. Run the server:
  ./server
- The server will start listening for client connections on port 12345 by default.

3.Terminating the server:
  - Send a "terminate" command from any connected client.
  - Or press CTRL + C in the terminal running the server.

Client
1.Compile the client code:
  gcc client.c -o client -pthread

2.Run the client:
  ./client

3.Disconnecting from the server:
  Type "DISCONNECT" to exit the client application.


Implementation Details
Server (server.c):

 - Uses pthreads for handling multiple clients concurrently.
 - Utilizes mutex locks to synchronize access to shared resources (e.g., client list).
 - Handles termination signals (SIGINT) for graceful shutdown.

Client (client.c):

 - Connects to the server using TCP/IP sockets.
 - Implements separate threads for sending and receiving messages.
 - Handles user inputs and displays received messages.

Notes
  - This application serves as a foundational example of client-server communication using sockets in C.
  - It provides a framework for developing more sophisticated chat applications with features such as user authentication, message encryption, or file sharing.
