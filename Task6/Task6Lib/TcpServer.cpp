#include "TcpServer.h"
#include "SystemException.h"
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <MSWSock.h>
#include <stdexcept>
#include <string>

TcpServer::TcpServer(int port) : _port(port)
{
	// Initializing Winsock.
	WSADATA wsaData{};
	int errCode = 0;
	if (errCode = WSAStartup(MAKEWORD(2, 2), &wsaData))
		throw SystemException((DWORD)errCode);
}

void TcpServer::start(int backlog)
{
	if (this->active())
		throw std::logic_error("The server is already started.");

	// Resolve the local address and port to be used by the server.
	std::string portStr = std::to_string(this->_port);

	addrinfo hints{};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	addrinfo* addressInfo = nullptr;

	INT result = getaddrinfo(nullptr, portStr.c_str(), &hints, &addressInfo);
	if (result != 0)
		throw SystemException((DWORD)WSAGetLastError());

	if (addressInfo == nullptr)
		throw std::runtime_error("getaddrinfo returned a null adress.");

	// Create a SOCKET for the server to listen for client connections.
	this->_listener = socket(addressInfo->ai_family, addressInfo->ai_socktype, addressInfo->ai_protocol);
	if (this->_listener == INVALID_SOCKET) {
		freeaddrinfo(addressInfo);
		throw SystemException((DWORD)WSAGetLastError());
	}

	// Setup the TCP listening socket.
	int result2 = bind(this->_listener, addressInfo->ai_addr, (int)addressInfo->ai_addrlen);
	if (result2 != 0) {
		freeaddrinfo(addressInfo);
		closesocket(this->_listener);
		this->_listener = INVALID_SOCKET;
		throw SystemException((DWORD)WSAGetLastError());
	}

	freeaddrinfo(addressInfo);

	// Start listening.
	result2 = listen(this->_listener, backlog ? backlog : SOMAXCONN);
	if (result2 != 0) {
		closesocket(this->_listener);
		this->_listener = INVALID_SOCKET;
		throw SystemException((DWORD)WSAGetLastError());
	}
}

void TcpServer::accept()
{
	if (!this->active())
		throw std::logic_error("The server must be started first.");

	SOCKET client = INVALID_SOCKET;

	//AcceptEx(this->_listener, client)
}
