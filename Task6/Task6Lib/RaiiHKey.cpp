#include "RaiiHKey.h"
#include <cassert>

RaiiHKey::RaiiHKey() noexcept : hKey((HKEY)0) {}

RaiiHKey::RaiiHKey(HKEY _hKey) noexcept : hKey(_hKey) {}

RaiiHKey::RaiiHKey(RaiiHKey&& other) noexcept
{
	*this = std::move(other);
}

RaiiHKey& RaiiHKey::operator=(RaiiHKey&& other) noexcept
{
	if (this == &other)
		return *this;

	if (this->hKey != (HKEY)0) {
		auto res = RegCloseKey(this->hKey);
		assert(res == ERROR_SUCCESS);
	}

	this->hKey = other.hKey;
	other.hKey = (HKEY)0;
	return *this;
}

RaiiHKey::~RaiiHKey() noexcept
{
	if (this->hKey != (HKEY)0) {
		auto res = RegCloseKey(this->hKey);
		assert(res == ERROR_SUCCESS);
	}
}

RaiiHKey::operator const HKEY() const noexcept {
	return this->hKey;
}
