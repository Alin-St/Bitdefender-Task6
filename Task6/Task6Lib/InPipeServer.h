#pragma once

#include "RaiiHandle.h"
#include "PipeState.h"
#include "RaiiIndexedOverlapped.h"
#include "RaiiCriticalSection.h"
#include <Windows.h>
#include <string>
#include <vector>
#include <memory>

/// <summary> A pipe server represents a collection of pipe instances with the same name, that can be connected simultaneously to a number of clients.
/// All time consuming operations are handled asynchronously, using overlapped structures and a given iocp. To use the server, one must first start it,
/// start connecting to the clients, then call the 'service' method to handle completed background tasks. </summary>
class InPipeServer
{
private:
	struct PipeInstance {
		RaiiHandle hPipe = INVALID_HANDLE_VALUE;
		PipeState state = PipeState::UNCONNECTED;
		RaiiIndexedOverlapped connReadOverlapped; // Used for both connecting and reading data.
		RaiiCriticalSection critSec;

		PipeInstance(RaiiHandle&& hPipe, size_t index) : hPipe(std::move(hPipe)), state(PipeState::UNCONNECTED), connReadOverlapped(false, index), critSec() {}
	};

public:
	/// <summary> Object describing the result of an async operation. </summary>
	struct AsyncResult
	{
		/// <summary> The index of the pipe instace that completed the async operation. </summary>
		size_t instanceIndex;

		/// <summary> The operation that has completed. Only if the error code is ERROR_SUCCESS the operation completed successfully.
		/// Operations might complete even when a client is disconnecting, in which case the new pipe state might be unconnected. </summary>
		enum class Operation { CONNECT, READ } operation;

		/// <summary> If the completed operation was successful this value is ERROR_SUCCESS, otherwise it's the error code. </summary>
		DWORD errorCode;

		/// <summary> The number of bytes transferred. Valid only for a successful write operation. </summary>
		size_t bytesTransferred = 0;

		/// <summary> The new pipe state of the instance that completed the operation. </summary>
		PipeState newPipeState;
	};

private:
	const std::string name;
	const HANDLE hCompPort;
	const ULONG_PTR compKey;
	const DWORD inBufferSize;
	std::vector<std::unique_ptr<PipeInstance>> pipes;

public:
	InPipeServer() = delete;
	InPipeServer(const InPipeServer&) = delete;
	InPipeServer(InPipeServer&&) = default;
	InPipeServer& operator=(const InPipeServer&) = delete;
	InPipeServer& operator=(InPipeServer&&) = default;
	~InPipeServer() = default;

	/// <summary> Initialize a pipe server with the specified number of pipe instances. </summary>
	/// <param name="name"> The name of the underlying named pipes used. Must have the form "\\.\pipe\[any name]". </param>
	/// <param name="max_clients"> The number of underlying pipes used. </param>
	/// <param name="hCompletionPort"> A HANDLE to the iocp used by the server. Must be valid the entire lifetime of the server. </param>
	/// <param name="completionKey"> The completion key associated with this server on the iocp. </param>
	/// <param name="inBufferSize"> The input buffer size used for the underlying pipes. The out buffer will have size 0. </param>
	InPipeServer(const std::string name, size_t max_clients, HANDLE hCompletionPort, ULONG_PTR completionKey, DWORD inBufferSize = 65536);

	/// <summary> Get the name used to create the pipe instances. </summary>
	const std::string& getName() const noexcept;

	/// <summary> Get a handle to the iocp associated with the pipe instances. </summary>
	HANDLE getIocpHandle() const noexcept;

	/// <summary> Get the completion key associated with the server on the iocp. </summary>
	ULONG_PTR getCompKey() const noexcept;

	/// <summary> Get the input buffer size used for the underlying pipes. Note: the output buffers have size 0. </summary>
	DWORD getInBufferSize() const noexcept;

	/// <summary> Get the number of pipe instances used by the server (connected or unconnected). </summary>
	size_t pipeCount() const noexcept;

	/// <summary> Get the state of the specified pipe. </summary>
	/// <param name="index"> The index of the pipe instance you want the status for. Must be between 0 and pipeCount() -1. </param>
	PipeState getPipeState(size_t index) const;

	/// <summary> Enables the specified pipe instance to wait for a client to connect. The operation is saved on the iocp. </summary>
	/// <param name="index"> The index of the unconnected pipe instance you want to connect. Must be between 0 and pipeCount() - 1. </param>
	void connectInstance(size_t index);

	/// <summary> Reads the data from the specified pipe instance. The operation is saved on the iocp. </summary>
	/// <param name="index"> The index of the connected, non-reading pipe instance you want to connect. Must be between 0 and pipeCount() - 1. </param>
	/// <param name="buffer"> A pointer to a buffer that receives the data; must be valid until the operation is completed. </param>
	/// <param name="size"> The maximum number of bytes to read. </param>
	void readInstance(size_t index, void* buffer, size_t size);

	/// <summary> Handle a given completed operation on the server. You should use the completion key to make sure the
	/// completed iocp operation belongs to this server. </summary>
	/// <param name="overlapped"> The OVERLAPPED structure returned by GetQueuedCompletionStatus from the iocp (cannot be null). </param>
	/// <param name="bytesTransferred"> The number of bytes transferred returned along with the overlapped structure. </param>
	/// <param name="errorCode"> If GetQueuedCompletionStatus failed, the error code, otherwise ERROR_SUCCESS. </param>
	/// <param name="result"> An object describing the completed operation (out parameter). </param>
	void service(OVERLAPPED* overlapped, DWORD bytesTransferred, DWORD errorCode, AsyncResult& result);

	/// <summary> Disconnects the specified pipe from the client. </summary>
	/// <param name="index"> The index of the connected pipe instance you want to disconnect. Must be between 0 and pipeCount() - 1. </param>
	void disconnectInstance(size_t index);
};
