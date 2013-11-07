#pragma once

#include "stdafx.h"
#include "Debug.h"

class Platform
{
public:
#ifdef WIN32
	static void Initialize()
	{
		WSADATA wsaData;
		int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
		Debug::Assert(result == 0, "WSAStartup error.");
	}

	static void Finalize()
	{
		WSACleanup();
	}
#else
	static void Initialize()
	{
	}

	static void Finalize()
	{
	}
#endif
};
