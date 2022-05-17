#include "ClientConsole.h"
#include "../Task6Lib/task6lib.h"
#include <iostream>

int wmain(int argc, wchar_t* argv[])
{
	bool succ = true;
	try {
		ClientConsole console;
		console.run();
	}
	catch (const std::exception& ex) {
		std::cout << "Error: " << ex.what() << '\n';
		succ = false;
	}

	_CrtDumpMemoryLeaks();
	system("pause");

	return succ ? 0 : -1;
}
