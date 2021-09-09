#pragma once
#include "../Log/Log.h"
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>
#include "Scripting.h"
#include "ScriptingUtilities.h"

namespace Aurora
{
    struct ScriptInstanceData
    {
        std::string m_FullReferralName;             // ScriptNamespace::ClassName
        std::string m_ClassName;                    // ClassName
        std::string m_NamespaceName;                // ScriptNamespace

        // Default Methods - These are guaranteed to exist with every script.
        MonoClass* m_Class = nullptr;               // Retrieves the class.
        MonoMethod* m_OnStartMethod = nullptr;
        MonoMethod* m_OnUpdateMethod = nullptr;

        void InitializeClassMethods(MonoImage* monoImage)
        {
            m_OnStartMethod = Scripting::GetMonoMethod(monoImage, m_FullReferralName + ":OnStart()");
            m_OnUpdateMethod = Scripting::GetMonoMethod(monoImage, m_FullReferralName + ":OnUpdate(single)");
        }
    };

    struct ScriptInstance
    {
        std::string m_ScriptFilePath;
        uint32_t m_MonoGCHandle = 0; // Mono Object Handle.
        MonoObject* GetInstance()
        {
            AURORA_ASSERT(m_MonoGCHandle, "Script instance has not been instantiated.");
            return mono_gchandle_get_target(m_MonoGCHandle); // GC handles are used to keep references to managed objects in unmanaged space and prevents them from being disposed.
        }

        ScriptInstanceData* m_ScriptInstanceData;
    };
}


        /*

        template<typename T>
        T ReadFieldValue(const std::string& valueName)
        {
            T value;
            if (MonoClassField* classField = mono_class_get_field_from_name(m_MonoClass, valueName.c_str()))
            {
                mono_field_get_value(m_MonoObject, classField, &value);
                return value;
            }
        }

        template<typename T>
        void SetFieldValue(T& value, const std::string& valueFieldName)
        {
            if (MonoClassField* classField = mono_class_get_field_from_name(m_MonoClass, valueFieldName.c_str()))
            {
                mono_field_set_value(m_MonoObject, classField, &value);
            }
        }

        std::string ReadFieldString(const std::string& valueFieldName)
        {
            MonoString* stringValue;

            if (MonoClassField* classField = mono_class_get_field_from_name(m_MonoClass, valueFieldName.c_str()))
            {
                // Always pass a pointer.
                mono_field_get_value(m_MonoObject, classField, &stringValue); // Retrieve MonoString from Mono.

                // Convert the string to UTF-8 encoding for printing.
                return std::string(mono_string_to_utf8(stringValue));
            }
        }

        void SetFieldString(const std::string& value, const std::string& valueFieldName)
        {
            MonoString* stringValue;
            MonoDomain* monoDomain = mono_object_get_domain(m_MonoObject);

            if (MonoClassField* classField = mono_class_get_field_from_name(m_MonoClass, valueFieldName.c_str()))
            {
                stringValue = mono_string_new(monoDomain, value.c_str());

                // For reference types, we pass the pointer directly, instead of a pointer to the value.
                mono_field_set_value(m_MonoObject, classField, stringValue);
            }
        }
    };

    
    */
