#include "Aurora.h"
#include <FMOD/fmod.hpp>
#include "Audio.h"
#include "AudioUtilities.h"
#include "../Input/Input.h"

namespace Aurora
{
    Audio::Audio(EngineContext* engineContext) : ISubsystem(engineContext)
    {
    }

    Audio::~Audio()
    {
        m_AudioContext->close();
        m_AudioContext->release();
    }

    bool Audio::Initialize()
    {
        // Create context instance.
        FMOD_RESULT result = FMOD::System_Create(&m_AudioContext);

        // Initialize context instance. 
        result = m_AudioContext->init(32, FMOD_INIT_NORMAL, nullptr);

        // m_AudioContext->getVersion();
        // m_AudioContext->get
        return true;
    }

    bool g_IsPlaying = false;
    void Audio::Tick(float deltaTime)
    {
        if (m_AudioContext != nullptr)
        {
            m_AudioContext->update();
        }
    }
}