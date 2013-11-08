#include "stdafx.h"
#include "Platform.h"
#include "BridgeApp.h"

int _tmain(int argc, _TCHAR* argv[])
{
    Platform::Initialize();
    BridgeApp().Run();
    Platform::Finalize();
    return 0;
}
