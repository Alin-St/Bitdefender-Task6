#include "InPipeServer.h"
#include "SystemException.h"
#include <stdexcept>

InPipeServer::InPipeServer(const std::string name, size_t max_clients, HANDLE hCompletionPort, ULONG_PTR completionKey, DWORD inBufferSize)
	: name(name), hCompPort(hCompletionPort), compKey(completionKey), pipes(), inBufferSize(inBufferSize)
{
	if (max_clients < 1)
		throw std::invalid_argument("max_clients must be greater than 0.");

	if (hCompletionPort == INVALID_HANDLE_VALUE)
		throw std::invalid_argument("The completion port handle must be valid.");

	// Create the underlying pipe instances and their OVERLAPPED structures, then associate the pipe HANDLEs with the iocp.
	for (size_t i = 0; i < max_clients; ++i)
	{
		DWORD first_pipe_instance = (i == 0 ? FILE_FLAG_FIRST_PIPE_INSTANCE : 0);

		HANDLE hPipe = CreateNamedPipeA(
			name.c_str(),
			PIPE_ACCESS_INBOUND | first_pipe_instance | FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
			PIPE_UNLIMITED_INSTANCES,
			0,
			inBufferSize,
			0,
			nullptr
		);

		if (hPipe == INVALID_HANDLE_VALUE)
			throw SystemException(GetLastError());

		// Initialize the OVERLAPPED structure and CRITICAL_SECTION in the constructor of PipeInstace.
		auto pipeInst = std::make_unique<PipeInstance>(hPipe, i);

		// Associate the pipe HANDLE with the iocp, using the given completion key associated with the server.
		if (CreateIoCompletionPort(hPipe, hCompletionPort, completionKey, 0) != hCompletionPort)
			throw SystemException(GetLastError());

		this->pipes.emplace_back(std::move(pipeInst));
	}
}

const std::string& InPipeServer::getName() const noexcept
{
	return this->name;
}

HANDLE InPipeServer::getIocpHandle() const noexcept
{
	return this->hCompPort;
}

ULONG_PTR InPipeServer::getCompKey() const noexcept
{
	return this->compKey;
}

DWORD InPipeServer::getInBufferSize() const noexcept
{
	return this->inBufferSize;
}

size_t InPipeServer::pipeCount() const noexcept
{
	return this->pipes.size();
}

PipeState InPipeServer::getPipeState(size_t index) const
{
	auto& pipe = this->pipes.at(index).operator*();

	// The following block of code is synchronized by the pipe's CRITICAL_SECTION.
	{
		auto pipeLock = pipe.critSec.lock();
		return pipe.state;
	}
}


void InPipeServer::connectInstance(size_t index)
{
	auto& pipe = this->pipes.at(index).operator*();

	// The following block of code is synchronized by the pipe's CRITICAL_SECTION.
	{
		auto pipeLock = pipe.critSec.lock();

		if (pipe.state != PipeState::UNCONNECTED)
			throw std::logic_error("The selected pipe must be unconnected before you try to connect it.");

		// Begin the async operation of connecting to a client.
		OVERLAPPED& connReadOl = pipe.connReadOverlapped;
		BOOL bResult = ConnectNamedPipe(pipe.hPipe, &connReadOl);
		auto lastError = GetLastError();

		if (bResult || (lastError != ERROR_IO_PENDING && lastError != ERROR_PIPE_CONNECTED))
			throw SystemException(lastError);

		pipe.state = PipeState::_UNCONNECTED_CONNECTING;

		// Even if the operation completed non-blocking, it will still be added to the iocp.
	}
}

void InPipeServer::readInstance(size_t index, void* buffer, size_t size)
{
	auto& pipe = this->pipes.at(index).operator*();

	// The following block of code is synchronized by the pipe's CRITICAL_SECTION.
	{
		auto pipeLock = pipe.critSec.lock();

		bool bConnected = (int)pipe.state & (int)PipeState::CONNECTED;
		bool bReading = (int)pipe.state & (int)PipeState::FLAG_READING;
		if (!bConnected || bReading)
			throw std::logic_error("The selected pipe must be connected and not reading when you try to begin reading from it.");

		// Begin the async operation of reading from a client.
		OVERLAPPED& connReadOl = pipe.connReadOverlapped;
		BOOL bResult = ReadFile(pipe.hPipe, buffer, (DWORD)size, NULL, &connReadOl);
		auto lastError = GetLastError();

		if (!bResult && lastError != ERROR_IO_PENDING)
			throw SystemException(lastError);

		pipe.state = (PipeState)((int)pipe.state | (int)PipeState::FLAG_READING);

		// Even if the operation completed non-blocking, it will still be added to the iocp.
	}
}

