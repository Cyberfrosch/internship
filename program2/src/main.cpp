#include "server.h"

int main()
{
	try
	{
		Server server;
		server.run();
	}
	catch (const std::exception& ex)
	{
		std::cout << "Error: " << ex.what() << std::endl;
	}

	return 0;
}