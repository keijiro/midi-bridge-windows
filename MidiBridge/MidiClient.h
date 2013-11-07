#pragma once

#include "stdafx.h"
#include "Debug.h"
#include "MidiMessage.h"

class MidiClient
{
public:
	class MessageDelegate
	{
	public:
		virtual void ProcessIncommingMidiMessageFromDevice(MidiMessage message) = 0;
	};

	MessageDelegate *messageDelegate;
	std::vector<HMIDIIN> handles;

	MidiClient(MessageDelegate *md)
		: messageDelegate(md)
	{
	}

	void OpenAllDevices()
	{
		int deviceCount = midiInGetNumDevs();
		for (int i = 0; i < deviceCount; i++) {
			HMIDIIN handle;

			MMRESULT result = midiInOpen(&handle, i, reinterpret_cast<DWORD_PTR>(MidiInProc), reinterpret_cast<DWORD_PTR>(messageDelegate), CALLBACK_FUNCTION);
			Debug::Assert(result == MMSYSERR_NOERROR, "Failed to open a connection to a MIDI device.");

			result = midiInStart(handle);
			Debug::Assert(result == MMSYSERR_NOERROR, "Failed to start input from a MIDI device.");

			handles.push_back(handle);

			wprintf(L"MIDI device found: %s\n", GetDeviceName(handle).c_str());
		}
	}

	void CloseAllDevices()
	{
		for (auto& handle : handles)
		{
			midiInClose(handle);
		}
	}

	std::wstring GetDeviceName(HMIDIIN handle)
	{
		UINT id;
		MMRESULT result = midiInGetID(handle, &id);
		Debug::Assert(result == MMSYSERR_NOERROR, "Can't get the device ID.");

		MIDIINCAPS caps;
		result = midiInGetDevCaps(id, &caps, sizeof(caps));
		Debug::Assert(result == MMSYSERR_NOERROR, "Can't retrieve the device caps.");

		return std::wstring(caps.szPname);
	}

	// MIDI callback function.
	static void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
	{
		if (wMsg == MIM_DATA) {
			MessageDelegate* md = reinterpret_cast<MessageDelegate*>(dwInstance);
			md->ProcessIncommingMidiMessageFromDevice(MidiMessage(dwParam1));
		}
	}
};
