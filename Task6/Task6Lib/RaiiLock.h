#pragma once

#include <Windows.h>

/// <summary> Objects of this class are resposible for entering a critical section when constructed and leave it when destroyed. </summary>
class RaiiLock
{
private:
	CRITICAL_SECTION* critSec = nullptr;

public:
	RaiiLock() = default;
	RaiiLock(const RaiiLock&) = delete;
	RaiiLock(RaiiLock&& other) noexcept;
	RaiiLock& operator=(const RaiiLock&) = delete;
	RaiiLock& operator=(RaiiLock&& other) noexcept;

	/// <summary> Enter the given critical section and create an object which will leave it when it's destroyed. </summary>
	RaiiLock(CRITICAL_SECTION& criticalSection) noexcept;
	~RaiiLock() noexcept;
};
