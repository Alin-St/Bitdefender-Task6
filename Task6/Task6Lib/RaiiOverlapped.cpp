#include "RaiiOverlapped.h"
#include "SystemException.h"
#include <cassert>

RaiiOverlapped::RaiiOverlapped(bool withEvent)
{
	if (withEvent)
	{
		HANDLE hEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
		if (hEvent == NULL)
			throw SystemException(GetLastError());

		this->overlapped.hEvent = hEvent;
	}
}

RaiiOverlapped::~RaiiOverlapped() noexcept
{
	if (this->overlapped.hEvent != NULL && this->overlapped.hEvent != INVALID_HANDLE_VALUE) {
		auto res = CloseHandle(this->overlapped.hEvent);
		assert(res == TRUE);
	}

	this->overlapped = { 0 };
}

RaiiOverlapped::operator OVERLAPPED& () noexcept {
	return this->overlapped;
}
