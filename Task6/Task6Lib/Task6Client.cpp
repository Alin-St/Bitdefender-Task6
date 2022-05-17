#include "Task6Client.h"
#include <stdexcept>

void Task6Client::startPipe()
{
	if (this->pipeClient != nullptr || this->sharedMemClient != nullptr)
		throw std::runtime_error("The client has already started.");

	this->pipeClient = std::make_unique<OutPipeClient>();
	this->pipeClient->open(TASK6_PIPE_SERVER_NAME);

	char* buff; size_t size;
	this->getDataToSend(buff, size);
	this->pipeClient->write(buff, size);

	this->pushMessg("Client has started as a pipe client.");
}

void Task6Client::startSharedMemory()
{
	if (this->pipeClient != nullptr || this->sharedMemClient != nullptr)
		throw std::runtime_error("The client has already started.");

	this->sharedMemClient = std::make_unique<OutSharedMemoryClient>(
		TASK6_SHARED_MEMORY_NAME,
		TASK6_SHARED_MEMORY_SVTOCL_EVENT_NAME,
		TASK6_SHARED_MEMORY_CLTOSV_EVENT_NAME
	);

	this->pushMessg("Client has started as a shared memory client.");
}

void Task6Client::stop()
{
	this->pipeClient = nullptr;
}

bool Task6Client::wait(unsigned long timeout)
{
	if (this->pipeClient != nullptr)
	{
		// THE CLIENT IS A PIPE CLIENT

		// Wait for the pipe client operation to complete.
		HANDLE hWait = this->pipeClient->getWaitHandle();
		DWORD dwResult = WaitForSingleObject(hWait, timeout);
		if (dwResult != WAIT_OBJECT_0)
			return false;

		// Get the async result.
		using PipeClResult = OutPipeClient::AsyncResult;
		using PipeClOp = OutPipeClient::AsyncResult::Operation;

		PipeClResult result;
		if (!this->pipeClient->service(false, result))
			throw std::runtime_error("service method should be successful");

		// Log messages about the async result.
		std::string op = (result.operation == PipeClOp::WRITE) ? "write" : "unknown";
		std::string opType = (result.errorCode == ERROR_SUCCESS) ? "successful" : "failed (" + std::to_string(result.errorCode) + ")";
		std::string bytesTr = (result.operation == PipeClOp::WRITE && result.errorCode == ERROR_SUCCESS)
			? " with " + std::to_string(result.bytesTransferred) + " bytes transferred" : "";
		this->pushMessg("The " + opType + " " + op + " operation completed" + bytesTr + ".");

		return true;
	}
	else if (this->sharedMemClient != nullptr)
	{
		// THE CLIENT IS A SHARED MEMORY CLIENT.

		if (!this->bDataSent)
		{
			char* buff; size_t size;
			this->getDataToSend(buff, size);
			bool succ = this->sharedMemClient->writeData(buff, size, timeout);

			if (succ) {
				this->bDataSent = true;
				this->pushMessg("Client sent the data to the shared memorey server.");
			}
			else
				this->pushMessg("Timeout while trying to send the data to the shared memory server.");

			return succ;
		}
		else
		{
			Sleep(timeout);
			return false;
		}
	}
	else
		throw std::runtime_error("Client not started.");
}

bool Task6Client::popMessg(std::string& messg)
{
	if (this->log.empty())
		return false;

	messg = std::move(this->log.front());
	this->log.pop();
	return true;
}

void Task6Client::pushMessg(std::string&& messg)
{
	this->log.emplace(std::move(messg));
}

void Task6Client::getDataToSend(char*& buffer, size_t& size)
{
	if (this->dataBuff == nullptr)
	{
		this->dataBuff = std::make_unique<std::array<char, DATA_BUFFER_SIZE>>();
		strcpy_s(this->dataBuff->data(), this->dataBuff->size(), "Hello world!");
	}

	buffer = this->dataBuff->data();
	size = strlen(buffer);
}
