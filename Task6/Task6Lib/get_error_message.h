#pragma once

#include <Windows.h>
#include <string>

/// <summary> Gets the error message using FormatMessageA() with FORMAT_MESSAGE_FROM_SYSTEM flag. </summary>
/// <param name="dwMessageId"> The Win32 error code you want to get the message for. </param>
/// <returns> The string containing the requested message or an adequate message if the function fails. </returns>
std::string getSystemErrorMessage(DWORD dwMessageId);
