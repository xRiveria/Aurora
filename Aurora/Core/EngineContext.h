#pragma once
#include <vector>
#include <memory>
#include "../Log/Log.h"
#include "ISubsystem.h"
#include "Engine.h"

namespace Aurora
{
    struct _Subsystem
    {
        _Subsystem(const std::shared_ptr<ISubsystem>& subsystem) : m_Pointer(subsystem)
        {

        }

        std::shared_ptr<ISubsystem> m_Pointer;
    };

    class EngineContext
    {
    public:
        EngineContext(Engine* engine) : m_Engine(engine)
        {

        }

        ~EngineContext()
        {
            // Loop in reverse registration order to avoid dependency conflicts.
            for (size_t i = m_Subsystems.size() - 1; i > 0; i--)
            {
                m_Subsystems[i].m_Pointer.reset();
            }

            m_Subsystems.clear();
        }

        template<typename T>
        void RegisterSubsystem()
        {
            m_Subsystems.emplace_back(std::make_shared<T>(this));
        }

        template<typename T>
        T* GetSubsystem() const
        {
            for (const auto& subsystem : m_Subsystems)
            {
                if (subsystem.m_Pointer)
                {
                    if (typeid(T) == typeid(*subsystem.m_Pointer))
                    {
                        return static_cast<T*>(subsystem.m_Pointer.get());
                    }
                }
            }

            return nullptr;
        }

        void Initialize()
        {
            std::vector<uint8_t> failedSubsystems;

            // Initialize
            for (uint8_t i = 0; i < static_cast<uint8_t>(m_Subsystems.size()); i++)
            {
                if (!m_Subsystems[i].m_Pointer->Initialize())
                {
                    failedSubsystems.emplace_back(i);
                    AURORA_ERROR("Failed to initialize %s Subsystem.", typeid(*m_Subsystems[i].m_Pointer).name());
                }

                AURORA_INFO("Successfully initialized %s Subsystem.", typeid(*m_Subsystems[i].m_Pointer).name());
            }

            // Remove the systems that failed.
            for (const uint8_t subsystemIndex : failedSubsystems)
            {
                m_Subsystems.erase(m_Subsystems.begin() + subsystemIndex);
            }
        }

        void Tick(float deltaTime)
        {
            for (const _Subsystem& subsystem : m_Subsystems)
            {
                subsystem.m_Pointer->Tick(deltaTime);
            }
        }

        void Shutdown()
        {
            for (const _Subsystem& subsystem : m_Subsystems)
            {
                subsystem.m_Pointer->Shutdown();
            }
        }

        std::vector<_Subsystem>::iterator begin() { return m_Subsystems.begin(); }
        std::vector<_Subsystem>::iterator end() { return m_Subsystems.end(); }

        Engine* GetEngine() { return m_Engine; }

    private:
        std::vector<_Subsystem> m_Subsystems;
        Engine* m_Engine = nullptr;
    };
}