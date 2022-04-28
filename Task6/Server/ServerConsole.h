#pragma once

#include <string>

/// <summary> The console based interface for the server. </summary>
class ServerConsole
{
private:
	const std::wstring applicationPath;

public:
	/// <param name="applicationPath"> The path of the application executable to use for auto start-up. </param>
	ServerConsole(const wchar_t* applicationPath);

	/// <returns> The exit code (0 for normal exit). </returns>
	int run();

private:
	void uiSetProgramToRunOnStartup();
};
