#include "Aurora.h"
#include "Script.h"

namespace Aurora
{
    Script::Script(EngineContext* engineContext, Entity* entity, uint32_t componentID) : IComponent(engineContext, entity, componentID)
    {

    }

    void Script::SetModule(const std::string& moduleName)
    {
        m_ModuleName = moduleName;
    }
}