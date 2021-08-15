#include "WebsocketServer.h"

int main()
{
	std::string sPort;
	std::cout << "Input server port: ";

	int port = -1;
	bool isValidPort = false;
	do
	{
		try
		{
			std::cin >> sPort;
			port = std::stoi(sPort);
			isValidPort = true;
		}
		catch (const std::exception&)
		{
			std::cout << "Bad port format, try again: ";
		}
	} while (!isValidPort);
	
	WebsocketServer::Init(port);
	WebsocketServer::Run();
}