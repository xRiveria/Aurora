#pragma once
#include "IComponent.h"
#include "../Scripting/ScriptField.h"

namespace Aurora
{
    class Script : public IComponent
    {
    public:
        Script(EngineContext* engineContext, Entity* entity, uint32_t componentID = 0);
        ~Script() = default;

        void SetModule(const std::string& moduleName);
        std::string& GetModuleName() { return m_ModuleName; }

        ScriptFieldMapping& GetFieldMap() { return m_ModuleFieldMapping; }
        
    private:
        std::string m_ModuleName;
        ScriptFieldMapping m_ModuleFieldMapping;
    };
}