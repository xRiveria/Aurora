#include "Aurora.h"
#include "Script.h"

namespace Aurora
{
    Script::Script(EngineContext* engineContext, Entity* entity, uint32_t componentID) : IComponent(engineContext, entity, componentID)
    {
        m_Type = ComponentType::Script;
    }

    void Script::SetModule(const std::string& moduleName)
    {
        m_ModuleName = moduleName;
    }
}