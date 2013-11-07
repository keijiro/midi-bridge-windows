#include "stdafx.h"
#include "Platform.h"
#include "Server.h"

int _tmain(int argc, _TCHAR* argv[])
{
	Platform::Initialize();

	{
		Server server;
		server.SetUp();

		while (true) {
			puts("Wait for a connection.");
			server.WaitAndAccept();
			puts("Accepted.");

			server.StartInputThread();
			server.RunOutputLoop();

			server.CloseConnection();
		}
	}

	Platform::Finalize();
	return 0;
}
