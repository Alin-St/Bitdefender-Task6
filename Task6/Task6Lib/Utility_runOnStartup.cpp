#include "Utility.h"
#include "RaiiHKey.h"
#include "SystemException.h"
#include <Windows.h>
#include <stdexcept>
#include <memory>

constexpr int MAX_COMMAND_LEN = 260; // 260 is the maximum length for a value under Run and RunOnce registry key.

/// <summary> Tries to open the Run or RunOnce registry key. </summary>
/// <param name="currentUser"> Use base key 'HKEY_CURRENT_USER' or 'HKEY_LOCAL_MACHINE'? </param>
/// <param name="openRunOnce"> Open '..\Software\Microsoft\Windows\CurrentVersion\Run' or '..\Software\Microsoft\Windows\CurrentVersion\RunOnce'? </param>
/// <returns> The handler to the opened key. </returns>
RaiiHKey openRunOrRunOnceRegistryKey(bool currentUser, bool openRunOnce)
{
	const HKEY hKey = currentUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
	const wchar_t* lpSubKey = openRunOnce ?
		L"Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce" :
		L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";

	HKEY phkResult{};
	LSTATUS lStatus = RegOpenKeyExW(hKey, lpSubKey, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &phkResult);

	if (lStatus != ERROR_SUCCESS)
		throw SystemException((DWORD)lStatus);

	// This object makes sure the key handle is closed.
	return RaiiHKey(phkResult);
}

bool Utility::runProgramOnStartup(const wchar_t* appName, const wchar_t* appPath, bool currentUser, bool onlyOnce, bool* pathMatches)
{
	if (appName == nullptr || wcscmp(appName, L"") == 0 || appPath == nullptr || wcscmp(appPath, L"") == 0 || wcslen(appPath) + 1 > MAX_COMMAND_LEN)
		throw std::invalid_argument("The application name or the application path are invalid.");

	// First try to open the registry key that makes a program run when the user logs on.
	const RaiiHKey hKey = openRunOrRunOnceRegistryKey(currentUser, onlyOnce);

	// Then check if a value with the program name is already there and is a string.
	DWORD dwType{}, dwDataSize{};
	LSTATUS lStatus = RegQueryValueExW(hKey, appName, NULL, &dwType, NULL, &dwDataSize);

	if (lStatus != ERROR_SUCCESS && lStatus != ERROR_FILE_NOT_FOUND)
		throw SystemException((DWORD)lStatus);

	// If I found a value with the same name, don't modify the register, otherwise add a new value to the key.
	if (lStatus == ERROR_SUCCESS)
	{
		// If requested, check if the found path matches the given one.
		if (pathMatches != nullptr) {
			size_t dataLen = dwDataSize / sizeof(wchar_t) - 1;

			if (dwType != REG_SZ || dataLen < 1 || dataLen + 1 > MAX_COMMAND_LEN)
				*pathMatches = false;
			else {
				dwType = 0;
				auto lpData = std::make_unique<wchar_t[]>(dataLen + 1);
				lStatus = RegGetValueW(hKey, NULL, appName, RRF_RT_REG_SZ, &dwType, lpData.get(), &dwDataSize);

				if (lStatus != ERROR_SUCCESS)
					throw SystemException((DWORD)lStatus);

				if (dwType != REG_SZ || wcscmp(appPath, lpData.get()) != 0)
					*pathMatches = false;
				else
					*pathMatches = true;
			}
		}

		return false;
	}
	else {
		DWORD cbData = (DWORD)((wcslen(appPath) + 1) * sizeof(wchar_t));
		lStatus = RegSetValueExW(hKey, appName, 0, REG_SZ, (const BYTE*)appPath, cbData);

		if (lStatus != ERROR_SUCCESS)
			throw SystemException((DWORD)lStatus);

		return true;
	}
}

bool Utility::stopRunningProgramOnStartup(const wchar_t* appName, const wchar_t* appPath, bool currentUser, bool stopOneTimeRun)
{
	if (appName == nullptr || wcscmp(appName, L"") == 0 || (appPath != nullptr && (wcscmp(appPath, L"") == 0 || wcslen(appPath) + 1 > MAX_COMMAND_LEN)))
		throw std::invalid_argument("The application name or the application path are invalid.");

	// First try to open the registry key that makes a program run when the user logs on.
	const RaiiHKey hKey = openRunOrRunOnceRegistryKey(currentUser, stopOneTimeRun);

	// Then check if a value with the given program name is already there.
	DWORD dwType{}, dwDataSize{};
	LSTATUS lStatus = RegQueryValueExW(hKey, appName, NULL, &dwType, NULL, &dwDataSize);

	if (lStatus != ERROR_SUCCESS && lStatus != ERROR_FILE_NOT_FOUND)
		throw SystemException((DWORD)lStatus);

	// If a value with the given program name is already there it must be removed.
	if (lStatus == ERROR_SUCCESS)
	{
		// If the application path provided is not null, throw exception if the value doesn't refer to the same path.
		if (appPath != nullptr) {
			size_t dataLen = dwDataSize / sizeof(wchar_t) - 1;

			if (dwType != REG_SZ || dataLen < 1 || dataLen + 1 > MAX_COMMAND_LEN)
				throw std::runtime_error("Invalid value with the same name under the registry key.");

			dwType = 0;
			auto lpData = std::make_unique<wchar_t[]>(dataLen + 1);
			lStatus = RegGetValueW(hKey, NULL, appName, RRF_RT_REG_SZ, &dwType, lpData.get(), &dwDataSize);

			if (lStatus != ERROR_SUCCESS)
				throw SystemException((DWORD)lStatus);

			if (dwType != REG_SZ || wcscmp(appPath, lpData.get()) != 0)
				throw std::runtime_error("Invalid value with the same name under the registry key.");
		}

		// Delete the registry value.
		lStatus = RegDeleteValueW(hKey, appName);

		if (lStatus != ERROR_SUCCESS)
			throw SystemException((DWORD)lStatus);

		return true;
	}

	// If no value with the given name is there return false.
	return false;
}
