#include "ClientConsole.h"
#include <iostream>

void ClientConsole::run()
{
	this->client = std::make_unique<Task6Client>();

	char command = 0;
	std::cout << "Connect to the pipe server or shared memory server? (1/2): ";
	std::cin >> command;
	if (command == '1')
		this->client->startPipe();
	else if (command == '2')
		this->client->startSharedMemory();
	else
		throw std::runtime_error("Unknown command.");

	while (true)
	{
		auto timerStart = std::time(nullptr);

		while (true)
		{
			int timeout = (int)(timerStart + 10 - std::time(nullptr)) * 1000;
			if (timeout < 0)
				break;

			bool bReady = this->client->wait((unsigned long)timeout);

			if (bReady)
			{
				std::string messg;
				while (this->client->popMessg(messg))
					std::cout << messg << '\n';
			}
			else {
				std::cout << "Nothing happened.\n";
				break;
			}
		}

		std::cout << "Do you want to continue? (1/0): ";
		std::cin >> command;

		if (command == '0')
			return;
		else if (command == '1')
			continue;
		else
			throw std::runtime_error("Unknown command.");
	}
}
