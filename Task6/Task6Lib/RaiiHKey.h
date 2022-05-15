#pragma once

#include <Windows.h>
#include <utility>

/// <summary> Similar to HKEY, but when the object is destroyed or assigned it closes the underlying HKEY if it is not invalid.
/// Copy constructor and assignment are not provided because only one object can have the responsability of closing a HKEY.
/// In order to close the key prematurely, you can assign an invalid HKEY to the object. Only (HKEY)0 is considered an invalid HKEY. </summary>
class RaiiHKey
{
private:
	HKEY hKey;

public:
	/// <summary> Initialize an invalid key. </summary>
	RaiiHKey() noexcept;

	RaiiHKey(HKEY _hKey) noexcept;
	RaiiHKey(const RaiiHKey&) = delete;
	RaiiHKey(RaiiHKey&& other) noexcept;

	RaiiHKey& operator=(const RaiiHKey&) = delete;
	RaiiHKey& operator=(RaiiHKey&& other) noexcept;

	~RaiiHKey() noexcept;

	operator const HKEY() const noexcept;
};
