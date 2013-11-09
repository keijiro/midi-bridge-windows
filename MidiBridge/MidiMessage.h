#pragma once

#include "stdafx.h"

// MIDI message class.
struct MidiMessage
{
    uint8_t bytes[4];

    // Construct from a MIDI-in dword.
    MidiMessage(uint32_t raw32)
    {
        uint8_t status = raw32 & 0xff;
        uint8_t eventType = status >> 4;

        bytes[0] = status;

        if (eventType == 0xc || eventType == 0xd || status == 0xf1 || status == 0xf3)
        {
            // Program change (0xc*), aftertouch (0xd*), timecode (0xf1) or song select (0xf3).
            bytes[1] = (raw32 >> 8) & 0xff;
            bytes[2] = 0xff;
            bytes[3] = 0xff;
        }
        else if (eventType != 0xf || status == 0xf2)
        {
            // Song position (0xf2) or other kinds of channel voice messages.
            bytes[1] = (raw32 >> 8) & 0xff;
            bytes[2] = (raw32 >> 16) & 0xff;
            bytes[3] = 0xff;
        }
        else
        {
            // Other system messages: without data bytes.
            bytes[1] = 0xff;
            bytes[2] = 0xff;
            bytes[3] = 0xff;
        }
    }

    // Construct from a byte array.
    MidiMessage(const uint8_t* source)
    {
        bytes[0] = source[0];
        bytes[1] = source[1];
        bytes[2] = source[2];
        bytes[3] = source[3];
    }

    // Returns a MIDI-out dword.
    uint32_t GetRaw32() const
    {
        uint32_t temp = bytes[0];
        if (bytes[1] < 0x80) temp += bytes[1] << 8;
        if (bytes[2] < 0x80) temp += bytes[2] << 16;
        return temp;
    }
};
