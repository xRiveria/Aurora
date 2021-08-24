#pragma once
#include "../Log/Log.h"
#include <FMOD/fmod.h>
#include <FMOD/fmod_errors.h>

namespace Aurora
{
    inline bool ParseResult_Audio(FMOD_RESULT result)
    {
        if (result != FMOD_OK)
        {
            AURORA_ERROR(LogLayer::Audio, "Error with FMOD: %s.", FMOD_ErrorString(result));
            return false;
        }

        return true;
    }
}