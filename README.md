# Messaging-App Engine(C++)

### Description
A multi-user messaging server developed in C++ using Winsock. The project is built as a dedicated back-end engine that utilizes multithreading within a single process to manage concurrent client connections. It focuses on TCP stream handling, thread-safe data relay, and real-time message broadcasting

![image_alt](https://github.com/yosef926/Messaging-App/blob/main/messageAppScreenshot.png?raw=true)

### Core Features
Socket Management: Implements reliable Winsock logic to handle persistent TCP connections and client-server handshakes.

Threaded Concurrency: Uses multithreading with mutexes and condition variables to synchronize simultaneous users and prevent data races.

Backend Architecture: Decoupled design focused on thread-safe message broadcasting and real-time relay between the server and external clients.

### How to Run  

To ensure the messaging system initializes correctly, you must launch the components in the following order:

Start the Backend: Run the server by compiling and executing main.cpp located in the server folder.

Start the Client: Run the messenger interface by executing MagshiChat.exe from the client folder.

Note: The server must be active before launching the client to allow the Winsock connection to be established successfully and enable real-time message relay.


### Technical SetupCompiler:  
g++ (or any standard C++ compiler)  
Execution Order: server (main.cpp) $\rightarrow$ client (MagshiChat.exe)  

