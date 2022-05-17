#include "OutPipeClient.h"
#include "SystemException.h"
#include <Windows.h>
#include <stdexcept>
#include <cassert>

OutPipeClient::OutPipeClient() : writeOverlapped(true), critSec() {}

const std::string& OutPipeClient::getServerName()
{
	auto lock = this->critSec.lock();

	if (this->state == PipeState::UNCONNECTED)
		throw std::logic_error("Cannot get the server name. The client is unconnected.");

	return this->serverName;
}

PipeState OutPipeClient::getState()
{
	auto lock = this->critSec.lock();
	return this->state;
}

HANDLE OutPipeClient::getWaitHandle()
{
	OVERLAPPED& ol = this->writeOverlapped;
	return ol.hEvent;
}

void OutPipeClient::open(const std::string& pipeName)
{
	const std::string prefix = "\\\\.\\pipe\\";
	if (pipeName.substr(0, prefix.size()) != prefix)
		throw std::invalid_argument("The pipe server name has an invalid prefix.");

	auto lock = this->critSec.lock();

	if (this->state != PipeState::UNCONNECTED)
		throw std::logic_error("The client must be unconnected before connecting to a new server.");

	// Connect to the pipe server.
	HANDLE hPipe = CreateFileA(
		pipeName.c_str(),
		GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		NULL
	);

	if (hPipe == INVALID_HANDLE_VALUE)
		throw SystemException(GetLastError());

	this->hPipe = hPipe;
	this->state = PipeState::CONNECTED;
}

void OutPipeClient::write(void* buffer, size_t size)
{
	auto lock = this->critSec.lock();

	bool bConnected = (int)this->state & (int)PipeState::CONNECTED;
	bool bWriting = (int)this->state & (int)PipeState::FLAG_WRITING;
	if (!bConnected || bWriting)
		throw std::logic_error("The client must be connected and not writing when you try writing to the server.");

	// Begin the async operation of writing to the server.
	OVERLAPPED& ol = this->writeOverlapped;
	BOOL bResult = WriteFile(this->hPipe, buffer, (DWORD)size, NULL, &ol);
	auto lastError = GetLastError();

	if (!bResult && lastError != ERROR_IO_PENDING)
		throw SystemException(lastError);

	this->state = (PipeState)((int)this->state | (int)PipeState::FLAG_WRITING);
}

void OutPipeClient::flushWriteBuffer()
{
	auto lock = this->critSec.lock();

	bool bConnected = (int)this->state & (int)PipeState::CONNECTED;
	if (!bConnected)
		throw std::logic_error("Cannot flush buffers unless the client is connected.");

	BOOL bResult = FlushFileBuffers(this->hPipe);

	if (!bResult)
		throw SystemException(GetLastError());
}

bool OutPipeClient::service(bool wait, AsyncResult& result)
{
	auto lock = this->critSec.lock();

	// If the client is not connected and writing, there is no pending operation (return false).
	bool bConnected = (int)this->state | (int)PipeState::CONNECTED;
	bool bWriting = (int)this->state | (int)PipeState::FLAG_WRITING;
	if (!bConnected || !bWriting)
		return false;

	// Get the result of the last write operation.
	OVERLAPPED& ol = this->writeOverlapped;
	DWORD bytesTransferred = 0;
	BOOL bResult = GetOverlappedResult(this->hPipe, &ol, &bytesTransferred, (BOOL)wait);
	auto lastError = GetLastError();
	bool bPending = (bResult || lastError != ERROR_IO_PENDING);

	// If GetOverlappedResult failed because the operation is pending, return false.
	// If GetOverlappedResult succeeded and the write operation completed successfully, return true and an appropriate async result.
	// If GetOverlappedResult failed and the operation completed (the event is signaled), return an appropriate async result with the error.
	if (!bPending)
		return false;

	// The event should be signaled if the operation is not pending.
	if (bPending)
	{
		DWORD dwResult = WaitForSingleObject(ol.hEvent, 0);
		assert(dwResult == WAIT_OBJECT_0);
	}

	if (bResult) // the operation completed successfully
		result = { AsyncResult::Operation::WRITE, ERROR_SUCCESS, (size_t)bytesTransferred };
	else // the operation completed with error
		result = { AsyncResult::Operation::WRITE, lastError, 0 };

	// If the operation completed, remove the writing flag, reset the event, then return true with an appropriate async result.
	this->state = (PipeState)((int)this->state & ~(int)PipeState::FLAG_WRITING);
	BOOL bEventReset = ResetEvent(ol.hEvent);
	assert(bEventReset);
	return true;
}

void OutPipeClient::close()
{
	auto lock = this->critSec.lock();

	if (this->state == PipeState::UNCONNECTED)
		return;

	// Delete server name, close the pipe handle, set state to UNCONNECTED, reset overlapped event.
	this->serverName = "";
	this->hPipe = INVALID_HANDLE_VALUE;
	this->state = PipeState::UNCONNECTED;

	OVERLAPPED& ol = this->writeOverlapped;
	BOOL bResult = ResetEvent(ol.hEvent);
	assert(bResult);
}
