#pragma once

#include <WinSock2.h>
#include <Windows.h>
#include "Helper.h"
#include <thread>


class Server
{
public:
	Server();
	~Server();
	void serve(int port);

private:

	void accept();
	void clientHandler(SOCKET clientSocket);

	SOCKET _serverSocket;

	void client(SOCKET clientSocket);
};
