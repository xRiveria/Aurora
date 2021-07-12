#pragma once

namespace Aurora
{
    class EngineContext;

    class ISubsystem : public std::enable_shared_from_this<ISubsystem>
    {
    public:
        ISubsystem(EngineContext* engineContext) : m_EngineContext(engineContext) {}
        virtual ~ISubsystem() = default;

        virtual bool Initialize() { return true; }
        virtual void Tick(float deltaTime) {}
        virtual void Shutdown() {}

    protected:
        EngineContext* m_EngineContext;
    };
}