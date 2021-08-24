#include "Aurora.h"
#include "Audio.h"
#include <FMOD/fmod.hpp>
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
        m_Sound->release();
    }

    bool Audio::Initialize()
    {
        // Create context instance.
        FMOD_RESULT result = FMOD::System_Create(&m_AudioContext);

        // Initialize context instance. 
        result = m_AudioContext->init(32, FMOD_INIT_NORMAL, nullptr);

        const std::string resourceDirectory = m_EngineContext->GetSubsystem<Aurora::Settings>()->GetProjectDirectory() + "Audio/Retro.wav";
        result = m_AudioContext->createSound(resourceDirectory.c_str(), FMOD_DEFAULT, 0, &m_Sound);
        // m_AudioContext->getVersion();
        // m_AudioContext->get



        return true;
    }

    void Audio::Tick(float deltaTime)
    {
        if (m_AudioContext != nullptr)
        {
            m_AudioContext->update();

            if (m_EngineContext->GetSubsystem<Input>()->IsKeyPressed(AURORA_KEY_P))
            {
                m_AudioContext->playSound(m_Sound, 0, false, &m_Channel);
            }
        }
    }
}