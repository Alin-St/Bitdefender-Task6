#pragma once

#include <string>

/// <summary> The console based interface for the server. </summary>
class ServerConsole
{
private:
	const std::wstring applicationPath;

public:
	/// <param name="applicationPath"> The path of the application executable to use for auto start-up. </param>
	ServerConsole(const std::wstring& appPath);

	void run();

private:
	void uiSetProgramToRunOnStartup();
};
