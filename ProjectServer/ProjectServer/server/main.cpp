#pragma comment (lib, "ws2_32.lib")

#include "WSAInitializer.h"
#include "Server.h"
#include <iostream>
#include <exception>
#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include "Helper.h"
#include <fstream>

#define PORT 8826

std::mutex messagesMutex;

int main()
{
	try
	{
		WSAInitializer wsaInit;
		Server myServer;
		std::thread connectorThread(&Server::serve, &myServer, PORT);
		connectorThread.detach();
		while (true)
		{
			std::unique_lock<std::mutex> locker(myServer.queueMutex);
			myServer.cond.wait(locker, [&] { return !myServer.messages.empty(); });
			Helper::insertMessageToFiles(myServer);
		}
	}
	catch (std::exception& e)
	{
		std::cout << "Error occured: " << e.what() << std::endl;
	}
	system("PAUSE");
	return 0;
}