#pragma once
#include <string>

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
        static uint32_t GenerateObjectID() { return ++g_ID_Count; }  // Generates and returns an object ID.

        /// Should we track GPU and CPU sizes?

        // Execution Context.
        EngineContext* GetEngineContext() const { return m_EngineContext; }
        
    protected:
        std::string m_ObjectName;
        uint32_t m_ObjectID;

        EngineContext* m_EngineContext = nullptr;
    };
}