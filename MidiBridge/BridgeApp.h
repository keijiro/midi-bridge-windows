#pragma once

#include "stdafx.h"
#include "Platform.h"
#include "IpcServer.h"
#include "MidiClient.h"

class BridgeApp
	: public MidiClient::MessageDelegate
{
	IpcServer ipcServer;
	MidiClient midiClient;
public:
	BridgeApp()
		: midiClient(this)
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

			ipcServer.StartInputThread();
			ipcServer.RunOutputLoop();

			ipcServer.CloseConnection();
		}

		midiClient.CloseAllDevices();
	}

	void ProcessIncommingMidiMessageFromDevice(MidiMessage message) override
	{
		printf("MIDI: %x %x %x\n", message.bytes[0], message.bytes[1], message.bytes[2]);
	}
};
