#pragma once
#include <string>
#include <fstream>
#include <algorithm>
#include <mutex>
#include <chrono>

namespace Aurora
{
    #define AURORA_PROFILE_SCOPE(name) InstrumentorTimer timer##__LINE__(name)
    #define AURORA_PROFILE_FUNCTION() AURORA_PROFILE_SCOPE(__FUNCSIG__)


    struct ProfileResult
    {
        const std::string m_ProfileName;
        long long m_Start, m_End;
        uint32_t m_ThreadID;
    };

    class Instrumentor
    {
    public:
        static Instrumentor& GetInstance()
        {
            static Instrumentor m_Instrumentor;
            return m_Instrumentor;
        }

        ~Instrumentor()
        {

        }

        void BeginSession(const std::string& sessionName)
        {
            if (m_IsSessionActive)
            {
                EndSession();
            }

            m_IsSessionActive = true;
            m_OutputStream.open("../ProfilerLogs/" + sessionName + ".json");
            WriteHeader();
            m_SessionName = sessionName;
        }

        void EndSession()
        {
            if (!m_IsSessionActive)
            {
                return;
            }

            m_IsSessionActive = false;
            WriteFooter();
            m_OutputStream.close();
            m_ProfileCount = 0;
        }

        void WriteHeader()
        {
            m_OutputStream << "{\"otherData\": {},\"traceEvents\":[";
        }

        void WriteProfile(const ProfileResult& result)
        {
            std::lock_guard<std::mutex> lock(m_Lock);

            if (m_ProfileCount++ > 0)
            {
                m_OutputStream << ",";
            }

            std::string name = result.m_ProfileName;
            std::replace(name.begin(), name.end(), '"', '\'');

            m_OutputStream << "{";
            m_OutputStream << "\"cat\":\"function\",";
            m_OutputStream << "\"dur\":" << (result.m_End - result.m_Start) << ',';
            m_OutputStream << "\"name\":\"" << name << "\",";
            m_OutputStream << "\"ph\":\"X\",";
            m_OutputStream << "\"pid\":0,";
            m_OutputStream << "\"tid\":" << result.m_ThreadID << ",";
            m_OutputStream << "\"ts\":" << result.m_Start;
            m_OutputStream << "}";
        }

        void WriteFooter()
        {
            m_OutputStream << "]}";
        }

    private:
        Instrumentor()
        {

        }

    private:
        std::string m_SessionName = "None";
        std::ofstream m_OutputStream;
        int m_ProfileCount = 0;
        std::mutex m_Lock;
        bool m_IsSessionActive = false;
    };

    class InstrumentorTimer
    {
    public:
        InstrumentorTimer(const std::string& profileName) : m_Result({profileName, 0, 0, 0}), m_HasStopped(false)
        {
            m_StartTimepoint = std::chrono::high_resolution_clock::now();
        }

        ~InstrumentorTimer()
        {
            if (!m_HasStopped)
            {
                Stop();
            }
        }

        void Stop()
        {
            auto endTimepoint = std::chrono::high_resolution_clock::now();

            m_Result.m_Start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
            m_Result.m_End = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();
            m_Result.m_ThreadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
            Instrumentor::GetInstance().WriteProfile(m_Result);

            m_HasStopped = true;
        }

    private:
        ProfileResult m_Result;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
        bool m_HasStopped;
    };
}