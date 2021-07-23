#pragma once
#include "../Time/Stopwatch.h"

namespace Aurora
{
    class Profiler
    {
    public:
        static Profiler& GetInstance()
        {
            static Profiler m_ProfilerInstance;
            return m_ProfilerInstance;
        }

        void Reset()
        {
            m_StopwatchOutputs.clear();
        }

        void AddEntry(const std::string& stopwatchOutput)
        {
            m_StopwatchOutputs.push_back(stopwatchOutput);
        }

        std::vector<std::string>& GetEntries()
        {
            return m_StopwatchOutputs;
        }
       
    private:
        std::vector<std::string> m_StopwatchOutputs;
    };
}