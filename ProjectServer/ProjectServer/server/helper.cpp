#pragma comment(lib, "Ws2_32.lib")
#include "Helper.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <set>
#include "Server.h"
#include <array>
#include <mutex>

using std::string;

// recieves the type code of the message from socket (3 bytes)
// and returns the code. if no message found in the socket returns 0 (which means the client disconnected)
int Helper::getMessageTypeCode(SOCKET sc)
{
	char* s = getPartFromSocket(sc, 3);
	std::string msg(s);

	if (msg == "")
		return 0;

	int res = std::atoi(s);
	delete s;
	return res;
}


void Helper::send_update_message_to_client(SOCKET sc, const string& file_content, const string& second_username, const string &all_users)
{
	//TRACE("all users: %s\n", all_users.c_str())
	const string code = std::to_string(MT_SERVER_UPDATE);
	const string current_file_size = getPaddedNumber(file_content.size(), 5);
	const string username_size = getPaddedNumber(second_username.size(), 2);
	const string all_users_size = getPaddedNumber(all_users.size(), 5);
	const string res = code + current_file_size + file_content + username_size + second_username + all_users_size + all_users;
	//TRACE("message: %s\n", res.c_str());
	sendData(sc, res);
}

// recieve data from socket according byteSize
// returns the data as int
int Helper::getIntPartFromSocket(SOCKET sc, int bytesNum)
{
	char* s = getPartFromSocket(sc, bytesNum, 0);
	return atoi(s);
}

// recieve data from socket according byteSize
// returns the data as string
string Helper::getStringPartFromSocket(SOCKET sc, int bytesNum)
{
	char* s = getPartFromSocket(sc, bytesNum, 0);
	string res(s);
	return res;
}

// return string after padding zeros if necessary
string Helper::getPaddedNumber(int num, int digits)
{
	std::ostringstream ostr;
	ostr << std::setw(digits) << std::setfill('0') << num;
	return ostr.str();

}

std::string Helper::getUsersString(std::set<std::string> usernames)
{
	std::string result = "";
	for (const std::string& name : usernames)
	{
		result += name + "&";
	}
	if (result != "")
	{
		result.pop_back();
	}
	return result;
}

// recieve data from socket according byteSize
// this is private function
char* Helper::getPartFromSocket(SOCKET sc, int bytesNum)
{
	return getPartFromSocket(sc, bytesNum, 0);
}

char* Helper::getPartFromSocket(SOCKET sc, int bytesNum, int flags)
{
	if (bytesNum == 0)
	{
		char* empty = new char[1];
		empty[0] = '\0';
		return empty;
	}


	char* data = new char[bytesNum + 1];
	int res = recv(sc, data, bytesNum, flags);

	if (res == INVALID_SOCKET)
	{
		//std::string s = "Error while recieving from socket: ";
		//s += std::to_string(sc);
		char* str = new char[3];
		str[0] = '0';
		str[1] = '0';
		str[2] = '\0';
		return str;
	}
	data[bytesNum] = 0;
	return data;
}

// send data to socket
// this is private function
void Helper::sendData(SOCKET sc, std::string message)
{
	const char* data = message.c_str();

	if (send(sc, data, message.size(), 0) == INVALID_SOCKET)
	{
		throw std::exception("Error while sending message to client");
	}
}

std::string Helper::buildFileName(std::string secondUsername, std::string firstUsername)
{
	std::string folder = "server";
	std::string subFolder = "assets";
	std::string fileName = "";

	if (secondUsername < firstUsername)
	{
		fileName = folder + "/" + subFolder + "/" + secondUsername + "&" + firstUsername + ".txt";
	}
	else
	{
		fileName = folder + "/" + subFolder + "/" + firstUsername + "&" + secondUsername + ".txt";
	}
	return fileName;
}

std::string Helper::buildMagshMessageByFormat(std::string author, std::string newMessage)
{
	return "&MAGSH_MESSAGE&&Author&" + author + "&DATA&" + newMessage;
}

void Helper::insertMessageToFiles(Server& myServer)
{
	std::array<std::string, 3> msg;

	while (!myServer.messages.empty())
	{
		msg = myServer.messages.front();
		myServer.messages.pop();

		std::string firstUsername = msg[0];
		std::string newMessage = msg[1];
		std::string secondUsername = msg[2];

		std::string magshMessageByFormat = Helper::buildMagshMessageByFormat(firstUsername, newMessage);
		std::string fileName = Helper::buildFileName(secondUsername, firstUsername);

		std::ofstream outFile;
		outFile.open(fileName, std::ios::app);
		if (!outFile.is_open())
		{
			std::cout << "Error: Could not open or create " << fileName << std::endl;
		}
		else
		{
			outFile << magshMessageByFormat << std::endl;
			outFile.close();
		}
	}
}

std::string Helper::getFileContent(std::string firstUsername, std::string secondUsername)
{
	std::string fileName = Helper::buildFileName(secondUsername, firstUsername);
	std::ifstream file(fileName);
	if (file.is_open()) {
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}
	return "";
}