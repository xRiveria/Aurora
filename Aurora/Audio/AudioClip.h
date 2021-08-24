#pragma once
#include "../Resource/AuroraResource.h"

namespace FMOD
{
    class System;
    class Sound;
    class Channel;
}

namespace Aurora
{
    class AudioClip : public AuroraResource
    {
    public:
        AudioClip(EngineContext* engineContext);
        ~AudioClip();

        bool LoadFromFile(const std::string& filePath) const;
        bool SaveToFile(const std::string& filePath) const;
    };
}