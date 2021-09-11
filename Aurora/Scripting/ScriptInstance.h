#pragma once
#include "../Log/Log.h"
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>
#include "ScriptingUtilities.h"
#include "ScriptField.h"

namespace Aurora
{
    struct ScriptClassData
    {
        std::string m_FullReferralName;             // ScriptNamespace::ClassName
        std::string m_ClassName;                    // ClassName
        std::string m_NamespaceName;                // ScriptNamespace

        // Default Methods - These are guaranteed to exist with every script.
        MonoClass* m_Class = nullptr;               // Retrieves the class.
        MonoMethod* m_OnStartMethod = nullptr;
        MonoMethod* m_OnUpdateMethod = nullptr;

        void InitializeClassMethods(MonoImage* monoImage);
    };

    struct ScriptInstanceData
    {
        std::string m_ScriptFilePath;
        uint32_t m_MonoGCHandle = 0; // Mono Object Handle.
        MonoObject* GetInstance()
        {
            AURORA_ASSERT(m_MonoGCHandle, "Script instance has not been instantiated.");
            return mono_gchandle_get_target(m_MonoGCHandle); // GC handles are used to keep references to managed objects in unmanaged space and prevents them from being disposed.
        }

        ScriptClassData* m_ScriptClassData;
    };

    struct ScriptInstance
    {
        ScriptInstanceData m_ScriptClassData;
        ScriptFieldMapping m_ScriptFieldMapping;
    };

    using ScriptMap = std::unordered_map<uint32_t, std::unordered_map<uint32_t, ScriptInstance>>;
}


