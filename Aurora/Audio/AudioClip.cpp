#include "Aurora.h"
#include <FMOD/fmod.hpp>
#include "AudioUtilities.h"
#include "Audio.h"
#include "AudioClip.h"

namespace Aurora
{
    AudioClip::AudioClip(EngineContext* engineContext) : AuroraResource(engineContext, ResourceType::ResourceType_Audio)
    {
        m_AudioContext = m_EngineContext->GetSubsystem<Audio>()->GetAudioContext();
        m_MinimumDistance = 1.0f;
        m_MaximumDistance = 10000.0f;
        m_ModeRolloff = FMOD_3D_LINEARROLLOFF;
        m_ModeLoop = FMOD_LOOP_OFF;
    }

    AudioClip::~AudioClip()
    {
        if (m_SoundInternal != nullptr)
        {
            ParseResult_Audio(m_SoundInternal->release());
        }
    }

    bool AudioClip::LoadFromFile(const std::string& filePath) 
    {
        m_SoundInternal = nullptr;
        m_ChannelInternal = nullptr;

        // Native
        if (FileSystem::GetExtensionFromFilePath(filePath) == EXTENSION_AUDIO)
        {

        }
        else // Foreign format.
        {
            SetResourceFilePath(filePath);
        }

        return (m_LoadingMode == Audio_LoadingMode::Memory) ? CreateSoundInternal(GetResourceFilePath()) : CreateStreamInternal(GetResourceFilePath());
    }

    bool AudioClip::SaveToFile(const std::string& filePath) 
    {
        return false;
    }

    bool AudioClip::Play()
    {
        if (IsChannelValid())
        {
            bool isPlaying = false;
            if (!ParseResult_Audio(m_ChannelInternal->isPlaying(&isPlaying)))
            {
                return false;
            }

            // Don't bother playing again.
            if (isPlaying)
            {
                return true;
            }
        }

        // Start playing our sound.
        return ParseResult_Audio(m_AudioContext->playSound(m_SoundInternal, nullptr, false, &m_ChannelInternal));
    }

    bool AudioClip::Stop()
    {
        if (!IsChannelValid())
        {
            return true;
        }

        // If nothing is playing, don't bother.
        if (!IsPlaying())
        {
            return true;
        }

        // Stop the sound.
        if (!ParseResult_Audio(m_ChannelInternal->stop()))
        {
            m_ChannelInternal = nullptr;
            return false;
        }

        m_ChannelInternal = nullptr;
        return true;
    }

    bool AudioClip::Pause()
    {
        if (!IsChannelValid())
        {
            return true;
        }

        // Get sound paused state.
        bool isPaused = false;
        if (!ParseResult_Audio(m_ChannelInternal->getPaused(&isPaused)))
        {
            return false;
        }

        // If its already paused, don't bother.
        if (isPaused)
        {
            return true;
        }

        // Pause the sound.     
        return ParseResult_Audio(m_ChannelInternal->setPaused(true));
    }

    bool AudioClip::Unpause()
    {
        if (!IsChannelValid())
        {
            return true;
        }

        // Get sound paused state.
        bool isPaused = false;
        if (!ParseResult_Audio(m_ChannelInternal->getPaused(&isPaused)))
        {
            return false;
        }

        // If its not paused, don't bother.
        if (!isPaused)
        {
            return true;
        }

        // Unpause the sound.
        return ParseResult_Audio(m_ChannelInternal->setPaused(false));
    }

    bool AudioClip::SetVolume(float volume)
    {
        if (!IsChannelValid())
        {
            return false;
        }

        // The value is clamped by our Audio Source. 0 = No Volume, 1 = Max Volume
        return ParseResult_Audio(m_ChannelInternal->setVolume(volume));
    }

    bool AudioClip::SetMute(bool muteState)
    {
        if (!IsChannelValid())
        {
            return false;
        }

        return ParseResult_Audio(m_ChannelInternal->setMute(muteState));
    }

    bool AudioClip::SetLoop(bool loopState)
    {
        m_ModeLoop = loopState ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;

        if (!m_SoundInternal)
        {
            return false;
        }

        // Infinite looping.
        if (loopState)
        {
            m_SoundInternal->setLoopCount(-1); // Set the number of times to loop before stopping. (0 for one shot, 1 for loop once then stop, -1 represents loop forever).
        }

        return ParseResult_Audio(m_SoundInternal->setMode(GetSoundMode()));
    }

    bool AudioClip::IsChannelValid() const
    {
        if (!m_ChannelInternal)
        {
            return false;
        }

        // Do a query and see if it fails.
        bool value;
        return m_ChannelInternal->isPlaying(&value) == FMOD_OK;
    }

    int AudioClip::GetSoundMode() const
    {
        return FMOD_3D | m_ModeLoop | m_ModeRolloff;
    }

    bool AudioClip::IsPlaying()
    {
        if (!m_ChannelInternal)
        {
            return false;
        }

        bool isPlaying = false;
        ParseResult_Audio(m_ChannelInternal->isPlaying(&isPlaying));

        return isPlaying;
    }

    bool AudioClip::CreateSoundInternal(const std::string& filePath)
    {
        if (!ParseResult_Audio(m_AudioContext->createSound(filePath.c_str(), GetSoundMode(), nullptr, &m_SoundInternal)))
        {
            return false;
        }

        AURORA_INFO(LogLayer::Audio, "Audio Created!");

        // Set 3D distances.
        return ParseResult_Audio(m_SoundInternal->set3DMinMaxDistance(m_MinimumDistance, m_MaximumDistance));
    }

    bool AudioClip::CreateStreamInternal(const std::string& filePath)
    {
        return true;
    }
}