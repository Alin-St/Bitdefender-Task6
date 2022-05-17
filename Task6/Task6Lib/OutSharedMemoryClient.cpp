#include "OutSharedMemoryClient.h"
#include "SystemException.h"
#include <cassert>

OutSharedMemoryClient::OutSharedMemoryClient(const std::string& serverName, const std::string& svToClEventName, const std::string& clToSvEventName)
	: serverName(serverName), svToClEventName(svToClEventName), clToSvEventName(clToSvEventName)
{
	// Open the shared memory.
	HANDLE hResult = OpenFileMappingA(FILE_MAP_WRITE, FALSE, serverName.c_str());
	if (hResult == NULL)
		throw SystemException(GetLastError());

	this->hSharedMemory = hResult;

	// Open the server to client event.
	hResult = OpenEventA(SYNCHRONIZE, FALSE, svToClEventName.c_str());
	if (hResult == NULL)
		throw SystemException(GetLastError());

	this->hSvToClEvent = hResult;

	// Open the client to server event.
	hResult = OpenEventA(EVENT_MODIFY_STATE, FALSE, clToSvEventName.c_str());
	if (hResult == NULL)
		throw SystemException(GetLastError());

	this->hClToSvEvent = hResult;
}

bool OutSharedMemoryClient::writeData(char* buffer, size_t size, unsigned long timeout)
{
	// Wait until the server is ready for a client.
	DWORD dwResult = WaitForSingleObject(this->hSvToClEvent, timeout);

	if (dwResult == WAIT_TIMEOUT)
		return false;

	if (dwResult != WAIT_OBJECT_0)
		throw SystemException(GetLastError());

	// Get a pointer to the shared memory.
	LPVOID lpResult = MapViewOfFile(this->hSharedMemory, FILE_MAP_WRITE, 0, 0, (SIZE_T)size);
	assert(lpResult != NULL);

	// Write the data.
	memcpy(lpResult, buffer, size);

	BOOL bResult = UnmapViewOfFile(lpResult);
	assert(bResult);

	// Tell the server you are ready.
	bResult = SetEvent(this->hClToSvEvent);
	assert(bResult);

	return true;
}
