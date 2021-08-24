#pragma once
#include "EngineContext.h"
#include "ISubsystem.h"

namespace FMOD
{
    class System;
    class Sound;
    class Channel;
}

namespace Aurora
{
    class Audio : public ISubsystem
    {
    public:
        Audio(EngineContext* engineContext);
        ~Audio();

        bool Initialize() override;
        void Tick(float deltaTime) override;

    private:
        FMOD::System* m_AudioContext;
        void* m_ExtraDriverData = 0;
        FMOD::Channel* m_Channel = 0;

        FMOD::Sound* m_Sound;
    };
}
