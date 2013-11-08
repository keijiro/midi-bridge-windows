#pragma once

#include "stdafx.h"
#include "IpcServer.h"
#include "MidiClient.h"

// Application class.
class BridgeApp
	: public MidiClient::MessageDelegate, IpcServer::MessageDelegate
{
public:

	BridgeApp()
		: ipcServer(*this), midiClient(*this)
	{
	}

	// Application main loop.
	void Run()
	{
		ipcServer.SetUp();
		midiClient.OpenAllDevices();

		while (true)
		{
			puts("Ready to connect.");
			ipcServer.WaitAndAccept();

			puts("A connection was established.");
			ipcServer.RunReceiverLoop();

			ipcServer.CloseConnection();
		}
	}

	// IPC -> MIDI out
	int ProcessIncomingIpcMessageFromClient(const uint8_t* data, int offset, int length)
	{
		Debug::Assert(offset + 4 <= length, "Invalid IPC message.");
		MidiMessage message(data + offset);
		midiClient.SendMessageToDevices(message);
		printf("OUT: %s\n", message.ToString().c_str());
		return offset + 4;
	}

	// MIDI in -> IPC
	void ProcessIncomingMidiMessageFromDevice(MidiMessage message) override
	{
		printf("IN: %s\n", message.ToString().c_str());
		ipcServer.SendToClient(message);
	}

private:

	IpcServer ipcServer;
	MidiClient midiClient;
};
