#pragma once

#include "RaiiHandle.h"
#include "RaiiCriticalSection.h"
#include <string>

/// <summary> The code executed by the thread that waits for the client to server event and adds the completion to the iocp. </summary>
/// <param name="lpParameter"> Pointer to an InSharedMemoryServer. </param>
/// <returns> 0 on success. </returns>
DWORD WINAPI WaiterThreadProc(_In_ LPVOID lpParameter);

class InSharedMemoryServer
{
	friend DWORD WINAPI WaiterThreadProc(_In_ LPVOID lpParameter);

private:
	RaiiHandle hSharedMemory;
	RaiiHandle hSvToClEvent;
	RaiiHandle hClToSvEvent;
	RaiiHandle hShutdownEvent;
	RaiiHandle hThread;
	RaiiCriticalSection critSection;

	HANDLE hCompPort;

	unsigned long bufferSize;
	std::string name;
	std::string svToClEventName;
	std::string clToSvEventName;
	ULONG_PTR compKey;

public:
	/// <summary> Initalize a shared memory server. </summary>
	/// <param name="bufferSize"> The size of the shared memory. </param>
	/// <param name="name"> The name of the shared memory. </param>
	/// <param name="svToClEventName"> The name of the event that signals that clients are ready to write. </param>
	/// <param name="clToSvEventName"> The name of the event that signals that server is ready to read. </param>
	/// <param name="hCompPort"> A completion port where a message will be posted when the server is ready to read. </param>
	/// <param name="compKey"> The completion key associated with this server on the iocp. </param>
	InSharedMemoryServer(unsigned long bufferSize, const std::string& name, const std::string& svToClEventName,
		const std::string& clToSvEventName, HANDLE hCompPort, ULONG_PTR compKey);

	/// <summary> Get the data from the shared memory. Make sure the client finished sending data, using the
	/// iocp, before calling this method. Signals another client to send data after completion. </summary>
	/// <param name="buffer"> A buffer where the data is copied to. </param>
	/// <param name="size"> The size of the given buffer. It is recommended to be as large as the shared memory buffer size. </param>
	/// <returns> The number of bytes received from the shared memory. </returns>
	size_t getData(char* buffer, size_t size);

	~InSharedMemoryServer();
};
