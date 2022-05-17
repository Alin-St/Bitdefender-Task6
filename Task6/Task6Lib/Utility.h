#pragma once

#include <string>
#include <Windows.h>

namespace Utility
{
	/// <summary> Uses the registry to make a program run when a user logs on. </summary>
	/// <param name="appName"> The name of the registry value used (usually the name of the application). Can't be 0 nor empty. </param>
	/// <param name="appPath"> The command performed when a user logs on (usually the path of the 'exe'). Can't be 0 nor empty. </param>
	/// <param name="currentUser"> Run program on startup only for current user or for all users? </param>
	/// <param name="onlyOnce"> Run program on startup only once or every time a user logs on? </param>
	/// <param name="pathMatches"> If not null and registry value name already used, it returns whether or not the path was the same. </param>
	/// <returns> True if the registry value was added, false if a value with the same name was already there. </returns>
	bool runProgramOnStartup(const wchar_t* appName, const wchar_t* appPath, bool currentUser = true, bool onlyOnce = false, bool* pathMatches = nullptr);

	/// <summary> Removes the program' entry from the 'Run' or 'RunOnce' registry. Doesn't look for any subkeys. </summary>
	/// <param name="appName"> The name of the registry value to remove (usually the name of the application). Can't be 0 nor empty. </param>
	/// <param name="appPath"> If this is not null, the registry value will be removed only if its data refers to the same file. </param>
	/// <param name="currentUser"> Remove registry value from 'HKEY_CURRENT_USER' root key or 'HKEY_LOCAL_MACHINE'? </param>
	/// <param name="stopOneTimeRun"> Remove registry value from 'Run' key or 'RunOnce'? </param>
	/// <returns> True if the registry value was actually removed, false if it wasn't there to begin with. </returns>
	bool stopRunningProgramOnStartup(const wchar_t* appName, const wchar_t* appPath = nullptr, bool currentUser = true, bool stopOneTimeRun = false);

	/// <summary> Get the number of logical processors in the current group. </summary>
	int getNumberOfProcessors();
};