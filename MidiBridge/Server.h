#pragma once

#include "stdafx.h"
#include "Debug.h"

class Server
{
public:
#ifdef WIN32
	typedef SOCKET socket_t;
#else
	typedef int socket_t;
	static const socket_t SOCKET_ERROR = -1;
#endif

	static const int portNumber = 52364;

	socket_t listenSocket;
	socket_t clientSocket;

	HANDLE inputThreadHandle;
	bool quitInputThread;

	Server()
	{
		listenSocket = SOCKET_ERROR;
		clientSocket = SOCKET_ERROR;
		inputThreadHandle = NULL;
	}

	~Server()
	{
		if (inputThreadHandle != NULL)
		{
			quitInputThread = true;
			WaitForSingleObject(inputThreadHandle, INFINITE);
		}
		if (clientSocket != SOCKET_ERROR) closesocket(clientSocket);
		if (listenSocket != SOCKET_ERROR) closesocket(listenSocket);
	}

	void CloseConnection()
	{
		if (inputThreadHandle != NULL)
		{
			quitInputThread = true;
			WaitForSingleObject(inputThreadHandle, INFINITE);
		}
		if (clientSocket != SOCKET_ERROR) closesocket(clientSocket);
	}

	void SetUp()
	{
		// Create a socket for listening.
		listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		Debug::Assert(listenSocket != SOCKET_ERROR, "Failed to create a socket for listening (%d)", errno);

		// Make the socket reusable.
		int flag = 1;
		setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag));

		// Give a name for the listening socket.
		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(portNumber);

		int result = bind(listenSocket, (sockaddr *)&addr, sizeof(addr));
		Debug::Assert(result != SOCKET_ERROR, "Failed on binding the listening socket (%d)", errno);

		// Start listening.
		result = listen(listenSocket, SOMAXCONN);
		Debug::Assert(result != SOCKET_ERROR, "Failed to start listening on the socket (%d)", errno);
	}

	void WaitAndAccept()
	{
		clientSocket = accept(listenSocket, NULL, NULL);
		Debug::Assert(clientSocket != SOCKET_ERROR, "Failed on accepting the socket (%d)", errno);
	}

	void RunOutputLoop()
	{
		while (true)
		{
			u_char buffer[1023];
			int length = recv(clientSocket, (char *)buffer, sizeof(buffer), 0);

			if (length == 0)
			{
				puts("OutputThread: the connection seems to be list. Stop receiving.");
				break;
			}
			else if (length < 0)
			{
				printf("recv failed (%d)\n", errno);
				break;
			}

			for (int i = 0; i < length; i++) {
				printf("%02x ", buffer[i]);
			}
			puts("");
		}
	}

	void RunInputLoop()
	{
		while (!quitInputThread)
		{
			static uint8_t buffer[4] = { 0xb0, 30, 100, 0xff };
			int result = send(clientSocket, (char*)buffer, sizeof(buffer), 0);

			if (result < sizeof(buffer)) {
				puts("InputThread: the connection seems to be lost. Stop sending.");
				break;
			}

			puts("Sent.");
			Sleep(100);

			buffer[2] = (buffer[2] + 1) & 0x7f;
		}
	}

#ifdef WIN32
	void StartInputThread()
	{
		quitInputThread = false;
		inputThreadHandle = CreateThread(NULL, 0, InputThreadEntryFunction, this, 0, NULL);
		Debug::Assert(inputThreadHandle != NULL, "Failed to create a thread.");
	}

	static DWORD WINAPI InputThreadEntryFunction(LPVOID lpParam)
	{
		Server* server = reinterpret_cast<Server*>(lpParam);
		server->RunInputLoop();
		return 0;
	}
#else
	void StartInputThread()
	{
#error Unimplemented!!
	}
#endif
};
