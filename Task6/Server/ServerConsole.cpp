#include "ServerConsole.h"
#include "utilities.h"
#include <iostream>

ServerConsole::ServerConsole(const wchar_t* appPath) : applicationPath(appPath) {}

int ServerConsole::run()
{
	//uiSetProgramToRunOnStartup();

	return 0;
}

void ServerConsole::uiSetProgramToRunOnStartup()
{
	// Try to set the program to run on startup. If it was already there do nothing.
	// If a value with the same name was in the registry, ask the user whether to replace it or not.
	// If the user wants to replace it, delete it and set the value again. Otherwise do nothing.

	constexpr const wchar_t* APP_NAME = L"Task6";

	bool pathMatches{}, success = false;
	if (runProgramOnStartup(APP_NAME, this->applicationPath.c_str(), true, false, &pathMatches))
		success = true;
	else {
		if (pathMatches)
			std::cout << "Program was already set to run on startup.\n";
		else {
			bool resetRegValue{};
			std::cout << "A registry value with the name Task6 already found; "
				"do you want to delete it and try setting the program to run on startup again? (1/0): ";
			std::cin >> resetRegValue;

			if (resetRegValue) {
				stopRunningProgramOnStartup(APP_NAME, nullptr, true, false);
				std::cout << "Registry value with the same name deleted.\n";
				success = runProgramOnStartup(APP_NAME, this->applicationPath.c_str(), true, false);
			}
		}
	}

	std::cout << (success ? "Program set to run on startup\n" : "Program not set to run on startup\n");
}
