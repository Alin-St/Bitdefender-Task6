#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

class TcpServer
{
private:
	SOCKET _listener = INVALID_SOCKET;
	int _port = 0;

public:
	/// <summary> Initializes a new instance of the TcpServer class that listens for incoming connection attempts
	/// on the local IP address and the specified port number. </summary>
	/// <param name="port"> The port on which to listen for incoming connection attempts. </param>
	TcpServer(int port);

	~TcpServer();
	TcpServer(const TcpServer&) = delete;
	TcpServer(TcpServer&&) = delete;
	TcpServer& operator=(const TcpServer&) = delete;
	TcpServer& operator=(TcpServer&&) = delete;

	/// <summary> Get the port number being used to listen for incoming client connection requests. </summary>
	/// <returns> The port to which the socket is bound. </returns>
	inline int port() const { return this->_port; }

	/// <summary> Gets a value that indicates whether TcpServer is actively listening for client connections. </summary>
	/// <returns> true if TcpServer is actively listening; otherwise, false. </returns>
	inline bool active() const { return this->_listener != INVALID_SOCKET; }

	/// <summary> Starts listening for incoming connection requests with a maximum number of pending connection. </summary>
	/// <param name="backlog"> The maximum length of the pending connections queue. If set to 0, the underlying service provider
	/// responsible for the underlying socket will set the backlog to a maximum reasonable value. </param>
	void start(int backlog);


	void accept();
};
