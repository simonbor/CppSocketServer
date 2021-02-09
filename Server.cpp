#include "Server.h"
#include <exception>
#include <iostream>
#include <string>

Server::Server()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
	wVersionRequested = MAKEWORD(2, 2);



	// this server use TCP. that why SOCK_STREAM & IPPROTO_TCP
	// if the server use UDP we will use: SOCK_DGRAM & IPPROTO_UDP
	this->_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	err = WSAStartup(wVersionRequested, &wsaData);

	if (err != 0)
	{
		std::cout << "WSAStartup failed with error code " << err << std::endl;
		exit(1);
	}

	if (this->_serverSocket == INVALID_SOCKET)
	{
		throw std::exception(__FUNCTION__ " - socket");
	}
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

	// again stepping out to the global namespace
	// Connects between the socket and the configuration (port and etc..)
	if (bind(_serverSocket, (struct sockaddr*)&sa, sizeof(sa)) == SOCKET_ERROR)
		throw std::exception(__FUNCTION__ " - bind");

	// Start listening for incoming requests of clients
	if (listen(_serverSocket, SOMAXCONN) == SOCKET_ERROR)
		throw std::exception(__FUNCTION__ " - listen");
	std::cout << "Listening on port " << port << std::endl;

	while (true)
	{
		// the main thread is only accepting clients 
		// and add then to the list of handlers
		std::cout << "Waiting for client connection request" << std::endl;
		accept();
	}
}


void Server::accept()
{
	// notice that we step out to the global namespace
	// for the resolution of the function accept

	// this accepts the client and create a specific socket from server to this client
	SOCKET client_socket = ::accept(_serverSocket, NULL, NULL);

	if (client_socket == INVALID_SOCKET)
		throw std::exception(__FUNCTION__);

	std::cout << "Client accepted. Server and client can speak" << std::endl;

	// the function that handle the conversation with the client
	clientHandler(client_socket);
}


void Server::clientHandler(SOCKET clientSocket)
{
	try
	{
		std::thread clientThread(&Server::client, this, clientSocket);
		clientThread.detach();
		std::cout << "New client thread has been started!" << std::endl;
	}
	catch (const std::exception& e)
	{
		closesocket(clientSocket);
	}
}

void Server::client(SOCKET clientSocket)
{
	try
	{
		std::string name;
		
		while (true)
		{
			int code = Helper::getMessageTypeCode(clientSocket);
			if (code == 200)	// MT_CLIENT_LOG_IN
			{
				int len = Helper::getIntPartFromSocket(clientSocket, 2);
				name = Helper::getStringPartFromSocket(clientSocket, len);
				std::cout << "Hello new client " << name << "!" << std::endl;
				Helper::send_update_message_to_client(clientSocket, "", "", "shimon&moshe&ithik");
			}

			if (code == 204)	// MT_CLIENT_UPDATE
			{	
				int len = Helper::getIntPartFromSocket(clientSocket, 2);
				std::string userName = Helper::getStringPartFromSocket(clientSocket, len);
				len = Helper::getIntPartFromSocket(clientSocket, 5);
				std::string userMess = Helper::getStringPartFromSocket(clientSocket, len);
				std::cout << "New message from: " << name << " to: " << userName << " message: " << userMess << std::endl;
				Helper::sendData(clientSocket, userMess);
			}

			if (code == 207)	// MT_CLIENT_FINISH
			{	
				// ...
			}

			if (code == 208)	// MT_CLIENT_EXIT
			{	
				// ...
			}
		}
	}
	catch (...)
	{
		closesocket(clientSocket);
	}
}
