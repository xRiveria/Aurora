#include "Aurora.h"
#include "FadeSystem.h"
#include "../Math/MathUtilities.h"

namespace Aurora
{
    void FadeSystem::Clear()
    {
        m_Opacity = 0;
        m_State = Fade_State::Fade_Finished;
    }

    void FadeSystem::Tick(float deltaTime)
    {
        if (!IsActive()) // We are considered to be "active" if the state isn't Fade_Finished.
        {
            return;
        }

        if (m_TargetFadeTimeInSeconds <= 0)
        {
            // Skip fade, simply launch the job.
            m_OnFadeFunction();
            m_State = Fade_State::Fade_Finished;
        }

        float time = m_Timer / m_TargetFadeTimeInSeconds; // Current time in respect to the target time.

        m_Timer += Math::Clamp(deltaTime, 0, 0.33f);

        if (m_State == Fade_State::Fade_In)
        {
            m_Opacity = Math::Lerp(0.0f, 1.0f, time);
            if (time >= 1.0f)
            {
                m_State = Fade_State::Fade_Middle;
                m_Opacity = 1.0f;
            }
        }
        else if (m_State == Fade_State::Fade_Middle) // We start to fade out once we've reached the middle.
        {
            m_State = Fade_State::Fade_Out;
            m_Opacity = 1.0f;
            m_OnFadeFunction();
            m_Timer = 0;
        }
        else if (m_State == Fade_State::Fade_Out)
        {
            m_Opacity = Math::Lerp(1.0f, 0.0f, time); // Begin opacity fade out with the lerp.
            if (time >= 1.0f)
            {
                m_State = Fade_State::Fade_Finished;
                m_Opacity = 0.0f;
            }
        }
        else if (m_State == Fade_State::Fade_Finished)
        {
            m_Opacity = 0.0f;
            m_OnFadeFunction = [] {};
        }
    }
}