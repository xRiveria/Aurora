#pragma once
#include <chrono>
#include "../Log/Log.h"
#include "../Profiler/Profiler.h"

namespace Aurora
{
    class Stopwatch
    {
    public:
        Stopwatch(const std::string& queryName, bool logToProfiler) : m_QueryName(queryName), m_LogToProfiler(logToProfiler)
        {
            StartTimer();
        }

        ~Stopwatch()
        {
            if (m_IsWatchRunning)
            {
                OutputTimer();
                m_IsWatchRunning = false;
            }
        }

        inline void StartTimer()
        {
            m_IsWatchRunning = true;
            m_Start = std::chrono::high_resolution_clock::now();
        }

        inline void OutputTimer()
        {
            char buffer[256];
            sprintf_s(buffer, "%s: %.3fms", m_QueryName.c_str(), GetElapsedTimeInMilliseconds());
            if (m_LogToProfiler)
            {
                Profiler::GetInstance().AddEntry(std::string(buffer));
            }
        }

        inline float GetElapsedTimeInSeconds() const
        {
            const std::chrono::duration<double, std::milli> milliseconds = std::chrono::high_resolution_clock::now() - m_Start;
            return static_cast<float>(milliseconds.count() / 1000);
        }

        inline float GetElapsedTimeInMilliseconds() const
        {
            const std::chrono::duration<double, std::milli> milliseconds = std::chrono::high_resolution_clock::now() - m_Start;
            return static_cast<float>(milliseconds.count());
        }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
        std::string m_QueryName;
        bool m_IsWatchRunning = false;
        bool m_LogToProfiler = false;
    };
}