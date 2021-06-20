#include "Aurora.h"
#include "Timer.h"

namespace Aurora
{
    Timer::Timer(EngineContext* engineContext) : ISubsystem(engineContext)
    {
        m_TimeFrameStart = std::chrono::high_resolution_clock::now();
        m_TimeFrameEnd = std::chrono::high_resolution_clock::now();
    }

    void Timer::Tick(float deltaTime)
    {
        m_TimeFrameStart = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> computedDeltaTime = m_TimeFrameStart - m_TimeFrameEnd;

        m_TimeFrameEnd = std::chrono::high_resolution_clock::now();

        m_DeltaTimeInMilliseconds = static_cast<double>(computedDeltaTime.count());
    }
}