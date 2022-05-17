#pragma once

#include "RaiiHandle.h"
#include <Windows.h>
#include <string>

class OutSharedMemoryClient
{
private:
	RaiiHandle hSharedMemory;
	RaiiHandle hSvToClEvent;
	RaiiHandle hClToSvEvent;

	std::string serverName;
	std::string svToClEventName;
	std::string clToSvEventName;

public:
	/// <summary> Initialize the client and open the named shared memory and the events. </summary>
	/// <param name="serverName"> The name of the shared memory on the server side. </param>
	/// <param name="svToClEventName"> The name of the server to client event. </param>
	/// <param name="clToSvEventName"> The name of the client to server event. </param>
	OutSharedMemoryClient(const std::string& serverName, const std::string& svToClEventName, const std::string& clToSvEventName);

	/// <summary> Write the given data to the shared memory and alert the server. </summary>
	/// <param name="buffer"> A pointer to the data you want to write. </param>
	/// <param name="size"> The number of bytes in the buffer. </param>
	/// <param name="timeout"> The number of milliseconds to wait for the server to be ready to accept data. </param>
	/// <returns> True if the data was written, false for a timeout. </returns>
	bool writeData(char* buffer, size_t size, unsigned long timeout);
};
