#pragma once

#include "RaiiHandle.h"
#include "PipeState.h"
#include "RaiiOverlapped.h"
#include "RaiiCriticalSection.h"
#include <string>

class OutPipeClient
{
public:
	/// <summary> Object describing the result of an async operation. </summary>
	struct AsyncResult
	{
		/// <summary> The operation that has completed. Only if the error code is ERROR_SUCCESS the operation completed successfully. </summary>
		enum class Operation { WRITE } operation;

		/// <summary> If the completed operation was successful this value is ERROR_SUCCESS, otherwise it's the error code. </summary>
		DWORD errorCode;

		/// <summary> The number of bytes transferred. Valid only for a successful write operation. </summary>
		size_t bytesTransferred = 0;
	};

private:
	std::string serverName;
	RaiiHandle hPipe = INVALID_HANDLE_VALUE;
	PipeState state = PipeState::UNCONNECTED;
	RaiiOverlapped writeOverlapped;
	RaiiCriticalSection critSec;

public:
	OutPipeClient();
	OutPipeClient(const OutPipeClient&) = delete;
	OutPipeClient(OutPipeClient&&) = delete;
	OutPipeClient& operator=(const OutPipeClient&) = delete;
	OutPipeClient& operator=(OutPipeClient&&) = delete;
	~OutPipeClient() = default;

	/// <summary> If the client is connecting, or connected, get the name of the server. </summary>
	const std::string& getServerName();

	/// <summary> Get the connection and writing state of the client. </summary>
	PipeState getState();

	/// <summary> Get the handle of an event that is signaled when the write operation completed. </summary>
	HANDLE getWaitHandle();

	/// <summary> Connect to a named pipe server. </summary>
	/// <param name="pipeName"> The name of the server. Must have the form "\\.\pipe\[any name]". </param>
	void open(const std::string& pipeName);

	/// <summary> Begin writing the given data to the pipe server. </summary>
	/// <param name="buffer"> The data you want to send. Don't modify this buffer until the write operation is completed. </param>
	/// <param name="size"> The number of bytes to write. Check the async result's bytes transferred to see how many bytes were written. </param>
	void write(void* buffer, size_t size);

	/// <summary> If the client is connected, flush the write buffer to make sure all data is sent.
	/// This function locks the client until the data is flushed. </summary>
	void flushWriteBuffer();

	/// <summary> Get an object describing the last completed async operation. </summary>
	/// <param name="wait"> Wait until a pending operation completes. Value ignored if there are no pending operations. </param>
	/// <param name="result"> If the return value is true, this object describes the completed operation. (out param) </param>
	/// <returns> If a pending operation completed (successfully or with error) returns true, otherwise false. </returns>
	bool service(bool wait, AsyncResult& result);

	/// <summary> Close the connection to the server. The client can be reconnected after this call. </summary>
	void close();
};
