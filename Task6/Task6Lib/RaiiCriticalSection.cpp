#include "RaiiCriticalSection.h"
#include <utility>

RaiiCriticalSection::RaiiCriticalSection() noexcept : critSec{0}
{
	InitializeCriticalSection(&this->critSec);
}

RaiiCriticalSection::~RaiiCriticalSection() noexcept
{
	DeleteCriticalSection(&this->critSec);
}

RaiiCriticalSection::operator CRITICAL_SECTION& () noexcept
{
	return this->critSec;
}

RaiiLock RaiiCriticalSection::lock() noexcept
{
	return RaiiLock(this->critSec);
}
