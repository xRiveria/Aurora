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
    };
}