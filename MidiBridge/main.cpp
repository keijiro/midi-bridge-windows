#include "stdafx.h"
#include "Platform.h"
#include "BridgeApp.h"

int _tmain(int argc, _TCHAR* argv[])
{
    Platform::Initialize();

	// Parse the options.
	bool interactive = false;
	for (int i = 0; i < argc; i++)
	{
		auto arg = std::wstring(argv[i]);
		if (arg == L"/i" || arg == L"-i")
		{
			interactive = true;
		}
	}

	// Run the app in the specified mode.
	if (interactive)
	{
		BridgeApp().RunInteractive();
	}
	else
	{
		BridgeApp().RunAutomatic();
	}

    Platform::Finalize();
    return 0;
}
