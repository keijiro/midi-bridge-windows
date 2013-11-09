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
		Logger::Enable();

		ipcServer.SetUp();
		ipcServer.Start();

		Sleep(100);

		while (true)
		{
			midiClient.OpenAllDevices();
			getchar();
			midiClient.CloseAllDevices();
		}

		ipcServer.StopAndWait();
    }

    // IPC -> MIDI out
    int ProcessIncomingIpcMessageFromClient(const uint8_t* data, int offset, int length)
    {
        Debug::Assert(offset + 4 <= length, "Invalid IPC message.");
        MidiMessage message(data + offset);
        midiClient.SendMessageToDevices(message);
		Logger::RecordOutput(message);
        return offset + 4;
    }

    // MIDI in -> IPC
    void ProcessIncomingMidiMessageFromDevice(MidiMessage message) override
    {
		Logger::RecordInput(message);
        ipcServer.SendToClient(message);
    }

private:

    IpcServer ipcServer;
    MidiClient midiClient;
};
