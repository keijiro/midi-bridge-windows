#pragma once

#include "stdafx.h"
#include "Debug.h"
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

    // Show the names of the all input and output devices, and try to open these devices.
    void OpenAllDevices()
    {
        // Open the all input devices.
        puts("Input devices:");
        auto inDeviceCount = midiInGetNumDevs();
        for (auto i = 0U; i < inDeviceCount; i++)
        {
            HMIDIIN handle;
            auto result = midiInOpen(&handle, i, reinterpret_cast<DWORD_PTR>(MidiInProc), reinterpret_cast<DWORD_PTR>(&messageDelegate), CALLBACK_FUNCTION);
            bool opened = (result == MMSYSERR_NOERROR);

            MIDIINCAPS caps;
            result = midiInGetDevCaps(i, &caps, sizeof(caps));
            Debug::Assert(result == MMSYSERR_NOERROR, "Failed to retrieve the device caps.");
            wprintf(L"[%d] %s %s\n", i, caps.szPname, opened ? L"" : L"(unavailable)");

            if (opened)
            {
                result = midiInStart(handle);
                Debug::Assert(result == MMSYSERR_NOERROR, "Failed to start input from a MIDI device.");
                inDeviceHandles.push_back(handle);
            }
        }
        puts("");

        // Open the all output devices.
        puts("Output devices:");
        auto outDeviceCount = midiOutGetNumDevs();
        for (auto i = 0U; i < outDeviceCount; i++)
        {
            HMIDIOUT handle;
            auto result = midiOutOpen(&handle, i, NULL, NULL, CALLBACK_NULL);
            bool opened = (result == MMSYSERR_NOERROR);

            MIDIOUTCAPS caps;
            result = midiOutGetDevCaps(i, &caps, sizeof(caps));
            Debug::Assert(result == MMSYSERR_NOERROR, "Failed to retrieve the device caps.");
            wprintf(L"[%d] %s %s\n", i, caps.szPname, opened ? L"" : L"(unavailable)");

            if (opened) outDeviceHandles.push_back(handle);
        }
        puts("");
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
            printf("MIDI: Device (%0x) was disconnected.\n", hMidiIn);
        }
    }
};
