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
        m_IsMuted = false;
        m_Volume = 1.0f;
        m_Priority = 128; // 0 represents the most important, while 256 represents least important. 128 is the default.
        m_Pitch = 1.0f;   // 1.0 for unmodified, 2.0 for double pitch (one octave up) and 0.5 for half pitch (one octave down).
        m_Pan = 0.0f;     // -1 for full left, 0 for center and 1 for full right.
        m_DistanceMinimum = 1.0f;
        m_DistanceMaximum = 30.0f;

        AURORA_REGISTER_ATTRIBUTE_VALUE_VALUE(m_AudioClip, std::shared_ptr<AudioClip>);
        AURORA_REGISTER_ATTRIBUTE_VALUE_VALUE(m_PlayOnStart, bool);
        AURORA_REGISTER_ATTRIBUTE_VALUE_SET(m_IsLooping, SetLoopState, bool);
        AURORA_REGISTER_ATTRIBUTE_VALUE_SET(m_IsMuted, SetMuteState, bool);
        AURORA_REGISTER_ATTRIBUTE_VALUE_SET(m_Volume, SetVolume, float);
        AURORA_REGISTER_ATTRIBUTE_VALUE_SET(m_Priority, SetPriority, int);
        AURORA_REGISTER_ATTRIBUTE_VALUE_SET(m_Pitch, SetPitch, float);
        AURORA_REGISTER_ATTRIBUTE_VALUE_SET(m_Pan, SetPan, float);
        AURORA_REGISTER_ATTRIBUTE_VALUE_SET(m_DistanceMinimum, SetRolloffDistanceMinimum, float);
        AURORA_REGISTER_ATTRIBUTE_VALUE_SET(m_DistanceMaximum, SetRolloffDistanceMaximum, float);
    }

    void AudioSource::Initialize()
    {
        if (!m_AudioClip)
        {
            return;
        }

        // Set the transform.
        m_AudioClip->SetTransform(GetEntity()->GetTransform());
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

    void AudioSource::Tick(float deltaTime)
    {
        if (!m_AudioClip)
        {
            return;
        }

        m_AudioClip->Update();

        /// Hmm...
        {
            // Set the transform.
            m_AudioClip->SetTransform(GetEntity()->GetTransform());
        }
    }

    void AudioSource::Serialize(BinarySerializer* binarySerializer)
    {
        binarySerializer->Write(m_PlayOnStart);
        binarySerializer->Write(m_IsMuted);
        binarySerializer->Write(m_IsLooping);
        binarySerializer->Write(m_Priority);
        binarySerializer->Write(m_Volume);
        binarySerializer->Write(m_Pitch);
        binarySerializer->Write(m_Pan);
        binarySerializer->Write(m_DistanceMinimum);
        binarySerializer->Write(m_DistanceMaximum);

        const bool hasAudioClip = m_AudioClip != nullptr;
        binarySerializer->Write(hasAudioClip);
        if (hasAudioClip)
        {
            binarySerializer->Write(m_AudioClip->GetResourceName());
        }
    }

    void AudioSource::Deserialize(BinarySerializer* binaryDeserializer)
    {
        binaryDeserializer->Read(&m_PlayOnStart);
        binaryDeserializer->Read(&m_IsMuted);
        binaryDeserializer->Read(&m_IsLooping);
        binaryDeserializer->Read(&m_Priority);
        binaryDeserializer->Read(&m_Volume);
        binaryDeserializer->Read(&m_Pitch);
        binaryDeserializer->Read(&m_Pan);
        binaryDeserializer->Read(&m_DistanceMinimum);
        binaryDeserializer->Read(&m_DistanceMaximum);

        if (binaryDeserializer->ReadAs<bool>())
        {
            m_AudioClip = m_EngineContext->GetSubsystem<ResourceCache>()->GetResourceByName<AudioClip>(binaryDeserializer->ReadAs<std::string>());
        }
    }

    void AudioSource::SetAudioClip(const std::string& filePath)
    {
        // Create and load the audio clip.
        m_AudioClip = std::make_shared<AudioClip>(m_EngineContext);

        if (m_AudioClip->LoadFromFile(filePath))
        {
            // In order for the component to guarentee serialization and deserialization.
            m_EngineContext->GetSubsystem<ResourceCache>()->CacheResource(m_AudioClip);
        }
    }

    std::string AudioSource::GetAudioClipPath() const
    {
        return m_AudioClip ? m_AudioClip->GetResourceFilePathNative() : "No Audio Clip Loaded...";
    }

    void AudioSource::SetVolume(float volume)
    {
        m_Volume = Math::Helper::Clamp(volume, 0.0f, 1.0f);

        if (!m_AudioClip)
        {
            return;
        }

        m_AudioClip->SetVolume(volume);
    }

    void AudioSource::SetLoopState(bool loopState)
    {
        m_IsLooping = loopState;

        if (loopState == m_IsLooping || !m_AudioClip)
        {
            return;
        }

        m_AudioClip->SetLoop(loopState);
    }

    void AudioSource::SetMuteState(bool muteState)
    {
        m_IsMuted = muteState;

        if (muteState == m_IsMuted || !m_AudioClip)
        {
            return;
        }

        m_AudioClip->SetMute(muteState);
    }

    void AudioSource::SetPriority(int priority)
    {
        m_Priority = Math::Helper::Clamp(priority, 0, 255);

        if (priority == m_Priority || !m_AudioClip)
        {
            return;
        }

        m_AudioClip->SetPriority(m_Priority);
    }

    void AudioSource::SetPitch(float pitch)
    {
        m_Pitch = Math::Helper::Clamp(pitch, 0.0f, 3.0f);

        if (!m_AudioClip)
        {
            return;
        }

        m_AudioClip->SetPitch(m_Pitch);
    }

    void AudioSource::SetRolloffDistanceMinimum(float distanceMinimum)
    {
        m_DistanceMinimum = distanceMinimum;

        if (!m_AudioClip)
        {
            return;
        }

        m_AudioClip->SetRolloffDistance(m_DistanceMinimum, m_DistanceMaximum);
    }

    void AudioSource::SetRolloffDistanceMaximum(float distanceMaximum)
    {
        m_DistanceMaximum = distanceMaximum;

        if (!m_AudioClip)
        {
            return;
        }

        m_AudioClip->SetRolloffDistance(m_DistanceMinimum, m_DistanceMaximum);
    }

    void AudioSource::SetPan(float pan)
    {
        m_Pan = Math::Helper::Clamp(pan, -1.0f, 1.0f);

        if (!m_AudioClip)
        {
            return;
        }

        m_AudioClip->SetPan(m_Pan);
    }

    bool AudioSource::Play() const
    {
        if (!m_AudioClip)
        {
            return false;
        }

        m_AudioClip->Play();
        m_AudioClip->SetLoop(m_IsLooping);
        m_AudioClip->SetMute(m_IsMuted);
        m_AudioClip->SetVolume(m_Volume);
        m_AudioClip->SetPitch(m_Pitch);
        m_AudioClip->SetPan(m_Pan);
        m_AudioClip->SetRolloffDistance(m_DistanceMinimum, m_DistanceMaximum);
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