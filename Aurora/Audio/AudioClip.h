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
    enum class Audio_LoadingMode
    {
        // Assets are completely decompressed into memory when loaded. This uses more memory than streaming, but requires very little CPU when playing assets.
        // Suitable for short assets that are played frequently and mobile platforms.
        Memory,
        // Assets are continously loaded from disk into a memory buffer while playing. Reduces memory requires to load and play assets, but simultaenous streams are limited. 
        // Suitable for music/background ambiences.
        Stream
    };

    class AudioClip : public AuroraResource
    {
    public:
        AudioClip(EngineContext* engineContext);
        ~AudioClip();

        bool LoadFromFile(const std::string& filePath);
        bool SaveToFile(const std::string& filePath);

        bool Play();
        bool Stop();
        bool Pause();
        bool Unpause();

        // Sets the volume 
        bool SetVolume(float volume);

        // Sets the mute.
        bool SetMute(bool muteState);

        // Set looping state.
        bool SetLoop(bool loopState);
        
        // Set priority.
        bool SetPriority(int priority);

        // Set pitch.
        bool SetPitch(float pitch);

        // Set pan.
        bool SetPan(float pan);

        // Set distances.
        bool SetRolloffDistance(float distanceMinimum, float distanceMaximum);

        // Allows for 3D attributes.
        void SetTransform(Transform* transform) { m_Transform = transform; }

        // Set per frame to use the 3D attributes of the transform.
        bool Update();

        bool IsChannelValid() const;
        int GetSoundMode() const;       

        bool IsPlaying();

    private:
        bool CreateSoundInternal(const std::string& filePath);
        bool CreateStreamInternal(const std::string& filePath);

    private:
        Transform* m_Transform = nullptr;

        FMOD::System* m_AudioContext     = nullptr;
        FMOD::Channel* m_ChannelInternal = nullptr;
        FMOD::Sound* m_SoundInternal     = nullptr;
        
        int m_ModeRolloff;
        int m_ModeLoop;
        bool m_IsPlaying = false;
        Audio_LoadingMode m_LoadingMode = Audio_LoadingMode::Memory;
    };
}