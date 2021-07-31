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
        /// Might need improvement.
        static uint32_t GenerateObjectID() { return static_cast<uint32_t>(rand()); }  // Generates and returns an object ID. Seeded at the start of each run.

        /// Should we track GPU and CPU sizes?

        // Execution Context.
        EngineContext* GetEngineContext() const { return m_EngineContext; }

    protected:
        std::string m_ObjectName;
        uint32_t m_ObjectID;

        EngineContext* m_EngineContext = nullptr;
    };
}