#pragma once
#include "LogUtilities.h"

namespace Aurora
{
    class ILogger
    {
    public:
        virtual ~ILogger() = default;
        virtual void LogMessage(const LogPackage& logPackage) = 0;
    };
}