#pragma once

#include <Windows.h>
#include <utility>

/// <summary> Similar to HKEY, but when the object is destroyed it calls RegCloseKey() on the underlying HKEY if was not moved.
/// Copy constructor and assignment are not provided because only one object can have the responsability of destroying a HKEY. </summary>
class RaiiHKey
{
private:
	HKEY hKey;

public:
	RaiiHKey(HKEY hKey) noexcept : hKey(hKey) {}
	RaiiHKey(const RaiiHKey&) = delete;
	RaiiHKey(RaiiHKey&& other) noexcept { *this = std::move(other); }

	RaiiHKey& operator=(const RaiiHKey&) = delete;
	RaiiHKey& operator=(RaiiHKey&& other) noexcept {
		this->hKey = other.hKey;
		other.hKey = (HKEY)0;
		return *this;
	}

	~RaiiHKey() noexcept {
		if (hKey != (HKEY)0)
			RegCloseKey(this->hKey);
	}

	operator const HKEY() const noexcept { return hKey; }
};
