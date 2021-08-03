#pragma once
#include <unordered_map>
#include <memory>
#include "Core.h"

namespace Aurora
{
    class EngineContext;
    class InputEvent;
    class ISubsystem;

    class Engine
    {
    public:
        Engine();
        ~Engine();

        void Tick() const; // Supplies deltaTime from the Time subsystem.
        void OnEvent(InputEvent& inputEvent);

        EngineContext* GetEngineContext() const { return m_EngineContext.get(); }

    private:
        std::shared_ptr<EngineContext> m_EngineContext;
    };
}
