#pragma once

#include "stdafx.h"
#include "Debug.h"

class MidiMessage
{
public:
	uint8_t bytes[4];

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
};
