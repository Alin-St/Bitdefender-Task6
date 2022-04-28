#include "ui.h"
#include <iostream>

int wmain(int argc, wchar_t* argv[])
{
	int exitCode{};

	try {
		ClientConsole console;
		exitCode = console.run();
	}
	catch (const std::exception& ex) {
		std::cerr << "Error: " << ex.what() << '\n';
		exitCode = -1;
	}

	_CrtDumpMemoryLeaks();
	system("pause");

	return exitCode;
}
