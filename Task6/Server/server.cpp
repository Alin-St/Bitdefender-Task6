#include "ServerConsole.h"
#include "../Task6Lib/task6lib.h"
#include <iostream>

int wmain(int argc, wchar_t* argv[])
{
	// Run tests from Task6 library.
	test_run_on_startup(argv[0]);

	bool succ = true;
	try {
		ServerConsole console(argv[0]);
		console.run();
	}
	catch (const std::exception& ex) {
		std::cout << "Error: " << ex.what() << "\n";
		succ = false;
	}

	_CrtDumpMemoryLeaks();
	system("pause");

	return succ ? 0 : -1;
}
