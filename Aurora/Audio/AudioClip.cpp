#include "Aurora.h"
#include <FMOD/fmod.hpp>
#include "AudioUtilities.h"
#include "Audio.h"
#include "AudioClip.h"

namespace Aurora
{
    AudioClip::AudioClip(EngineContext* engineContext) : AuroraResource(engineContext, ResourceType::ResourceType_Audio)
    {
        m_SoundInternal = nullptr;
        m_ChannelInternal = nullptr;
        m_AudioContext = m_EngineContext->GetSubsystem<Audio>()->GetAudioContext();
        m_ModeRolloff = FMOD_3D_LINEARROLLOFF;
        m_ModeLoop = FMOD_LOOP_OFF;
        m_Transform = nullptr;
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
            std::string retrievedPath;

            std::unique_ptr<FileSerializer> fileDerializer = std::make_unique<FileSerializer>(m_EngineContext);
            if (fileDerializer->LoadFromFile(filePath))
            {
                fileDerializer->GetProperty("File_Path", &retrievedPath);
                SetResourceFilePath(retrievedPath);
                AURORA_INFO(LogLayer::Serialization, "Found Audio and Deserialized!");
            }
        }
        else // Foreign format.
        {
            SetResourceFilePath(filePath);
        }

        // We recognize each sound in FMOD as a unique object so they don't interfere with each other's output.
        return (m_LoadingMode == Audio_LoadingMode::Memory) ? CreateSoundInternal(GetResourceFilePath()) : CreateStreamInternal(GetResourceFilePath());
    }

    bool AudioClip::SaveToFile(const std::string& filePath) 
    {
        std::unique_ptr<FileSerializer> fileSerializer = std::make_unique<FileSerializer>(m_EngineContext);
        if (fileSerializer->BeginSerialization("Audio"))
        {
            fileSerializer->AddProperty("File_Path", GetResourceFilePath());
        }

        return fileSerializer->EndSerialization(GetResourceFilePathNative());
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
        std::cout << "Play!";
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

    bool AudioClip::SetPriority(int priority)
    {
        if (!IsChannelValid())
        {
            return false;
        }

        return ParseResult_Audio(m_ChannelInternal->setPriority(priority));
    }

    bool AudioClip::SetPitch(float pitch)
    {
        if (!IsChannelValid())
        {
            return false;
        }

        return ParseResult_Audio(m_ChannelInternal->setPitch(pitch));
    }

    bool AudioClip::SetPan(float pan)
    {
        if (!IsChannelValid())
        {
            return false;
        }

        return ParseResult_Audio(m_ChannelInternal->setPan(pan));
    }

    bool AudioClip::SetRolloffDistance(float distanceMinimum, float distanceMaximum)
    {
        if (!m_SoundInternal)
        {
            return false;
        }

        std::cout << distanceMaximum;
        return ParseResult_Audio(m_SoundInternal->set3DMinMaxDistance(distanceMinimum, distanceMaximum));
    }

    bool AudioClip::Update()
    {
        if (!IsChannelValid() || !m_Transform)
        {
            return true;
        }

        const XMFLOAT3 position = m_Transform->GetPosition();

        FMOD_VECTOR fModPosition = { position.x, position.y, position.z };
        FMOD_VECTOR fModVelocity = { 0.0f, 0.0f, 0.0f };

        // Set 3D attributes.
        if (!ParseResult_Audio(m_ChannelInternal->set3DAttributes(&fModPosition, &fModVelocity)))
        {
            m_ChannelInternal = nullptr;
            return false;
        }

        return true;
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
        bool isPaused = false;
        m_ChannelInternal->isPlaying(&isPlaying);
        m_ChannelInternal->getPaused(&isPaused);

        return isPlaying || isPaused;
    }

    bool AudioClip::CreateSoundInternal(const std::string& filePath)
    {
        AURORA_INFO(LogLayer::Audio, "Audio Created!");

        return ParseResult_Audio(m_AudioContext->createSound(filePath.c_str(), GetSoundMode(), nullptr, &m_SoundInternal));
    }

    bool AudioClip::CreateStreamInternal(const std::string& filePath)
    {
        return true;
    }
}