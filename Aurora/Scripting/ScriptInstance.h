#pragma once
#include "../Log/Log.h"
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>

namespace Aurora
{
    struct ScriptInstance
    {
        MonoAssembly* m_Assembly = nullptr;   // Assemblies are essentially compiled code that can be executed by the CLR. It is a collection of types and resources that are built to work together and form a logical unit of functionality.
        MonoImage* m_MonoImage   = nullptr;   // Images contain all functions and classes within the Assembly.
        MonoClass* m_MonoClass   = nullptr;   // Retrieves the class.
        MonoObject* m_MonoObject = nullptr;   // Pointer to the created class instance.
        
        // All scripts contain a Start() and Update() method.
        MonoMethod* m_MonoMethodStart  = nullptr;
        MonoMethod* m_MonoMethodUpdate = nullptr;

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
}