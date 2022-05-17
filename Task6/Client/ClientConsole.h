#pragma once

#include "../Task6Lib/task6lib.h"

class ClientConsole
{
private:
	std::unique_ptr<Task6Client> client;

public:
	void run();
};
