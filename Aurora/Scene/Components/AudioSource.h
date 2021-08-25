#pragma once
#include "IComponent.h"

namespace Aurora
{
    class AudioClip;

    class AudioSource : public IComponent
    {
    public:
        AudioSource(EngineContext* engineContext, Entity* entity, uint32_t componentID = 0);
        ~AudioSource() = default;

        void Initialize() override;
        void Start() override;
        void Stop() override;
        void Remove() override;
        void Tick(float deltaTime) override;
        void Serialize(BinarySerializer* binarySerializer) override;
        void Deserialize(BinarySerializer* binaryDeserializer) override;

        void SetAudioClip(const std::string& filePath);
        std::string GetAudioClipPath() const;
        AudioClip* GetAudioClip() const { return m_AudioClip.get(); }

        // Volume
        float GetVolume() const { return m_Volume; }
        void SetVolume(float volume);

        // Play on Start
        bool GetPlayOnStartState() const { return m_PlayOnStart; }
        void SetPlayOnStartState(bool playOnStart) { m_PlayOnStart = playOnStart; }

        // Loop
        bool GetLoopState() const { return m_IsLooping; }
        void SetLoopState(bool loopState);

        // Mute
        bool GetMuteState() const { return m_IsMuted; }
        void SetMuteState(bool muteState);

        // Priority
        int GetPriority() const { return m_Priority; }
        void SetPriority(int priority);

        // Pitch
        float GetPitch() const { return m_Pitch; }
        void SetPitch(float pitch);

        // Rolloff Distance
        float GetRolloffDistanceMinimum() const { return m_DistanceMinimum; }
        void SetRolloffDistanceMinimum(float distanceMinimum);

        float GetRolloffDistanceMaximum() const { return m_DistanceMaximum; }
        void SetRolloffDistanceMaximum(float distanceMaximum);

        // Pan
        float GetPan() const { return m_Pan; }
        void SetPan(float pan);
            
        bool Play() const;
        bool _Stop() const;
        bool Pause() const;
        bool Unpause() const;

    private:
        std::shared_ptr<AudioClip> m_AudioClip;
        bool m_PlayOnStart = false;
        bool m_IsLooping;
        bool m_IsMuted;
        float m_Volume;
        float m_Pitch;
        float m_Pan;
        float m_DistanceMinimum;
        float m_DistanceMaximum;
        int m_Priority;
    };
}