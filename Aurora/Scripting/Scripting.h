#pragma once
#include "EngineContext.h"
#include "ISubsystem.h"
#include "ScriptInstance.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>

namespace Aurora
{
    class Scripting : public ISubsystem
    {
    public:
        Scripting(EngineContext* engineContext);
        ~Scripting();

        bool Initialize() override;
        void Tick(float deltaTime) override;

        bool LoadScript(const std::string& filePath);
        bool InvokeScriptStartMethod(const ScriptInstance* scriptInstance);
        bool InvokeScriptUpdateMethod(const ScriptInstance* scriptInstance, float deltaTime);

        // std::unordered_map<uint32_t, ScriptInstance> GetScriptLibrary() { return m_ScriptLibrary; }

        bool HotReload();

    private:
        bool CompileAssemblyAPI(MonoDomain* monoDomain);

    public:
        bool m_IsReloading = false;
        MonoDomain* m_MonoDomain = nullptr;
        bool m_IsAssemblyAPICompiled = false;

        std::unordered_map<uint32_t, ScriptInstance> m_ScriptLibrary;
        uint32_t m_ScriptIndex = 0;
    }; 
}