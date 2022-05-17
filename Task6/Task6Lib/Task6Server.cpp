#include "Task6Server.h"
#include "SystemException.h"
#include <stdexcept>
#include <cassert>

DWORD WINAPI WorkerThreadProc(_In_ LPVOID lpParameter)
{
	Task6Server& server = *(Task6Server*)lpParameter;

	const std::string thId = std::to_string(GetCurrentThreadId());
	server.pushMessg("Thread " + thId + " started running.");

	while (true)
	{
		DWORD bytesTransferred = 0;
		ULONG_PTR compKey = 0;
		OVERLAPPED* lpOverlapped = nullptr;

		BOOL bResult = GetQueuedCompletionStatus(server.hCompPort, &bytesTransferred, &compKey, &lpOverlapped, INFINITE);
		auto lastError = GetLastError();

		// If the iocp failed, or I received a "shutdown now" message, or the shutdown event is signaled, then exit.
		if ((!bResult && lpOverlapped == NULL) ||
			(bResult && compKey == TASK6_SHUTDOWN_COMP_KEY) ||
			(WaitForSingleObject(server.hShutdownEvent, 0) != WAIT_TIMEOUT))
		{
			if (!bResult && lpOverlapped == NULL) {
				SystemException ex(lastError);
				server.pushMessg("Iocp failed on thread " + thId + " with message: " + ex.what());
			}

			server.pushMessg("Thread " + thId + " shutdown.");
			return 0;
		}

		// Handle the iocp packet.
		try
		{
			if (compKey == TASK6_PIPE_SERVER_COMP_KEY)
			{
				// THE IOCP PACKET IS FROM THE PIPE SERVER

				using PipeSvResult = InPipeServer::AsyncResult;
				using PipeSvOp = PipeSvResult::Operation;

				PipeSvResult result;
				server.pipeServer->service(lpOverlapped, bytesTransferred, bResult ? ERROR_SUCCESS : lastError, result);

				// Log a message describing what kind of operation completed (successful / failed + err code, zombie / normal, connect / read, index).
				std::string op = (result.operation == PipeSvOp::CONNECT) ? "connect" : (result.operation == PipeSvOp::READ) ? "read" : "unknown";
				std::string ind = std::to_string(result.instanceIndex);
				std::string opType = (result.errorCode == ERROR_SUCCESS ? "successful" : "failed (" + std::to_string(result.errorCode) + ")");
				opType += (result.bZombieOperation ? " zombie" : "");

				server.pushMessg("Thread " + thId + " received a " + opType + " pipe server " + op + " operation for pipe instance " + ind + ".");
				
				// If a message was received, handle it.
				if (result.operation == PipeSvOp::READ && result.errorCode == ERROR_SUCCESS && !result.bZombieOperation)
				{
					auto& buff = server.pipeSvBuffers.at(result.instanceIndex);
					buff.at(result.bytesTransferred) = '\0';
					server.pushMessg("Thread " + thId + " received from pipe " + ind + " message: " + buff.data());
					server.handleClientMessg(buff.data());
				}

				// If the pipe is unconnected or connected but not receiving messages, I should connect it, or begin receiving messages.
				if (result.newPipeState == PipeState::UNCONNECTED) {
					server.pipeServer->connectInstance(result.instanceIndex);
					server.pushMessg("Thread " + thId + " began reconnecting pipe " + ind);
				}

				if (result.newPipeState == PipeState::CONNECTED) {
					auto& buff = server.pipeSvBuffers.at(result.instanceIndex);
					server.pipeServer->readInstance(result.instanceIndex, buff.data(), buff.size() - 1); // reserve a byte for null char
					server.pushMessg("Thread " + thId + " began reading from pipe " + ind);
				}
			}
			else if (compKey == TASK6_SHARED_MEMORY_SERVER_COMP_KEY)
			{
				// THE IOCP PACKET IS FROM THE SHARED MEMORY SERVER

				server.pushMessg("Thread " + thId + " received data to the shared memory server.");

				// That means the data is ready to be read.
				auto lock = server.shMemBufferCS.lock();
				size_t recv = server.sharedMemServer->getData(server.sharedMemoryBuffer->data(), server.sharedMemoryBuffer->size());
				server.handleClientMessg(server.sharedMemoryBuffer->data());
			}
			else
			{
				// THE IOCP PACKET HAS UNKNOWN ORIGINS
				server.pushMessg("Thread " + thId + " received an invalid iocp packet.");
				break;
			}
		}
		catch (std::exception& ex) {
			server.pushMessg("Thread " + thId + " caught an exception while managing an iocp packet: " + ex.what());
		}
	}

	return 0;
}

