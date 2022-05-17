#include "InSharedMemoryServer.h"
#include "SystemException.h"
#include <stdexcept>
#include <cassert>

DWORD WINAPI WaiterThreadProc(_In_ LPVOID lpParameter)
{
	InSharedMemoryServer& server = *(InSharedMemoryServer*)lpParameter;

	while (true)
	{
		HANDLE hEvents[] = { server.hClToSvEvent, server.hShutdownEvent };
		DWORD dwResult = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);

		assert(dwResult == WAIT_OBJECT_0 || dwResult == WAIT_OBJECT_0 + 1);

		// The client to server event is signaled.
		if (dwResult == WAIT_OBJECT_0) {
			BOOL bResult = PostQueuedCompletionStatus(server.hCompPort, 0, server.compKey, NULL);
			assert(bResult);
		}

		// The shutdown event is signaled.
		if (dwResult == WAIT_OBJECT_0 + 1)
			break;
	}

	return 0;
}

InSharedMemoryServer::InSharedMemoryServer(unsigned long bufferSize, const std::string& name, const std::string& svToClEventName,
	const std::string& clToSvEventName, HANDLE hCompPort, ULONG_PTR compKey)
	: bufferSize(bufferSize), name(name), svToClEventName(svToClEventName), clToSvEventName(clToSvEventName), hCompPort(hCompPort), compKey(compKey)
{
	if (hCompPort == INVALID_HANDLE_VALUE || hCompPort == NULL)
		throw std::invalid_argument("The completion port is invalid.");

	// Initialize the shared memory.
	HANDLE hResult = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, bufferSize, name.c_str());
	if (hResult == NULL || GetLastError() == ERROR_ALREADY_EXISTS)
		throw SystemException(GetLastError());

	this->hSharedMemory = hResult;

	// Initialize the server to client auto reset event.
	hResult = CreateEventA(NULL, FALSE, FALSE, svToClEventName.c_str());
	if (hResult == NULL || GetLastError() == ERROR_ALREADY_EXISTS)
		throw SystemException(GetLastError());

	this->hSvToClEvent = hResult;

	// Initialize the client to server auto reset event.
	hResult = CreateEventA(NULL, FALSE, FALSE, clToSvEventName.c_str());
	if (hResult == NULL || GetLastError() == ERROR_ALREADY_EXISTS)
		throw SystemException(GetLastError());

	this->hClToSvEvent = hResult;

	// Initalize the shutdown event.
	hResult = CreateEventA(NULL, TRUE, FALSE, NULL);
	if (hResult == NULL)
		throw SystemException(GetLastError());

	this->hShutdownEvent = hResult;

	// Initalize and start the thread that waits for the client to server event.
	hResult = CreateThread(NULL, 0, WaiterThreadProc, this, 0, NULL);
	if (hResult == NULL)
		throw SystemException(GetLastError());

	this->hThread = hResult;

	// Set the "I'm ready to receive data" event.
	BOOL bResult = SetEvent(this->hSvToClEvent);
	if (!bResult)
		throw SystemException(GetLastError());
}

size_t InSharedMemoryServer::getData(char* buffer, size_t size)
{
	// This operation cannot be completed simultaneously.
	auto lock = this->critSection.lock();

	// Open the shared memory.
	void* lpResult = MapViewOfFile(this->hSharedMemory, FILE_MAP_READ, 0, 0, min(size, this->bufferSize));
	if (lpResult == nullptr)
		throw SystemException(GetLastError());

	const char* lpSharedMemory = (char*)lpResult;
	size_t sharedMemSize = strnlen_s(lpSharedMemory, this->bufferSize);

	// Copy the shared memory to the given buffer.
	if (sharedMemSize <= size)
		strcpy_s(buffer, size, lpSharedMemory);

	BOOL bResult = UnmapViewOfFile(lpSharedMemory);
	assert(bResult);

	// Signal the server to client event.
	bResult = SetEvent(this->hSvToClEvent);
	assert(bResult);

	if (sharedMemSize > size)
		throw std::invalid_argument("Buffer is too small.");

	return sharedMemSize;
}

InSharedMemoryServer::~InSharedMemoryServer()
{
	// If the thread was started, I must make sure I wait for it to close.
	if (this->hThread != INVALID_HANDLE_VALUE)
	{
		BOOL bResult = SetEvent(this->hShutdownEvent);
		assert(bResult);

		DWORD dwResult = WaitForSingleObject(this->hThread, 250);
		assert(dwResult == WAIT_OBJECT_0);
	}
}
