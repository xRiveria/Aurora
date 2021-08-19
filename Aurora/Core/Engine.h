#pragma once
#include <unordered_map>
#include <memory>
#include "Core.h"

namespace Aurora
{
    class EngineContext;
    class InputEvent;
    class ISubsystem;

    enum EngineFlag : uint32_t
    {
        EngineFlag_TickPhysics = 1 << 0,    // Should the physics system tick?
        EngineFlag_TickGame    = 1 << 1     // Is the game running in editor or game mode?
    };

    class Engine
    {
    public:
        Engine();
        ~Engine();

        void Tick() const; // Supplies deltaTime from the Time subsystem.
        void OnEvent(InputEvent& inputEvent);

        // Flags
        uint32_t GetEngineFlags() const { return m_EngineFlags; }
        void EngineFlag_Enable(const EngineFlag flag) { m_EngineFlags |= flag; }
        void EngineFlag_Disable(const EngineFlag flag) { m_EngineFlags &= ~flag; }   
        void EngineFlag_Toggle(const EngineFlag flag) { m_EngineFlags = !EngineFlag_IsSet(flag) ? m_EngineFlags | flag : m_EngineFlags & ~flag; }
        bool EngineFlag_IsSet(const EngineFlag flag) const { return m_EngineFlags & flag; }
        void EngineFlags_SetAll(const uint32_t flags) { m_EngineFlags = flags; }

        EngineContext* GetEngineContext() const { return m_EngineContext.get(); }

    private:
        uint32_t m_EngineFlags = 0;
        std::shared_ptr<EngineContext> m_EngineContext;
    };
}
