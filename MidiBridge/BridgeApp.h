#pragma once

#include "stdafx.h"
#include "IpcServer.h"
#include "MidiClient.h"
#include "Logger.h"

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
#if 0
		midiClient.OpenAllDevices();
		midiClient.PrintDeviceList();

		Logger::Enable();
		ipcServer.SetUp();
		ipcServer.Start();

		while (true)
		{
			getchar();
			midiClient.CloseAllDevices();
			midiClient.OpenAllDevices();
		}
#else

		midiClient.OpenAllDevices();

		ipcServer.SetUp();
		ipcServer.Start();

		while (true)
		{
			midiClient.PrintDeviceList();

			puts("Enter an ID# or one of the following commands: (s)can, (r)eset, (l)og, (q)uit");

			char input[1024];
			fgets(input, sizeof(input), stdin);

			if (input[0] == 's' || input[0] == '\n')
			{
			}
			else if (input[0] == 'r')
			{
				midiClient.CloseAllDevices();
				midiClient.OpenAllDevices();
			}
			else if (input[0] == 'l')
			{
				puts("Press ENTER key to stop the log viewer.");
				Logger::Enable();
				getchar();
				Logger::Disable();
			}
			else if (input[0] == 'q')
			{
				break;
			}
			else if (input[0] >= '0' && input[0] <= '9')
			{
				midiClient.TrySwitchState(atoi(input));
			}
		}

		midiClient.CloseAllDevices();
		ipcServer.StopAndWait();
#endif
	}

    // IPC -> MIDI out
    int ProcessIncomingIpcMessageFromClient(const uint8_t* data, int offset, int length)
    {
        Debug::Assert(offset + 4 <= length, "Invalid IPC message.");
        MidiMessage message(data + offset);
        midiClient.SendMessageToDevices(message);
		Logger::RecordMidiOutput(message);
        return offset + 4;
    }

    // MIDI in -> IPC
    void ProcessIncomingMidiMessageFromDevice(MidiMessage message) override
    {
		Logger::RecordMidiInput(message);
        ipcServer.SendToClient(message);
    }

private:

    IpcServer ipcServer;
    MidiClient midiClient;
};
