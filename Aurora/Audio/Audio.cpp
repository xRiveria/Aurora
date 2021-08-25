#include "Aurora.h"
#include <FMOD/fmod.hpp>
#include "Audio.h"
#include "AudioUtilities.h"
#include "../Input/Input.h"
#include "../Renderer/Renderer.h"

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


    void Audio::Tick(float deltaTime)
    {
        /// We will play audio regardless of whether we're in play or editor mode.



        if (!m_AudioContext)
        {
            return;
        }

        // Internal update loop.
        ParseResult_Audio(m_AudioContext->update());

        /// Our listener is our camera position at the moment.
        /// Temporary
        XMFLOAT3 position = m_EngineContext->GetSubsystem<Renderer>()->m_Camera->GetTransform()->GetPosition();
        FMOD_VECTOR fModPosition = { position.x, position.y, position.z };
        FMOD_VECTOR fModVelocity = { 0.0f ,0.0f, 0.0f };
        FMOD_VECTOR fModForward = { 0.0f, 0.0f, 1.0f };
        FMOD_VECTOR fModUp = { 0.0f, 1.0f, 0.0f };

        ParseResult_Audio(m_AudioContext->set3DListenerAttributes(0, &fModPosition, &fModVelocity, &fModForward, &fModUp));
    }
}