#pragma once

#include "stdafx.h"
#include "Debug.h"
#include "MidiMessage.h"
#include "Logger.h"

// ICP server used to communicate with Unity.
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

	// Port number used for communication with the client.
	static const int portNumber = 52364;

	// Delegate class for handling incoming IPC messages.
    class MessageDelegate
    {
    public:
        virtual int ProcessIncomingIpcMessageFromClient(const uint8_t* data, int offset, int length) = 0;
    };

    // Constructor.
    IpcServer(MessageDelegate& md)
        : messageDelegate(md)
    {
        listenSocket = SOCKET_ERROR;
        clientSocket = SOCKET_ERROR;
    }

    // Destructor. Blocks until the sender thread ends.
    ~IpcServer()
    {
		StopAndWait();
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

	// Send a message to the client.
	bool SendToClient(const MidiMessage message)
	{
		int result = send(clientSocket, (char*)message.bytes, sizeof(message), 0);
		return (result == sizeof(message));
	}

	// Start the receiver thread.
	void Start()
	{
		stopReceiverThread = false;
#ifdef WIN32
		receiverThread = CreateThread(nullptr, 0, ReceiverThreadEntry, this, 0, nullptr);
		Debug::Assert(receiverThread != nullptr, "Failed to start the IPC receiver thread.");
#else
#error unimplemented
#endif
	}

	// Stop the receiver thread and wait for it.
	void StopAndWait()
	{
		stopReceiverThread = true;
#ifdef WIN32
		WaitForSingleObject(receiverThread, INFINITE);
#else
#error unimplemented
#endif
	}

private:

    // Delegate used for processing the incoming messages.
    MessageDelegate& messageDelegate;

    // Sockets for listening and communication.
    socket_t listenSocket;
    socket_t clientSocket;

	// Stop flag for stopping the receiver thread.
	bool stopReceiverThread;

	// Runs the receiver thread loop.
	void RunReceiverLoop()
	{
		while (!stopReceiverThread)
		{
			Logger::RecordMisc("Waiting for a connection.");

			// Accept a new connection.
			clientSocket = accept(listenSocket, NULL, NULL);
			Debug::Assert(clientSocket != SOCKET_ERROR, "Failed on accepting the socket (%d)", errno);
			
			Logger::RecordMisc("Accepted a new connection.");

			u_char buffer[2048];
			int filled = 0;

			while (!stopReceiverThread)
			{
				// Receive data from the connection.
				int length = recv(clientSocket, (char *)buffer + filled, sizeof(buffer)-filled, 0);

				if (length == 0)
				{
					Logger::RecordMisc("IPC: The connection seems to be lost.");
					break;
				}
				else if (length < 0)
				{
					Logger::RecordMisc("recv failed (%d)\n", errno);
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

			Logger::RecordMisc("Closing the connection.");

			// Close the connection anyway.
			closesocket(clientSocket);
			clientSocket = SOCKET_ERROR;
		}
	}

#ifdef WIN32

	// Receiver thread handler.
	HANDLE receiverThread;

	// The entry point for the receiver thread.
	static DWORD WINAPI ReceiverThreadEntry(LPVOID param)
	{
		IpcServer* server = reinterpret_cast<IpcServer*>(param);
		server->RunReceiverLoop();
		return 0;
	}

#endif
};
