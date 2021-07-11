#include "Server.h"


int main()
{
    Server serv{ 8000 };
    serv.Start();
	return 0;
}