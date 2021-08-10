#include "Aurora.h"
#include "Threading.h"
#include <thread>
#define NOMINMAX
#include <windows.h>
#include <sstream>
#include <assert.h>
#include <winerror.h>

namespace Aurora
{
    Threading::Threading(EngineContext* engineContext) : ISubsystem(engineContext)
    {

    }

    bool Threading::Initialize()
    {
        // Retrieve the total number of hardware threads in the system.
        m_ThreadCountTotal = std::thread::hardware_concurrency();

        // Calculate the actual number of worker threads we want (-1 for the main thread, or any subsystems we wish to reserve a thread for).
        m_ThreadCountSupported = m_ThreadCountTotal - 1;
        m_ThreadNames[std::this_thread::get_id()] = "Main";

        // Create all our worker threads and put them to work.
        for (uint32_t threadID = 0; threadID < m_ThreadCountSupported; threadID++)
        {
            std::thread worker([this, threadID]
            {
                while (true)
                {
                    if (!TaskLoop())
                    {
                        // No tasks avaliable.
                        std::unique_lock<std::mutex> lock(m_WakeMutex);
                        m_WakeCondition.wait(lock); // Unlock and allow execution to proceed once condition is met.
                    }
                }
            });

            m_ThreadNames[worker.get_id()] = "Worker_" + std::to_string(threadID);
            AURORA_INFO(LogLayer::Engine, "Thread Initiated: %s", m_ThreadNames[worker.get_id()].c_str());

            // Platform specific setup. We will use the following to name our threads officially, allowing them to show up in Visual Studio Debugger.
            HANDLE hHandle = (HANDLE)worker.native_handle();
            std::wstringstream wideStringName;
            wideStringName << m_ThreadNames[worker.get_id()].c_str();
            SetThreadDescription(hHandle, wideStringName.str().c_str());

            worker.detach(); // Seperates the thread of execution, allowing execution to continue independantly. https://stackoverflow.com/questions/22803600/when-should-i-use-stdthreaddetache5
        }

        return true;
    }

    // The meat of our library. This function runs across all threads and is used to execute our jobs.
    bool Threading::TaskLoop()
    {
        Job job;
        if (m_JobQueue.pop_front(job))
        {
            JobInformation jobInformation = {};
            jobInformation.m_GroupID = job.m_GroupID;

            for (uint32_t i = job.m_GroupJobOffset; i < job.m_GroupJobEnd; ++i)
            {
                jobInformation.m_JobIndex = i;
                jobInformation.m_GroupIndex = i - job.m_GroupJobOffset;
                jobInformation.m_IsFirstJobInGroup = (i == job.m_GroupJobOffset);
                jobInformation.m_IsLastJobInGroup = (i == job.m_GroupJobEnd - 1);
                job.m_Job(jobInformation);
            }

            m_Counter.fetch_sub(1);
            return true;
        }

        return false;
    }

    uint32_t Threading::CalculateDispatchJobCount(uint32_t jobCount, uint32_t groupSize)
    {
        // Calculates the amount of jobs to dispatch for this specific task.
        return (jobCount + groupSize - 1) / groupSize;
    }

    void Threading::Dispatch(uint32_t jobCount, uint32_t groupSize, const std::function<void(JobInformation)>& jobInformation)
    {
        if (jobCount == 0 || groupSize == 0)
        {
            return;
        }

        const uint32_t groupCount = CalculateDispatchJobCount(jobCount, groupSize); // Tells us how many worker threads (or groups) will be activated to handle the queue.

        // Update context state.
        m_Counter.fetch_add(groupCount);

        Job job;
        job.m_Job = jobInformation;

        for (uint32_t groupID = 0; groupID < groupCount; ++groupID)
        {
            // For each group, generate one job to handle it.
            job.m_GroupID = groupID;
            job.m_GroupJobOffset = groupID * groupSize;
            job.m_GroupJobEnd = std::min(job.m_GroupJobOffset + groupSize, jobCount); // The last task belonging to this job.

            // Try to push this job until it is pushed successfully.
            while (!m_JobQueue.push_back(job))
            {
                m_WakeCondition.notify_all();
                TaskLoop();
            }
        }

        // Wake up any threads that might be sleeping.
        m_WakeCondition.notify_all();
    }

    void Threading::Execute(const std::function<void(JobInformation)>& jobInformation)
    {
        m_Counter.fetch_add(1);

        Job job;
        job.m_Job = jobInformation;
        job.m_GroupID = 0;
        job.m_GroupJobOffset = 0;
        job.m_GroupJobEnd = 1;

        // Try to push a new job. If we're unable to do so, we continue to wake threads up to execute existing tasks to make space for the new job.
        while (!m_JobQueue.push_back(job))
        {
            m_WakeCondition.notify_all();
            TaskLoop();
        }

        // Once pushed, we wake up a thread to execute the pushed thread.
        m_WakeCondition.notify_one();
    }

    bool Threading::IsBusy()
    {
        // Whenever the context label is greater than 0, it means there are tasks to be completed.
        return m_Counter.load() > 0;
    }

    void Threading::Wait()
    {
        // Wake any threads that might be sleeping to execute all tasks.
        m_WakeCondition.notify_all();

        // Waiting will also put the current thread to good use by working on a job if it can. Disable this to not make our main thread work.
        if (m_UseMainThreadForTasks)
        {
            TaskLoop();
        }
    }

    uint32_t Threading::GetThreadCountAvaliable()
    {
        if (IsBusy())
        {
            return std::min((unsigned int)0, m_ThreadCountSupported - m_Counter.load());
        }

        return m_ThreadCountSupported;
    }

    // ====== Unit Testing ======

    void Spin(float milliseconds)
    {
        milliseconds /= 1000.0f;  // Convert to seconds.
        std::chrono::high_resolution_clock::time_point timePoint1 = std::chrono::high_resolution_clock::now();
        double ms = 0;

        while (ms < milliseconds)
        {
            std::chrono::high_resolution_clock::time_point timePoint2 = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(timePoint2 - timePoint1);
            ms = timeSpan.count();
        }
    }

    struct MiniStopwatch
    {
        std::string m_ProcessName;
        std::chrono::high_resolution_clock::time_point m_Start;

        MiniStopwatch(const std::string& processName) : m_ProcessName(processName), m_Start(std::chrono::high_resolution_clock::now()) {}

        void Output()
        {
            std::chrono::steady_clock::time_point endTimePoint = std::chrono::high_resolution_clock::now();
            const std::chrono::duration<double, std::milli> milliseconds = endTimePoint - m_Start;

            AURORA_INFO(LogLayer::Engine, "%s: Computed in %.3f milliseconds.", m_ProcessName.c_str(), static_cast<float>(milliseconds.count()));
        }

        ~MiniStopwatch()
        {
            Output();
        }
    };

    void Threading::SingularTaskUnitTest()
    {
        MiniStopwatch T = MiniStopwatch("Execute Test");
        Execute([](JobInformation jobArguments) { Spin(100); });

        Wait();
    }

    void Threading::LoopingTaskUnitTest()
    {
        uint32_t dataCount = 1000;

        {
            MiniStopwatch T = MiniStopwatch("Dispatch Test");

            const uint32_t groupSize = 100; // Split each job into 1000 tasks.
            Dispatch(dataCount, groupSize, [&dataCount](JobInformation arguments)
            {
                Spin(10);
            });

            Wait();
        }
    }
}