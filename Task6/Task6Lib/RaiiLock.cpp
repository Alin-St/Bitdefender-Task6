#include "RaiiLock.h"
#include <utility>

RaiiLock::RaiiLock(CRITICAL_SECTION& criticalSection) noexcept : critSec(&criticalSection)
{
	if (this->critSec != nullptr)
		EnterCriticalSection(this->critSec);
}

RaiiLock::RaiiLock(RaiiLock&& other) noexcept : RaiiLock()
{
	*this = std::move(other);
}

RaiiLock& RaiiLock::operator=(RaiiLock&& other) noexcept
{
	if (this == &other)
		return *this;

	other.critSec = this->critSec;
	this->critSec = nullptr;
	return *this;
}

RaiiLock::~RaiiLock() noexcept
{
	if (this->critSec != nullptr)
		LeaveCriticalSection(this->critSec);
}
