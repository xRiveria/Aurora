#pragma once
#include <unordered_map>
#include <memory>

namespace Aurora
{
    class EngineContext;

    class Engine
    {
    public:
        Engine();
        ~Engine();

        void Tick() const; // Supplies deltaTime from the Time subsystem.

        EngineContext* RetrieveEngineContext() const { return m_EngineContext.get(); }

    private:
        std::shared_ptr<EngineContext> m_EngineContext;
    };
}
