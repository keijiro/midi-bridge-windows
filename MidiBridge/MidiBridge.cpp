#include "stdafx.h"
#include "Debug.h"
#include "Platform.h"
#include "Server.h"

int _tmain(int argc, _TCHAR* argv[])
{
	Platform::Initialize();

	{
		Server server;
		server.SetUp();

		puts("Wait for a connection.");
		server.WaitAndAccept();
		puts("Accepted.");

		server.StartInputThread();
		server.RunOutputLoop();

		server.Close();
	}

	Platform::Finalize();

	puts("Done.");
	getchar();

	return 0;
}
