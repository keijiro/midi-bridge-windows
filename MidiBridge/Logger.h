#pragma once

#include "stdafx.h"
#include "Debug.h"
#include "MidiMessage.h"

// Log taking and view class.
class Logger
{
public:

	static void Enable()
	{
		GetState().enabled = true;
	}

	static void Disable()
	{
		GetState().enabled = false;
	}

	// Miscellaneous record.
	static void RecordMisc(const char* format, ...)
	{
		Logger& state = GetState();
		if (!state.enabled) return;
		if (state.rowCount >= 0) puts("-----+----------+----+-----------");
		va_list args;
		va_start(args, format);
		vprintf(format, args);
		va_end(args);
		puts("");
		state.rowCount = -1;
	}

	static void RecordMisc(const wchar_t* format, ...)
	{
		Logger& state = GetState();
		if (!state.enabled) return;
		if (state.rowCount >= 0) puts("-----+----------+----+-----------");
		va_list args;
		va_start(args, format);
		vwprintf(format, args);
		va_end(args);
		puts("");
		state.rowCount = -1;
	}

	static void PrintSeparator()
	{
		puts("---------------------------------");
	}

	// MIDI input record.
	static void RecordInput(MidiMessage& message)
	{
		if (!GetState().enabled) return;
		PrintMidiHeaderWithInterval();
		RecordMidiMessage(message, "IN");
	}

	// MIDI output record.
	static void RecordOutput(MidiMessage& message)
	{
		if (!GetState().enabled) return;
		PrintMidiHeaderWithInterval();
		RecordMidiMessage(message, "OUT");
	}

	// Print a header for MIDI records.
	static void PrintMidiHeader()
	{
		puts("-----+----------+----+-----------");
		puts(" I/O | Event    | Ch | Data");
		puts("-----+----------+----+-----------");
	}

	static void PrintMidiHeaderWithInterval()
	{
		Logger& state = GetState();
		if (state.rowCount < 0 || state.rowCount++ >= 20) {
			PrintMidiHeader();
			state.rowCount = 0;
		}
	}

	// Print a MIDI message.
	static void RecordMidiMessage(MidiMessage& message, const char* ioLabel)
	{
		static const char *statusLabels[] =
		{
			"Note Off",	// 0x8*
			"Note On ", // 0x9*
			"A.Touch ", // 0xa*
			"CC      ", // 0xb*
			"Program ", // 0xc*
			"Pressure", // 0xd*
			"P.Wheel ", // 0xe*
			"System  "  // 0xf*
		};
		if ((message.bytes[0] & 0xf0) == 0xf0)
		{
			printf(" %3s | System   | -- | 0x%x\n", ioLabel, message.bytes[0]);
		}
		else
		{
			const char* statusLabel = statusLabels[(message.bytes[0] >> 4) & 7];
			int channel = (message.bytes[0] & 0xf) + 1;
			if (message.bytes[2] > 0x7f)
			{
				printf(" %3s | %s | %02d | %d\n", ioLabel, statusLabel, channel, message.bytes[1]);
			}
			else
			{
				printf(" %3s | %s | %02d | %d, %d\n", ioLabel, statusLabel, channel, message.bytes[1], message.bytes[2]);
			}
		}
	}

private:

	bool enabled;
	int rowCount;

	Logger()
	{
		rowCount = -1;
	}

	static Logger& GetState()
	{
		static Logger state;
		return state;
	}
};
