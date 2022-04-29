#pragma once

/// <summary> Test if you can successfully make the command run on startup, then remove it from the startup registry. </summary>
/// <param name="appPath"> The path of the application's exe or a command. </param>
void test_run_on_startup(const wchar_t* appPath);
