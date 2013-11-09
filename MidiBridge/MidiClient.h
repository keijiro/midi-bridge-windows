#pragma once

#include "stdafx.h"
#include "Debug.h"
#include "Logger.h"
#include "MidiMessage.h"

// MIDI interface client class.
class MidiClient
{
public:

    // Delegate class used for handling incoming MIDI messages.
    class MessageDelegate
    {
    public:
        virtual void ProcessIncomingMidiMessageFromDevice(MidiMessage message) = 0;
    };

    // Constructor/destructor.
    MidiClient(MessageDelegate& md)
        : messageDelegate(md)
    {
    }

    ~MidiClient()
    {
        CloseAllDevices();
    }

	void TrySwitchState(int id)
	{
		int inDeviceCount = midiInGetNumDevs();
		int outDeviceCount = midiOutGetNumDevs();

		if (id <= 0 || id > inDeviceCount + outDeviceCount) {
			// Invalid ID.
			return;
		}

		if (id <= inDeviceCount) {
			id = id - 1;
			if (CheckInputDeviceOpened(id))
			{
				// Try to close the device.
				CloseInputDevice(id);
			}
			else
			{
				// Try to open the device.
				TryOpenInputDevice(id);
			}
		}
		else
		{
			id = id - 1 - inDeviceCount;
			if (CheckOutputDeviceOpened(id))
			{
				// Try to close the device.
				CloseOutputDevice(id);
			}
			else
			{
				// Try to open the device.
				TryOpenOutputDevice(id);
			}
		}
	}

	// Print the device list.
	void PrintDeviceList()
	{
		// Header.
		puts("----+--------+--------------+----------------------------------");
		puts(" ID |  TYPE  |    STATUS    | DEVICE NAME");
		puts("----+--------+--------------+----------------------------------");

		// Input devices.
		auto inDeviceCount = midiInGetNumDevs();
		for (auto i = 0U; i < inDeviceCount; i++)
		{
			MIDIINCAPS caps;
			auto result = midiInGetDevCaps(i, &caps, sizeof(caps));
			Debug::Assert(result == MMSYSERR_NOERROR, "Failed to retrieve the device caps.");
			bool opened = CheckInputDeviceOpened(i);
			wprintf(L" %2d | Input  | %-12s | %-32s\n", i + 1, opened ? L"Active" : L"", caps.szPname);
		}

		puts("----+--------+--------------+----------------------------------");

		// Output devices.
		auto outDeviceCount = midiOutGetNumDevs();
		for (auto i = 0U; i < outDeviceCount; i++)
		{
			MIDIOUTCAPS caps;
			auto result = midiOutGetDevCaps(i, &caps, sizeof(caps));
			Debug::Assert(result == MMSYSERR_NOERROR, "Failed to retrieve the device caps.");
			bool opened = CheckOutputDeviceOpened(i);
			wprintf(L" %2d | Output | %-12s | %-32s\n", i + 1 + inDeviceCount, opened ? L"Active" : L"", caps.szPname);
		}

		puts("----+--------+--------------+----------------------------------");
	}

    // Try to open the all devices.
    void OpenAllDevices()
    {
        auto inDeviceCount = midiInGetNumDevs();
        for (auto i = 0U; i < inDeviceCount; i++)
        {
			TryOpenInputDevice(i);
        }

        auto outDeviceCount = midiOutGetNumDevs();
        for (auto i = 0U; i < outDeviceCount; i++)
        {
			TryOpenOutputDevice(i);
        }
    }

    // Close the all devices opened by this client.
    void CloseAllDevices()
    {
        for (auto& handle : inDeviceHandles)
        {
            midiInClose(handle);
        }
        inDeviceHandles.clear();

        for (auto& handle : outDeviceHandles)
        {
            midiOutClose(handle);
        }
        outDeviceHandles.clear();
    }

    // Send a MIDI message to the all output devices.
    void SendMessageToDevices(MidiMessage message)
    {
        for (auto& handle : outDeviceHandles)
        {
            midiOutShortMsg(handle, message.GetRaw32());
        }
    }

private:

    MessageDelegate& messageDelegate;
    std::vector<HMIDIIN> inDeviceHandles;
    std::vector<HMIDIOUT> outDeviceHandles;

	// Check if the device is already opened.
	bool CheckInputDeviceOpened(int id)
	{
		for (auto handle : inDeviceHandles)
		{
			UINT idFromHandle;
			if (midiInGetID(handle, &idFromHandle) == MMSYSERR_NOERROR)
			{
				if (idFromHandle == id) return true;
			}
		}
		return false;
	}

	bool CheckOutputDeviceOpened(int id)
	{
		for (auto handle : outDeviceHandles)
		{
			UINT idFromHandle;
			if (midiOutGetID(handle, &idFromHandle) == MMSYSERR_NOERROR)
			{
				if (idFromHandle == id) return true;
			}
		}
		return false;
	}

	// Try to open an device.
	bool TryOpenInputDevice(UINT id)
	{
		HMIDIIN handle;
		DWORD_PTR callback = reinterpret_cast<DWORD_PTR>(MidiInProc);
		DWORD_PTR instance = reinterpret_cast<DWORD_PTR>(&messageDelegate);
		if (midiInOpen(&handle, id, callback, instance, CALLBACK_FUNCTION) == MMSYSERR_NOERROR)
		{
			if (midiInStart(handle) == MMSYSERR_NOERROR)
			{
				inDeviceHandles.push_back(handle);
				return true;
			}
			midiInClose(handle);
		}
		return false;
	}

	bool TryOpenOutputDevice(UINT id)
	{
		HMIDIOUT handle;
		DWORD_PTR callback = reinterpret_cast<DWORD_PTR>(MidiInProc);
		DWORD_PTR instance = reinterpret_cast<DWORD_PTR>(&messageDelegate);
		if (midiOutOpen(&handle, id, callback, instance, CALLBACK_FUNCTION) == MMSYSERR_NOERROR)
		{
			outDeviceHandles.push_back(handle);
			return true;
		}
		return false;
	}

	// Try to close the device.
	void CloseInputDevice(int id)
	{
		for (auto handleItr = inDeviceHandles.begin(); handleItr != inDeviceHandles.end(); ++handleItr)
		{
			UINT idFromHandle;
			if (midiInGetID(*handleItr, &idFromHandle) == MMSYSERR_NOERROR)
			{
				if (idFromHandle == id)
				{
					midiInStop(*handleItr);
					midiInClose(*handleItr);
					inDeviceHandles.erase(handleItr);
					break;
				}
			}
		}
	}

	void CloseOutputDevice(int id)
	{
		for (auto handleItr = outDeviceHandles.begin(); handleItr != outDeviceHandles.end(); ++handleItr)
		{
			UINT idFromHandle;
			if (midiOutGetID(*handleItr, &idFromHandle) == MMSYSERR_NOERROR)
			{
				if (idFromHandle == id)
				{
					midiOutClose(*handleItr);
					outDeviceHandles.erase(handleItr);
					break;
				}
			}
		}
	}

	// MIDI callback function.
    static void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
    {
        if (wMsg == MIM_DATA)
        {
            auto md = reinterpret_cast<MessageDelegate*>(dwInstance);
            md->ProcessIncomingMidiMessageFromDevice(MidiMessage(dwParam1));
        }
        else if (wMsg == MIM_CLOSE)
        {
			Logger::RecordMisc("Device (%0x) was disconnected.", hMidiIn);
        }
    }
};
