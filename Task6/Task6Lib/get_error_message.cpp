#include "get_error_message.h"

std::string getSystemErrorMessage(DWORD dwMessageId)
{
	// Try to get the system error message for the given code. Some error message expect some arguments for
	// formatting so it is required to use FORMAT_MESSAGE_IGNORE_INSERTS for unknown errors.
	// By using dwLanguageId = 0, the language is automatically determined.

	LPSTR lpBuffer{};
	DWORD dwResult = FormatMessageA(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,	// lpSource
		dwMessageId,
		0,		// dwLanguageId
		(LPSTR) &lpBuffer,
		0,		// nSize
		NULL	// Arguments
	);

	std::string result;

	if (dwResult == 0)
		result = "Failed to get the system error message.";
	else {
		result = lpBuffer;
		LocalFree(lpBuffer);
	}

	result += " (error code " + std::to_string(dwMessageId) + ")";
	return result;
}
