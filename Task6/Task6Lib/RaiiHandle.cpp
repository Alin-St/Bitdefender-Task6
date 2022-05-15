#include "RaiiHandle.h"
#include <utility>
#include <cassert>

RaiiHandle::RaiiHandle() noexcept : handle(INVALID_HANDLE_VALUE) {}

RaiiHandle::RaiiHandle(HANDLE _handle) noexcept : handle(_handle) {}

RaiiHandle::RaiiHandle(RaiiHandle&& other) noexcept : RaiiHandle()
{
	*this = std::move(other);
}

RaiiHandle& RaiiHandle::operator=(RaiiHandle&& other) noexcept
{
	if (this == &other)
		return *this;

	if (this->handle != INVALID_HANDLE_VALUE) {
		BOOL bResult = CloseHandle(this->handle);
		assert(bResult);
	}

	this->handle = other.handle;
	other.handle = INVALID_HANDLE_VALUE;
	return *this;
}

RaiiHandle::~RaiiHandle() noexcept
{
	if (this->handle != INVALID_HANDLE_VALUE) {
		BOOL bResult = CloseHandle(this->handle);
		assert(bResult);
	}
}

RaiiHandle::operator const HANDLE() const noexcept {
	return this->handle;
}
