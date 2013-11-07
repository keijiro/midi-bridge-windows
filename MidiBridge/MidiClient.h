#pragma once

#include "stdafx.h"
#include "Debug.h"
#include "MidiMessage.h"

class MidiClient
{
public:

	// Delegate class used for handling incoming MIDI messages.
	class MessageDelegate
	{
	public:
		virtual void ProcessIncomingMidiMessageFromDevice(MidiMessage message) = 0;
	};

	MidiClient(MessageDelegate *md)
		: messageDelegate(md)
	{
	}

	void OpenAllDevices()
	{
		auto inDeviceCount = midiInGetNumDevs();
		auto outDeviceCount = midiOutGetNumDevs();
		printf("There are %d input and %d output devices.\n", inDeviceCount, outDeviceCount);

		// Open the all input devices.
		for (auto i = 0U; i < inDeviceCount; i++)
		{
			HMIDIIN handle;
			auto result = midiInOpen(&handle, i, reinterpret_cast<DWORD_PTR>(MidiInProc), reinterpret_cast<DWORD_PTR>(messageDelegate), CALLBACK_FUNCTION);
			Debug::Assert(result == MMSYSERR_NOERROR, "Failed to open a MIDI input device.");

			MIDIINCAPS caps;
			result = midiInGetDevCaps(i, &caps, sizeof(caps));
			Debug::Assert(result == MMSYSERR_NOERROR, "Can't retrieve the device caps.");
			wprintf(L"IN(%d): %s\n", i, caps.szPname);

			result = midiInStart(handle);
			Debug::Assert(result == MMSYSERR_NOERROR, "Failed to start input from a MIDI device.");

			inDeviceHandles.push_back(handle);
		}

		// Open the all output devices.
		for (auto i = 0U; i < outDeviceCount; i++)
		{
			HMIDIOUT handle;
			auto result = midiOutOpen(&handle, i, NULL, NULL, CALLBACK_NULL);
			Debug::Assert(result == MMSYSERR_NOERROR, "Failed to open a MIDI output device.");

			MIDIOUTCAPS caps;
			result = midiOutGetDevCaps(i, &caps, sizeof(caps));
			Debug::Assert(result == MMSYSERR_NOERROR, "Can't retrieve the device caps.");
			wprintf(L"OUT(%d): %s\n", i, caps.szPname);

			outDeviceHandles.push_back(handle);
		}
	}

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

private:

	MessageDelegate *messageDelegate;
	std::vector<HMIDIIN> inDeviceHandles;
	std::vector<HMIDIOUT> outDeviceHandles;

	// MIDI callback function.
	static void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
	{
		if (wMsg == MIM_DATA)
		{
			auto md = reinterpret_cast<MessageDelegate*>(dwInstance);
			md->ProcessIncomingMidiMessageFromDevice(MidiMessage(dwParam1));
		}
	}
};
