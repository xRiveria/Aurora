#pragma once
#include "../Core/ISubsystem.h"
#include <chrono>

/* Personal Notes

    - Frame Limiting? Users may set their target FPS as a result.
*/

namespace Aurora
{
    class EngineContext;

    class Timer : public ISubsystem
    {
    public:
        Timer(EngineContext* engineContext);
        ~Timer() = default;

        void Tick(float deltaTime) override;

        float GetDeltaTimeInSeconds() const { return static_cast<float>(m_DeltaTimeInMilliseconds / 1000.0); } // The time interval in seconds from the last frame to the current one.
        double GetDeltaTimeInMilliseconds() const { return m_DeltaTimeInMilliseconds; }

    private:
        std::chrono::high_resolution_clock::time_point m_TimeFrameStart;
        std::chrono::high_resolution_clock::time_point m_TimeFrameEnd;

        double m_DeltaTimeInMilliseconds = 0.0;
    };
}