#include "Aurora.h"
#include "../Window/WindowContext.h"
#include "../Time/Timer.h"
#include "EngineContext.h"

namespace Aurora
{
    Engine::Engine()
    {
        // Create Engine Context
        m_EngineContext = std::make_shared<EngineContext>(this);

        // Register Subsystem
        m_EngineContext->RegisterSubsystem<Timer>();
        m_EngineContext->RegisterSubsystem<WindowContext>();

        // Initialize Subsystem
        m_EngineContext->Initialize();
        m_EngineContext->GetSubsystem<WindowContext>()->Create({ "Aurora Engine", 1280, 720 });
        m_EngineContext->GetSubsystem<WindowContext>()->SetCurrentContext(0); // Index 0 is guarenteed to be our render window. Alternatively, use GetRenderWindow().
    }

    Engine::~Engine()
    {
        m_EngineContext->Shutdown();
    }

    void Engine::Tick() const
    {
        float deltaTime = m_EngineContext->GetSubsystem<Timer>()->GetDeltaTimeInSeconds();
        // AURORA_INFO("Delta Time (Seconds): %f", deltaTime);

        m_EngineContext->Tick(deltaTime);     
    }
}