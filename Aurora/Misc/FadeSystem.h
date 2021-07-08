#pragma once
#include <functional>
#include "AuroraColor.h"

namespace Aurora
{
    class FadeSystem
    {
    public:
        FadeSystem()
        {
            Clear();
        }

        void Clear();
        void Start(float seconds, AuroraColor color, std::function<void()> onFadeFunction)
        {
            m_TargetFadeTimeInSeconds = seconds;
            this->m_Color = color;
            m_Timer = 0;
            m_State = Fade_State::Fade_In;
            m_OnFadeFunction = onFadeFunction;
        }

        void Tick(float deltaTime);
        
        bool IsFaded() const { return m_State == Fade_State::Fade_Middle; }
        bool IsActive() const { return m_State != Fade_State::Fade_Finished; }

    private:
        enum Fade_State
        {
            Fade_In,
            Fade_Middle,
            Fade_Out,
            Fade_Finished
        } m_State = Fade_State::Fade_Finished;

        float m_Opacity = 0;
        float m_Timer = 0;
        float m_TargetFadeTimeInSeconds = 1.0f;
        AuroraColor m_Color = AuroraColor(0, 0, 0, 255);

        std::function<void()> m_OnFadeFunction = [] {};
    };
}