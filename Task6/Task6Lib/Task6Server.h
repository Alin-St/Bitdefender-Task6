#pragma once

#include "InPipeServer.h"
#include "InSharedMemoryServer.h"
#include "RaiiHandle.h"
#include "controller_options.h"
#include <vector>
#include <memory>
#include <queue>
#include <array>

/// <summary> The procedure that runs on all working threads from the iocp. </summary>
/// <param name="lpParameter"> Pointer to Task6Server. </param>
/// <returns> 0 on success. </returns>
DWORD WINAPI WorkerThreadProc(_In_ LPVOID lpParameter);

class Task6Server
{
	friend DWORD WINAPI WorkerThreadProc(_In_ LPVOID lpParameter);

private:
	RaiiHandle hCompPort;
	std::unique_ptr<InPipeServer> pipeServer;
	std::unique_ptr<InSharedMemoryServer> sharedMemServer;
	std::vector<RaiiHandle> workerThreads;

	// Buffers
	std::vector<std::array<char, DATA_BUFFER_SIZE>> pipeSvBuffers;

	std::unique_ptr<std::array<char, DATA_BUFFER_SIZE>> sharedMemoryBuffer;
	RaiiCriticalSection shMemBufferCS;

	// Message Log
	std::queue<std::string> log;
	RaiiCriticalSection logCS;
	RaiiHandle hLogSem;

	// Shutdown
	RaiiHandle hShutdownEvent;

public:
	Task6Server();

	/// <summary> Shutdown the server and wait for threads to complete. Use before destructor to close the threads gracefully. </summary>
	void stopAndWait();

	/// <summary> Waits until the queue is not empty, then pops a message from the message queue. </summary>
	/// <param name="timeout"> How long to wait until a message is available (in milliseconds). </param>
	/// <param name="messg"> The received message (out parameter). Valid only if the function returns true. </param>
	/// <returns> True if a message was dequed and returned, false if a timeout occured. </returns>
	bool popMessg(unsigned long timeout, std::string& messg);

private:
	void pushMessg(std::string&& messg);

	/// <summary> Handle a message from a client (with CNP, name etc..). </summary>
	void handleClientMessg(std::string messg);
};
