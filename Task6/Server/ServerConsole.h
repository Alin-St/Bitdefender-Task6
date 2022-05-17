#pragma once

#include <string>
#include "../Task6Lib/task6lib.h"

/// <summary> The console based interface for the server. </summary>
class ServerConsole
{
private:
	const std::wstring applicationPath;
	std::unique_ptr<Task6Server> server;

public:
	/// <param name="applicationPath"> The path of the application executable to use for auto start-up. </param>
	ServerConsole(const std::wstring& appPath);

	void run();

private:
	void uiSetProgramToRunOnStartup();
};
