#include <winsock2.h>
#include "Server.h"
#include <exception>
#include <iostream>
#include <string>
#include <thread>
#include "helper.h"
#include <fstream>
#include <sstream>
#include <chrono>
#include <array>

using namespace std::chrono_literals;

#define LEGNTH_PART 5
#define LENGTH_START_INDEX 3

Server::Server()
{

	// this server use TCP. that why SOCK_STREAM & IPPROTO_TCP
	// if the server use UDP we will use: SOCK_DGRAM & IPPROTO_UDP
	this->_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (this->_serverSocket == INVALID_SOCKET)
		throw std::exception(__FUNCTION__ " - socket");
		//int err = WSAGetLastError();
		//std::string msg = "Socket creation failed with error: " + std::to_string(err.c_str());
		//throw std::exception(msg.c_str());
}

Server::~Server()
{
	try
	{
		// the only use of the destructor should be for freeing 
		// resources that was allocated in the constructor
		closesocket(this->_serverSocket);
	}
	catch (...) {}
}

void Server::serve(int port)
{

	struct sockaddr_in sa = { 0 };

	sa.sin_port = htons(port); // port that server will listen for
	sa.sin_family = AF_INET;   // must be AF_INET
	sa.sin_addr.s_addr = INADDR_ANY;    // when there are few ip's for the machine. We will use always "INADDR_ANY"

	// Connects between the socket and the configuration (port and etc..)
	if (bind(this->_serverSocket, (struct sockaddr*)&sa, sizeof(sa)) == SOCKET_ERROR)
		throw std::exception(__FUNCTION__ " - bind");

	// Start listening for incoming requests of clients
	if (listen(this->_serverSocket, SOMAXCONN) == SOCKET_ERROR)
		throw std::exception(__FUNCTION__ " - listen");
	std::cout << "Listening on port " << port << std::endl;

	while (true)
	{
		// the main thread is only accepting clients 
		// and add then to the list of handlers
		std::cout << "Waiting for client connection request" << std::endl;
		acceptClient();
	}
}


void Server::acceptClient()
{

	// this accepts the client and create a specific socket from server to this client
	// the process will not continue until a client connects to the server
	SOCKET client_socket = accept(this->_serverSocket, NULL, NULL);
	if (client_socket == INVALID_SOCKET)
		throw std::exception(__FUNCTION__);

	std::cout << "\nClient accepted. Server and client can speak" << std::endl;
	
	// the function that handle the conversation with the client
	std::thread clientThread(&Server::clientHandler, this, client_socket);
	clientThread.detach();
}


void Server::clientHandler(SOCKET clientSocket)
{
	std::string newMessage = "";
	std::string secondUsername = "";
	int lengthSecondUsername = 0;
	int lengthNewMessage = 0;
	try
	{
		std::cout << "User has open his app...\n";
		int code = Helper::getMessageTypeCode(clientSocket);
		int length = Helper::getIntPartFromSocket(clientSocket, 2);
		std::string username = Helper::getStringPartFromSocket(clientSocket, length);
		loginHandle(clientSocket, username);

		while (true)
		{
			code = Helper::getMessageTypeCode(clientSocket);
			if (code == 0) // user dissconected
			{
				break;
			}
			lengthSecondUsername = Helper::getIntPartFromSocket(clientSocket, 2);
			secondUsername = Helper::getStringPartFromSocket(clientSocket, lengthSecondUsername);

			lengthNewMessage = Helper::getIntPartFromSocket(clientSocket, 5);
			newMessage = Helper::getStringPartFromSocket(clientSocket, lengthNewMessage);

			clientUpdateHandle(clientSocket, secondUsername, lengthSecondUsername, newMessage, username);
			std::this_thread::sleep_for(200ms);
		}
		// Closing the socket (in the level of the TCP protocol)
		{
			std::lock_guard<std::mutex> locker(this->usersMutex);
			this->_usernames.erase(username);
		}		
		std::cout << "SOCKET and THREAD is closed!";
		closesocket(clientSocket);
		
	}
	catch (const std::exception& e)
	{
		closesocket(clientSocket);
	}
}

void Server::loginHandle(SOCKET clientSocket, std::string username)
{
	std::string usernameStringFormat = "";

	{
		std::lock_guard<std::mutex> locker(this->usersMutex);
		this->_usernames.insert(username);
		usernameStringFormat = Helper::getUsersString(this->_usernames);
	}
	std::cout << "ADDED new client, " << username << ", to clients list\n\n";

	//send ServerUpdateMessage
	Helper::send_update_message_to_client(clientSocket, "0", "0", usernameStringFormat);
}

void Server::clientUpdateHandle(SOCKET clientSocket, std::string secondUsername, int secondUserLength, std::string newMessage, std::string username)
{
	if (newMessage != "") //Is clientUpdateMessage is a message that include a new message and not just the first "after-login" mesage that is without message content.
	{
		insertMessageToQueue(username, newMessage, secondUsername);
	}

	std::string fileContent = Helper::getFileContent(username, secondUsername);
	std::string AllUsernames;
	{
		std::lock_guard<std::mutex> lock(this->usersMutex);
		AllUsernames = Helper::getUsersString(this->_usernames);
	}
	Helper::send_update_message_to_client(clientSocket, fileContent, secondUsername, AllUsernames);
}

bool Server::isUserInSystem(std::string username)
{
	std::lock_guard<std::mutex> locker(this->usersMutex);
	return this->_usernames.find(username) != this->_usernames.end();
}

void Server::insertMessageToQueue(std::string username, std::string newMessage, std::string secondUsername)
{
	std::array<std::string, 3> msgArray = { username, newMessage, secondUsername }; //Format I decided to work with in the queue.
	{
		std::lock_guard<std::mutex> locker(this->queueMutex);
		this->messages.push(msgArray);
	}
	this->cond.notify_one();
}
