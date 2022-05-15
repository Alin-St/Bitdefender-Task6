#pragma once

#include "RaiiLock.h"
#include <Windows.h>

/// <summary> Simlar to a CRITICAL_SECTION, but it is initialized in the constructor and deleted in the destructor.
/// Copy or move constructor and assignment are not provided. </summary>
class RaiiCriticalSection
{
private:
	CRITICAL_SECTION critSec;

public:
	/// <summary> Initialize the underlying CRITICAL_SECTION. </summary>
	RaiiCriticalSection() noexcept;

	RaiiCriticalSection(const RaiiCriticalSection&) = delete;
	RaiiCriticalSection(RaiiCriticalSection&&) = delete;
	RaiiCriticalSection& operator=(const RaiiCriticalSection&) = delete;
	RaiiCriticalSection& operator=(RaiiCriticalSection&&) = delete;

	~RaiiCriticalSection() noexcept;

	operator CRITICAL_SECTION& () noexcept;

	/// <summary> Enters the critical section and returns an object that will leave it when destroyed. </summary>
	RaiiLock lock() noexcept;
};
