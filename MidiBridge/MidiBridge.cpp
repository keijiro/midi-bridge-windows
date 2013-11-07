#include "stdafx.h"
#include "Platform.h"
#include "Server.h"
#include "MidiClient.h"

namespace
{
	class BridgeApp
		: public MidiClient::MessageDelegate
	{
		Server ipcServer;
		MidiClient midiClient;
	public:
		BridgeApp()
			: midiClient(this)
		{
		}

		void Run()
		{
			ipcServer.SetUp();
			midiClient.OpenAllDevices();

			while (true)
			{
				puts("Wait for a connection.");
				ipcServer.WaitAndAccept();
				puts("Accepted.");

				ipcServer.StartInputThread();
				ipcServer.RunOutputLoop();

				ipcServer.CloseConnection();
			}

			midiClient.CloseAllDevices();
		}

		void ProcessIncommingMidiMessageFromDevice(MidiMessage message) override
		{
			printf("MIDI: %x %x %x\n", message.bytes[0], message.bytes[1], message.bytes[2]);
		}
	};
}

int _tmain(int argc, _TCHAR* argv[])
{
	Platform::Initialize();
	BridgeApp().Run();
	Platform::Finalize();
	return 0;
}
