#include "Aurora.h"
#include "AudioSource.h"
#include "../Audio/AudioClip.h"
#include "../Resource/ResourceCache.h"

namespace Aurora
{
    AudioSource::AudioSource(EngineContext* engineContext, Entity* entity, uint32_t componentID) : IComponent(engineContext, entity, componentID)
    {
        m_PlayOnStart = true;
        m_IsLooping = false;
        m_Volume = 1.0f;
    }

    void AudioSource::Initialize()
    {
        if (!m_AudioClip)
        {
            return;
        }
    }

    void AudioSource::Start()
    {
        if (m_PlayOnStart)
        {
            m_AudioClip->Play();
        }
    }

    void AudioSource::Stop()
    {
        _Stop();
    }

    void AudioSource::Remove()
    {
        if (!m_AudioClip)
        {
            return;
        }

        m_AudioClip->Stop();
    }

    void AudioSource::Serialize(BinarySerializer* binarySerializer)
    {
        ////
    }

    void AudioSource::Deserialize(BinarySerializer* binaryDeserializer)
    {
        ////
    }

    void AudioSource::SetAudioClip(const std::string& filePath)
    {
        // Create and load the audio clip.
        m_AudioClip = std::make_shared<AudioClip>(m_EngineContext);
        if (m_AudioClip->LoadFromFile(filePath))
        {
            // In order for the component to guarentee serialization and deserialization.
            m_AudioClip = m_EngineContext->GetSubsystem<ResourceCache>()->CacheResource(m_AudioClip);
        }
    }

    std::string AudioSource::GetAudioClipName() const
    {
        return m_AudioClip ? m_AudioClip->GetResourceName() : "";
    }

    void AudioSource::SetVolume(float volume)
    {
        if (!m_AudioClip)
        {
            return;
        }

        m_Volume = Math::Helper::Clamp(volume, 0.0f, 1.0f);
        m_AudioClip->SetVolume(m_Volume);
    }

    void AudioSource::SetLoopState(const bool loopState)
    {
        m_IsLooping = loopState;
        m_AudioClip->SetLoop(loopState);

        //if (!loopState)
        //{
        //    m_AudioClip->Stop(); // Stop after setting loop.
        //}
    }

    bool AudioSource::Play() const
    {
        if (!m_AudioClip)
        {
            return false;
        }

        m_AudioClip->Play();
        m_AudioClip->SetLoop(m_IsLooping);
        m_AudioClip->SetVolume(m_Volume);
        return true;
    }

    bool AudioSource::_Stop() const
    {
        if (!m_AudioClip)
        {
            return false;
        }

        m_AudioClip->Stop();
        return true;
    }

    bool AudioSource::Pause() const
    {
        if (!m_AudioClip)
        {
            return false;
        }

        m_AudioClip->Pause();
        return true;
    }

    bool AudioSource::Unpause() const
    {
        if (!m_AudioClip)
        {
            return false;
        }

        m_AudioClip->Unpause();
        return true;
    }
}