#include "tests.h"
#include "ui.h"
#include <iostream>

int wmain(int argc, wchar_t* argv[])
{
	int exitCode = 0;

	try
	{
		ServerConsole console(argv[0]);
		exitCode = console.run();
	}
	catch (const std::exception& ex) {
		std::cerr << "Error: " << ex.what() << "\n";
		exitCode = -1;
	}

	_CrtDumpMemoryLeaks();
	system("pause");

	return exitCode;
}
