#pragma once

#include "stdafx.h"
#include "Debug.h"
#include "MidiMessage.h"

// ICP server class.
class IpcServer
{
public:

	// Socket type and constants.
#ifdef WIN32
	typedef SOCKET socket_t;
#else
	typedef int socket_t;
	static const socket_t SOCKET_ERROR = -1;
#endif

	// Delegate class for handling incoming IPC messages.
	class MessageDelegate
	{
	public:
		virtual int ProcessIncomingIpcMessageFromClient(const uint8_t* data, int offset, int length) = 0;
	};

	// Port number used for communication with the client.
	static const int portNumber = 52364;

	// Constructor.
	IpcServer(MessageDelegate& md)
		: messageDelegate(md)
	{
		listenSocket = SOCKET_ERROR;
		clientSocket = SOCKET_ERROR;
		senderThreadHandle = NULL;
	}

	// Destructor. Blocks until the sender thread ends.
	~IpcServer()
	{
		if (senderThreadHandle != NULL)
		{
			quitSenderThread = true;
			WaitForSingleObject(senderThreadHandle, INFINITE);
		}
		if (clientSocket != SOCKET_ERROR) closesocket(clientSocket);
		if (listenSocket != SOCKET_ERROR) closesocket(listenSocket);
	}

	// Sets up the listening socket.
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

	// Waits for a connection from a client and accepts it.
	void WaitAndAccept()
	{
		clientSocket = accept(listenSocket, NULL, NULL);
		Debug::Assert(clientSocket != SOCKET_ERROR, "Failed on accepting the socket (%d)", errno);
	}

	// Closes the client connection and wait for it.
	void CloseConnectionAndWait()
	{
		if (senderThreadHandle != NULL)
		{
			quitSenderThread = true;
			WaitForSingleObject(senderThreadHandle, INFINITE);
		}
		if (clientSocket != SOCKET_ERROR) closesocket(clientSocket);
	}

	// Send a message to the client.
	bool SendToClient(const MidiMessage message)
	{
		int result = send(clientSocket, (char*)message.bytes, sizeof(message), 0);
		return result == sizeof(message);
	}

	// Start the sender thread.
	void StartSenderThread()
	{
		quitSenderThread = false;
		senderThreadHandle = CreateThread(NULL, 0, SenderThreadEntryFunction, this, 0, NULL);
		Debug::Assert(senderThreadHandle != NULL, "Failed to create sender thread.");
	}

	// Runs the receiver loop. Returns when the connection is closed.
	void RunReceiverLoop()
	{
		u_char buffer[2048];
		int filled = 0;

		while (true)
		{
			// Receive data from the connection.
			int length = recv(clientSocket, (char *)buffer + filled, sizeof(buffer) - filled, 0);

			if (length == 0)
			{
				puts("IPC Receiver: the connection seems to be list. Stop receiving.");
				break;
			}
			else if (length < 0)
			{
				printf("recv failed (%d)\n", errno);
				break;
			}

			filled += length;

			// Process the messages with the delegate.
			int offset = 0;
			while (offset + 4 <= filled)
			{
				offset = messageDelegate.ProcessIncomingIpcMessageFromClient(buffer, offset, filled);
			}

			// Clear the data processed with the delegate.
			if (offset == filled)
			{
				filled = 0;
			}
			else
			{
				memcpy(buffer, buffer + offset, filled - offset);
				filled = filled - offset;
			}
		}
	}

private:

	// Delegate used for processing the incoming messages.
	MessageDelegate& messageDelegate;

	// Sockets for listening and communication.
	socket_t listenSocket;
	socket_t clientSocket;

	// Sender thread.
	HANDLE senderThreadHandle;
	bool quitSenderThread;

	// Runs the sender loop. Returns when the connection is lost, or quit flag is raised.
	void RunSenderLoop()
	{
		return;
		while (!quitSenderThread)
		{
			static uint8_t buffer[4] = { 0xb0, 30, 100, 0xff };
			int result = send(clientSocket, (char*)buffer, sizeof(buffer), 0);

			if (result < sizeof(buffer)) {
				puts("IPC Sender: the connection seems to be lost. Stop sending.");
				break;
			}

			puts("Sent.");
			Sleep(100);

			buffer[2] = (buffer[2] + 1) & 0x7f;
		}
	}

#ifdef WIN32
	static DWORD WINAPI SenderThreadEntryFunction(LPVOID lpParam)
	{
		IpcServer* server = reinterpret_cast<IpcServer*>(lpParam);
		server->RunSenderLoop();
		return 0;
	}
#else
#error Unimplemented!!
#endif
};
