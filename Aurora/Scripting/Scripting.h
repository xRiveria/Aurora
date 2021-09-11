#pragma once
#include "EngineContext.h"
#include "ISubsystem.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include "../Scene/Components/Script.h"
#include "ScriptInstance.h"

namespace Aurora
{
    struct ScriptClassData;
    struct ScriptInstance;

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

        static void ShutdownScriptInstance(Script* scriptComponent);
        static bool InitializeScriptInstance(Script* scriptComponent);
        static bool ModuleExists(const std::string& moduleName);
        static std::string StripNamespace(const std::string& namespaceName, const std::string& moduleName);

    private:
        bool CompileAssemblyAPI(MonoDomain* monoDomain);

    public:
        // ===
        static void DestroyScriptInstance(uint32_t handle);

        // Loading
        static MonoAssembly* LoadAssemblyFromFile(const std::string& assemblyPath);
        static MonoAssembly* LoadMonoAssembly(const std::string& assemblyPath);
        static MonoImage* GetMonoAssemblyImage(MonoAssembly* assembly);
        static MonoMethod* GetMonoMethod(MonoImage* image, const std::string& methodDescription);
        static MonoClass* GetMonoClass(MonoImage* image, const ScriptClassData& scriptInstanceData);
        static uint32_t InstantiateMonoObjectInstance(ScriptClassData& scriptInstanceData);

        // Debug
        static void PrintMonoClassMethods(MonoClass* monoClass);
        static void PrintMonoClassFields(MonoClass* monoClass);


        bool m_IsReloading = false;

        MonoDomain* m_ScriptDomain = nullptr;
        MonoDomain* m_MonoDomain = nullptr;
        bool m_IsAssemblyAPICompiled = false;

        static ScriptMap s_ScriptInstanceMap;

    private:
        friend PublicField;
    };
};