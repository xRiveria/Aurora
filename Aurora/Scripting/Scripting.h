#pragma once
#include "EngineContext.h"
#include "ISubsystem.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>

namespace Aurora
{
    struct ScriptInstanceData;

    class Scripting : public ISubsystem
    {
    public:
        Scripting(EngineContext* engineContext);
        ~Scripting();

        bool Initialize() override;
        void Tick(float deltaTime) override;

        static bool LoadAuroraRuntimeAssembly(const std::string& assemblyPath);
        static bool LoadApplicationAssembly(const std::string& path);
        static bool ReloadAssembly(const std::string& filePath);

        static bool InitializeScriptInstance(const std::string& filePath);
        // bool InvokeScriptStartMethod(const ScriptInstance* scriptInstance);
        // bool InvokeScriptUpdateMethod(const ScriptInstance* scriptInstance, float deltaTime);

        // std::unordered_map<uint32_t, ScriptInstance> GetScriptLibrary() { return m_ScriptLibrary; }

    private:
        bool CompileAssemblyAPI(MonoDomain* monoDomain);

    public:
        // ===
        static void DestroyScriptInstance(uint32_t handle);

        static MonoAssembly* LoadAssemblyFromFile(const std::string& assemblyPath);
        static MonoAssembly* LoadMonoAssembly(const std::string& assemblyPath);
        static MonoImage* GetMonoAssemblyImage(MonoAssembly* assembly);
        static MonoMethod* GetMonoMethod(MonoImage* image, const std::string& methodDescription);
        static MonoClass* GetMonoClass(MonoImage* image, const ScriptInstanceData& scriptInstanceData);
        static uint32_t InstantiateMonoObjectInstance(ScriptInstanceData& scriptInstanceData);


        bool m_IsReloading = false;

        MonoDomain* m_ScriptDomain = nullptr;
        MonoDomain* m_MonoDomain = nullptr;
        bool m_IsAssemblyAPICompiled = false;
    }; 
}