#pragma once
#include <string>
#include <stdlib.h>

namespace Aurora
{
    class EngineContext;

    extern uint32_t g_ID_Count;

    class AuroraObject
    {

    public:
        AuroraObject(EngineContext* engineContext = nullptr);

        // Name & ID
        const std::string& GetObjectName() const { return m_ObjectName; }
        const uint32_t GetObjectID() const { return m_ObjectID; }

        void SetObjectID(const uint32_t newID) { m_ObjectID = newID; }
        void SetObjectName(const std::string& objectName) { m_ObjectName = objectName; }

        /// Might need improvement.
        static uint32_t GenerateObjectID() { return static_cast<uint32_t>(rand()); }  // Generates and returns an object ID. Seeded at the start of each run.

        // CPU & GPU Sizes
        const uint64_t GetObjectSizeCPU() const { return m_ObjectSizeCPU; }
        const uint64_t GetObjectSizeGPU() const { return m_ObjectSizeGPU; }

        // Execution Context.
        EngineContext* GetEngineContext() const { return m_EngineContext; }

    protected:
        std::string m_ObjectName;
        uint32_t m_ObjectID;

        // ==============================
        uint64_t m_ObjectSizeCPU;
        uint64_t m_ObjectSizeGPU;

        EngineContext* m_EngineContext = nullptr;
    };
}