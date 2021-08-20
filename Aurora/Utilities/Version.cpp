#pragma once
#include "Aurora.h"

namespace Aurora
{
    const int g_VersionMajor = 0;
    const int g_VersionMinor = 3;
    const int g_VersionRevision = 6;

    const std::string g_VersionToString = std::to_string(g_VersionMajor) + "." + std::to_string(g_VersionMinor) + "." + std::to_string(g_VersionRevision);

    int GetVersionMajor()
    {
        return g_VersionMajor;
    }

    int GetVersionMinor()
    {
        return g_VersionMinor;
    }

    int GetVersionRevision()
    {
        return g_VersionRevision;
    }

    const char* GetVersionString()
    {
        return g_VersionToString.c_str();
    }
}