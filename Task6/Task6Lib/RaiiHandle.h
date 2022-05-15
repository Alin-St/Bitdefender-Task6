#pragma once

#include <Windows.h>

/// <summary> Similar to HANDLE, but when the object is destroyed or assigned it closes the underlying HANDLE if it is not invalid.
/// Copy constructor and assignment are not provided because only one object can have the responsability of closing a HANDLE.
/// In order to close the handle prematurely, you can assign an invalid handle to the object. Only INVALID_HANDLE_VALUE is considered an invalid HANDLE. </summary>
class RaiiHandle
{
private:
	HANDLE handle;

public:
	/// <summary> Initialize an invalid handle. </summary>
	RaiiHandle() noexcept;

	RaiiHandle(HANDLE _handle) noexcept;
	RaiiHandle(const RaiiHandle&) = delete;
	RaiiHandle(RaiiHandle&& other) noexcept;

	RaiiHandle& operator=(const RaiiHandle&) = delete;
	RaiiHandle& operator=(RaiiHandle&& other) noexcept;

	~RaiiHandle() noexcept;

	operator const HANDLE() const noexcept;
};
