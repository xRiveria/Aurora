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
        void Serialize(BinarySerializer* binarySerializer) override;
        void Deserialize(BinarySerializer* binaryDeserializer) override;

        void SetAudioClip(const std::string& filePath);
        std::string GetAudioClipName() const;
        AudioClip* GetAudioClip() const { return m_AudioClip.get(); }

        // Volume
        float GetVolume() const { return m_Volume; }
        void SetVolume(float volume);

        // Play on Start
        bool GetPlayOnStartState() const { return m_PlayOnStart; }
        void SetPlayOnStart(const bool playOnStart) { m_PlayOnStart = playOnStart; }

        // Loop
        bool GetLoopState() const { return m_IsLooping; }
        void SetLoopState(const bool loopState);

        bool Play() const;
        bool _Stop() const;
        bool Pause() const;
        bool Unpause() const;

    private:
        std::shared_ptr<AudioClip> m_AudioClip;
        bool m_PlayOnStart = false;
        float m_Volume;
        bool m_IsLooping;
    };
}