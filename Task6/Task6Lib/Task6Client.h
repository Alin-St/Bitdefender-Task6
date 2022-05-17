#pragma once

#include "OutPipeClient.h"
#include "OutSharedMemoryClient.h"
#include "controller_options.h"
#include <memory>
#include <queue>
#include <array>

class Task6Client
{
private:
	std::unique_ptr<OutPipeClient> pipeClient;
	std::unique_ptr<OutSharedMemoryClient> sharedMemClient;
	std::unique_ptr<std::array<char, DATA_BUFFER_SIZE>> dataBuff;
	bool bDataSent = false;

	std::queue<std::string> log;

public:
	/// <summary> Start the client and connect to the pipe server. </summary>
	void startPipe();

	/// <summary> Start the client and connect to the shared memory server. </summary>
	void startSharedMemory();

	/// <summary> Disconnect the client. </summary>
	void stop();

	/// <summary> Wait until something happens on the client. </summary>
	/// <returns> True if something happened (has connected or sent data), false otherwise. </returns>
	bool wait(unsigned long timeout);

	/// <summary> Pop a message from the message queue. </summary>
	/// <param name="messg"> The received message (out parameter). Valid only if the function returns true. </param>
	/// <returns> True if a message was dequed and returned, false if no message was queued. </returns>
	bool popMessg(std::string& messg);

private:
	void pushMessg(std::string&& messg);

	/// <summary> Get a buffer of data to send to the server. Is not destroyed on the lifetime of the client. </summary>
	void getDataToSend(char*& buffer, size_t& size);
};
