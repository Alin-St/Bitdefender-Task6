#include "RaiiIndexedOverlapped.h"
#include "SystemException.h"
#include <cassert>

RaiiIndexedOverlapped::RaiiIndexedOverlapped(bool withEvent, size_t index)
{
	if (withEvent)
	{
		HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (hEvent == NULL)
			throw SystemException(GetLastError());

		OVERLAPPED& ol = *this;
		ol.hEvent = hEvent;
	}

	this->index() = index;
}

RaiiIndexedOverlapped::~RaiiIndexedOverlapped() noexcept
{
	OVERLAPPED& ol = *this;
	if (ol.hEvent != NULL && ol.hEvent != INVALID_HANDLE_VALUE) {
		auto res = CloseHandle(ol.hEvent);
		assert(res == TRUE);
	}

	ol = { 0 };
}

RaiiIndexedOverlapped::operator OVERLAPPED& () noexcept {
	return *(OVERLAPPED*)this->ptrOverlapped;
}

size_t& RaiiIndexedOverlapped::index() noexcept
{
	char* lpIndex = this->ptrOverlapped + sizeof(OVERLAPPED);
	return *(size_t*)lpIndex;
}

size_t& RaiiIndexedOverlapped::indexFromOverlappedLp(OVERLAPPED* lpOverlapped) noexcept
{
	char* lpIndex = (char*)lpOverlapped + sizeof(OVERLAPPED);
	return *(size_t*)lpIndex;
}
