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

    // Application main loop: automatic mode.
	void RunAutomatic()
	{
		// Initialize the MIDI client.
		midiClient.OpenAllDevices();
		midiClient.PrintDeviceList();

		Logger::Enable();
		
		// Start IPC.
		ipcServer.SetUp();
		ipcServer.Start();

		while (true)
		{
			GetLine();

			// Rescan and grab the MIDI devices.
			midiClient.CloseAllDevices();
			midiClient.OpenAllDevices();
		}
	}

	// Application main loop: interactive mode.
	void RunInteractive()
	{
		// Initialize the MIDI client.
		midiClient.OpenAllDevices();

		// Start IPC.
		ipcServer.SetUp();
		ipcServer.Start();

		while (true)
		{
			// Display the current status.
			midiClient.PrintDeviceList();

			// Command line.
			puts("Enter an ID or one of the following commands: (s)can, (r)eset, (l)og, (q)uit");
			auto input = GetLine();

			if (input[0] >= '0' && input[0] <= '9')
			{
				// ID number: switch the state of the device.
				midiClient.TrySwitchState(atoi(input.c_str()));
			}
			else if (input[0] == 'r')
			{
				// Reset: rescan and grab the all MIDI devices.
				midiClient.CloseAllDevices();
				midiClient.OpenAllDevices();
			}
			else if (input[0] == 'l')
			{
				// Log: enable the logger until the user interrupts.
				puts("===================================");
				puts("Press ENTER to quit the log viewer.");
				puts("===================================");
				Logger::Enable();
				GetLine();
				Logger::Disable();
			}
			else if (input[0] == 'q')
			{
				// Quit: break the main loop.
				break;
			}
		}

		// Cleaning up.
		midiClient.CloseAllDevices();
		ipcServer.StopAndWait();
	}

private:

	IpcServer ipcServer;
	MidiClient midiClient;
	
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

	// Utility: get a line from stdin.
	static std::string GetLine()
	{
		char input[32];
		fgets(input, sizeof(input), stdin);
		return std::string(input);
	}
};
