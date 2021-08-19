#include "Aurora.h"
#include "ISubsystem.h"
#include "EngineContext.h"
#include "../Window/WindowContext.h"
#include "Settings.h"
#include "../Threading/Threading.h"
#include "../Time/Timer.h"
#include "../Input/Input.h"
#include "../Renderer/Renderer.h"
#include "../Scene/World.h"
#include "../Physics/Physics.h"
#include "../Resource/ResourceCache.h"
#include "../Input/InputEvents/InputEvent.h"


namespace Aurora
{
    Engine::Engine()
    {
        // Flags
        m_EngineFlags |= EngineFlag::EngineFlag_TickPhysics;
        m_EngineFlags |= EngineFlag::EngineFlag_TickGame;

        // Create Engine Context
        m_EngineContext = std::make_shared<EngineContext>(this);

        // Register Subsystem
        m_EngineContext->RegisterSubsystem<Timer>();
        m_EngineContext->RegisterSubsystem<Threading>();
        m_EngineContext->RegisterSubsystem<Settings>();
        m_EngineContext->RegisterSubsystem<WindowContext>();
        m_EngineContext->RegisterSubsystem<Input>();
        m_EngineContext->RegisterSubsystem<Physics>();
        m_EngineContext->RegisterSubsystem<World>();
        m_EngineContext->RegisterSubsystem<Renderer>();
        m_EngineContext->RegisterSubsystem<ResourceCache>();

        // Initialize Subsystem
        m_EngineContext->Initialize();

        m_EngineContext->GetSubsystem<WindowContext>()->SetEventCallback(std::bind(&Engine::OnEvent, this, std::placeholders::_1));
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

    void Engine::OnEvent(InputEvent& inputEvent)
    {
        // Loop through our systems in reverse.
        for (auto it = m_EngineContext->end(); it != m_EngineContext->begin();)
        {
            if (inputEvent.IsEventHandled)
            {
                break;
            }

            (*--it).m_Pointer->OnEvent(inputEvent);
        }
    }
}