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
#include "../Scripting/Scripting.h"
#include "../Resource/ResourceCache.h"
#include "../Audio/Audio.h"
#include "../Input/InputEvents/InputEvent.h"


namespace Aurora
{
    Engine::Engine()
    {
        AURORA_PROFILE_FUNCTION();

        // Flags
        m_EngineFlags |= EngineFlag::EngineFlag_TickPhysics;
        m_EngineFlags |= EngineFlag::EngineFlag_TickGame;

        // Create Engine Context
        m_EngineContext = std::make_shared<EngineContext>(this);

        // Register Subsystem
        m_EngineContext->RegisterSubsystem<Timer>();
        m_EngineContext->RegisterSubsystem<Threading>();
        m_EngineContext->RegisterSubsystem<Settings>();
        m_EngineContext->RegisterSubsystem<Audio>();
        m_EngineContext->RegisterSubsystem<WindowContext>();
        m_EngineContext->RegisterSubsystem<Input>();
        m_EngineContext->RegisterSubsystem<Physics>();
        m_EngineContext->RegisterSubsystem<World>();
        m_EngineContext->RegisterSubsystem<Renderer>();
        m_EngineContext->RegisterSubsystem<ResourceCache>();
        m_EngineContext->RegisterSubsystem<Scripting>();

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
        AURORA_PROFILE_FUNCTION();
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