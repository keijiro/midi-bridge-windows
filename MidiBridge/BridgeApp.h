#pragma once

#include "stdafx.h"
#include "IpcServer.h"
#include "MidiClient.h"

class BridgeApp
	: public MidiClient::MessageDelegate, IpcServer::MessageDelegate
{
public:

	BridgeApp()
		: ipcServer(*this), midiClient(*this)
	{
	}

	void Run()
	{
		ipcServer.SetUp();
		midiClient.OpenAllDevices();

		while (true)
		{
			puts("Wait for a connection.");
			ipcServer.WaitAndAccept();
			puts("Accepted.");

			ipcServer.StartSenderThread();
			ipcServer.RunReceiverLoop();

			ipcServer.CloseConnectionAndWait();
		}
	}

	// IPC -> MIDI out
	int ProcessIncomingIpcMessageFromClient(const uint8_t* data, int offset, int length)
	{
		Debug::Assert(offset + 4 <= length, "Invalid IPC message.");
		midiClient.SendToDefaultDevice(MidiMessage(data + offset));
		return offset + 4;
	}

	// MIDI in -> IPC
	void ProcessIncomingMidiMessageFromDevice(MidiMessage message) override
	{
		ipcServer.SendToClient(message);
	}

private:

	IpcServer ipcServer;
	MidiClient midiClient;
};
