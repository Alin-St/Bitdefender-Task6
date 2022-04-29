#pragma once

#include "Utility.h"
#include <Windows.h>
#include <exception>
#include <string>

/// <summary> Exception used for Win32 errors. </summary>
class SystemException : public std::exception
{
protected:
	DWORD errCode{};
	std::string messg;

public:
	SystemException(DWORD errorCode) : errCode(errorCode), messg(getMessage(errorCode)) {}

	virtual const char* what() const noexcept override { return this->messg.c_str(); }

	/// <summary> Gets the error message using FormatMessageA() with FORMAT_MESSAGE_FROM_SYSTEM flag. </summary>
	/// <param name="dwErrorCode"> The Win32 error code you want to get the message for. </param>
	/// <returns> The string containing the requested message or an adequate message if the function fails. </returns>
	static std::string getMessage(DWORD dwErrorCode);
};