void InPipeServer::service(OVERLAPPED* lpOverlapped, DWORD bytesTransferred, DWORD errorCode, AsyncResult& result)
{
	if (lpOverlapped == nullptr)
		throw std::invalid_argument("'lpOverlapped' cannot be null.");

	// Identify the pipe whose completion succeeded.
	size_t index = RaiiIndexedOverlapped::indexFromOverlappedLp(lpOverlapped);
	auto& pipe = this->pipes.at(index).operator*();
	OVERLAPPED& connReadOl = pipe.connReadOverlapped;

	if (lpOverlapped != &connReadOl)
		throw std::invalid_argument("The given overlapped structure is not valid.");

	// The following code is synchronized by the pipe's CRITICAL_SECTION.
	auto pipeLock = pipe.critSec.lock();

	// There are four cases in which I expect an async operation to complete.
	switch (pipe.state)
	{
	case PipeState::_UNCONNECTED_CONNECTING:
		pipe.state = PipeState::CONNECTED;
		result = { index, AsyncResult::Operation::CONNECT, errorCode, 0, pipe.state };
		break;

	case PipeState::_CONNECTED_READING:
		pipe.state = PipeState::CONNECTED;
		result = { index, AsyncResult::Operation::READ, errorCode, bytesTransferred, pipe.state };
		break;

	case PipeState::_DISCONNECTING_CONNECTING:
		// If the pipe successfully connected before I canceled the connection, I need to disconnect it.
		if (errorCode == ERROR_SUCCESS) {
			BOOL bResult = DisconnectNamedPipe(pipe.hPipe);
			throw std::runtime_error("Cannot disconnect pipe.");
		}

		pipe.state = PipeState::UNCONNECTED;
		result = { index, AsyncResult::Operation::READ, errorCode, 0, pipe.state };
		break;

	case PipeState::_DISCONNECTING_READING:
		pipe.state = PipeState::UNCONNECTED;
		result = { index, AsyncResult::Operation::READ, errorCode, bytesTransferred, pipe.state };
		break;

	default:
		throw std::runtime_error("Invalid pipe state.");
	}
}

void InPipeServer::disconnectInstance(size_t index)
{
	auto& pipe = this->pipes.at(index).operator*();

	// The following code is synchronized by the pipe's CRITICAL_SECTION.
	auto pipeLock = pipe.critSec.lock();
		
	bool bConnected = (int)pipe.state & (int)PipeState::CONNECTED;

	if (bConnected)
	{
		// If the pipe is connected, call DisconnectNamedPipe.
		BOOL bResult = DisconnectNamedPipe(pipe.hPipe);
		if (!bResult)
			throw SystemException(GetLastError());
	}
	else if (pipe.state == PipeState::_UNCONNECTED_CONNECTING)
	{
		// If the pipe is connecting, cancel the operation. If the pipe has already connected, it will be disconnected in service.
		OVERLAPPED& connReadOl = pipe.connReadOverlapped;
		BOOL bResult = CancelIoEx(pipe.hPipe, &connReadOl);

		if (!bResult)
			throw SystemException(GetLastError());
	}
	else
		throw std::logic_error("The specified pipe is not connected or connecting.");

	// If the pipe still has pending I/O operations, mark it as disconnecting.
	int allFlags = (int)PipeState::FLAG_CONNECTING | (int)PipeState::FLAG_READING | (int)PipeState::FLAG_WRITING;
	int pipeFlags = (int)pipe.state & allFlags;

	if (pipeFlags)
		pipe.state = (PipeState)((int)PipeState::DISCONNECTING | pipeFlags);
	else
		pipe.state = PipeState::UNCONNECTED;
}
