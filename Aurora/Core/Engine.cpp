#include "Aurora.h"
#include "../Window/WindowContext.h"
#include "../Time/Timer.h"
#include "../Input/Input.h"
#include "../Renderer/Renderer.h"
#include "../Scene/World.h"
#include "../Resource/ResourceCache.h"
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
        m_EngineContext->RegisterSubsystem<Input>();
        m_EngineContext->RegisterSubsystem<World>();
        m_EngineContext->RegisterSubsystem<Renderer>();
        m_EngineContext->RegisterSubsystem<ResourceCache>();

        // Initialize Subsystem
        m_EngineContext->Initialize();
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

        // Shortcut example of a copy function.
        if (m_EngineContext->GetSubsystem<Input>()->IsKeyPressed(AURORA_KEY_LEFT_CONTROL) && m_EngineContext->GetSubsystem<Input>()->IsKeyPressed(AURORA_KEY_C))
        {
            AURORA_INFO("Copied!");
        }
    }
}