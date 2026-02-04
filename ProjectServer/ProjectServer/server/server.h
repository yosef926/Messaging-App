#pragma once

#include <WinSock2.h>
#include <Windows.h>
#include <set>
#include <string>
#include <mutex>
#include <queue>
#include <array>
#include "server.h"

#define PORT 8826
#define IP "127.0.0.1"

class Server
{
public:
	Server();
	~Server();
	void serve(int port);
	void loginHandle(SOCKET clientSocekt, std::string username);
	void clientUpdateHandle(SOCKET clientSocket, std::string secondUsername, int secondUserLength, std::string newMessage, std::string username);
	bool isUserInSystem(std::string username);
	void insertMessageToQueue(std::string username, std::string newMessage, std::string seocndMessage);

	std::condition_variable cond;
	std::mutex queueMutex;
	std::queue<std::array<std::string, 3>> messages; //Global var for all thread
private:
	void acceptClient();
	void clientHandler(SOCKET clientSocket);
	SOCKET _serverSocket;
	std::set<std::string> _usernames;
	std::mutex usersMutex;
};