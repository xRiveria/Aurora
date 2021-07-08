#pragma once
#include <unordered_map>
#include <memory>
#include "../Misc/FadeSystem.h"

namespace Aurora
{
    class EngineContext;

    class Engine
    {
    public:
        Engine();
        ~Engine();

        void Tick(); // Supplies deltaTime from the Time subsystem.
        void ComposeLoadingScreen();

        EngineContext* GetEngineContext() const { return m_EngineContext.get(); }

    private:
        // Fading system.
        FadeSystem m_FadeSystem;
        std::shared_ptr<EngineContext> m_EngineContext;
        bool m_IsLoadingComplete = false;
    };
}
