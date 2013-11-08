#pragma once

#include "stdafx.h"

// Utility class for debugging.
class Debug
{
public:

    // Assert function which displays a message on a failure.
    static void Assert(bool condition, const char* format, ...)
    {
        if (!condition)
        {
            puts("Assertion failed:");

            va_list args;
            va_start(args, format);
            vprintf(format, args);
            va_end(args);

            puts("");

            getchar();
            exit(EXIT_FAILURE);
        }
    }
};