Task6Server::Task6Server() : pipeSvBuffers(TASK6_PIPE_SERVER_MAX_CLIENTS)
{
	// Create the iocp.
	HANDLE hResult = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, TASK6_ACTIVE_THREADS);
	if (hResult == NULL)
		throw SystemException(GetLastError());

	this->hCompPort = hResult;

	// Initialize the pipe server.
	this->pipeServer = std::make_unique<InPipeServer>(
		TASK6_PIPE_SERVER_NAME,
		TASK6_PIPE_SERVER_MAX_CLIENTS,
		this->hCompPort,
		TASK6_PIPE_SERVER_COMP_KEY,
		DATA_BUFFER_SIZE
	);

	// Initialize the shared memory server.
	this->sharedMemServer = std::make_unique<InSharedMemoryServer>(
		DATA_BUFFER_SIZE,
		TASK6_SHARED_MEMORY_NAME,
		TASK6_SHARED_MEMORY_SVTOCL_EVENT_NAME,
		TASK6_SHARED_MEMORY_CLTOSV_EVENT_NAME,
		this->hCompPort,
		TASK6_SHARED_MEMORY_SERVER_COMP_KEY
	);

	// Initialize the shared memory buffer.
	this->sharedMemoryBuffer = std::make_unique<std::array<char, DATA_BUFFER_SIZE>>();

	// Create the log semaphore.
	hResult = CreateSemaphoreA(NULL, 0, MAXLONG, NULL);
	if (hResult == NULL)
		throw SystemException(GetLastError());

	this->hLogSem = hResult;

	// Begin connecting all the pipe clients.
	for (size_t i = 0; i < this->pipeServer->pipeCount(); ++i)
		this->pipeServer->connectInstance(i);

	// Initialize the shutdown event.
	hResult = CreateEventA(NULL, TRUE, FALSE, NULL);
	if (hResult == NULL)
		throw SystemException(GetLastError());

	this->hShutdownEvent = hResult;

	// Create and start the working threads (the following code must not throw exceptions).
	this->workerThreads.reserve(TASK6_WORKING_THREADS); // to avoid potential exceptions

	for (size_t i = 0; i < TASK6_WORKING_THREADS; ++i)
	{
		hResult = CreateThread(NULL, 0, WorkerThreadProc, this, 0, NULL);

		if (hResult == NULL) {
			this->stopAndWait();
			throw SystemException(GetLastError());
		}

		// Important to add only the successful, running threads to the vector.
		this->workerThreads.emplace_back(hResult);
	}
}

void Task6Server::stopAndWait()
{
	// Set off the shutdown event.
	BOOL bResult = SetEvent(this->hShutdownEvent);
	assert(bResult);

	// If I have 0 working threads, WaitForMultipleObjects doesn't work.
	if (this->workerThreads.size() == 0)
		return;

	// Transform the RaiiHandle vector into a normal HANDLE vector.
	std::vector<HANDLE> hThreadsVec;
	for (HANDLE hThread : this->workerThreads)
		hThreadsVec.push_back(hThread);

	// Post an "exit now" message for every thread in the iocp.
	for (size_t i = 0; i < hThreadsVec.size(); ++i)
	{
		bResult = PostQueuedCompletionStatus(this->hCompPort, 0, TASK6_SHUTDOWN_COMP_KEY, NULL);
		assert(bResult);
	}

	// Wait untill all threads complete.
	DWORD dwResult = WaitForMultipleObjects((DWORD)hThreadsVec.size(), hThreadsVec.data(), TRUE, TASK6_SHUTDOWN_TIMEOUT);
	assert(dwResult >= WAIT_OBJECT_0 && dwResult < WAIT_OBJECT_0 + hThreadsVec.size());

	// Shutdown the pipe server.
	this->pipeServer = nullptr;
}

bool Task6Server::popMessg(unsigned long timeout, std::string& messg)
{
	DWORD dwResult = WaitForSingleObject(this->hLogSem, timeout);

	if (dwResult == WAIT_OBJECT_0) {
		auto lock = this->logCS.lock();
		messg = std::move(this->log.front());
		this->log.pop();
		return true;
	}
	else if (dwResult == WAIT_TIMEOUT)
		return false;
	else if (dwResult == WAIT_FAILED)
		throw SystemException(GetLastError());
	else
		throw std::runtime_error("Unexpected result from the wait function.");
}

void Task6Server::pushMessg(std::string&& messg)
{
	{
		auto lock = this->logCS.lock();
		this->log.emplace(std::move(messg));
	}

	BOOL bResult = ReleaseSemaphore(this->hLogSem, 1, NULL);
	assert(bResult);
}

void Task6Server::handleClientMessg(std::string messg)
{
	// TODO: actually handle the messg
}
