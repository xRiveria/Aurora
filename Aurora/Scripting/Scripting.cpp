#include "Aurora.h"
#include "Scripting.h"
#include "ScriptBindings.h"
#include "ScriptingUtilities.h"

namespace Aurora
{
    Scripting::Scripting(EngineContext* engineContext) : ISubsystem(engineContext)
    {
        
    }

    Scripting::~Scripting()
    {
        // Release the domain.
        if (m_MonoDomain)
        {
            mono_jit_cleanup(m_MonoDomain);
        }
    }

    bool Scripting::Initialize()
    {
        // Tell Mono where its files are.
        std::string resourceDirectory = m_EngineContext->GetSubsystem<Settings>()->GetResourceDirectory(ResourceDirectory::Scripts);
        mono_set_dirs(resourceDirectory.c_str(), ".");

        // Create the main mono application domain, which initializes the default framework version.
        m_MonoDomain = mono_jit_init_version("Aurora Engine", "v4.0.30319");
        if (!m_MonoDomain)
        {
            AURORA_ERROR(LogLayer::Scripting, "Failed to initialize Mono Domain.");
            return false;
        }

        m_EngineContext->GetSubsystem<Settings>()->RegisterExternalLibrary("Mono", "6.12.0.122", "https://www.mono-project.com");

        // Currently our class name.
        LoadScript("Initializer");
            /*
                /// Per Script
                ScriptInstance scriptInstance;

                // Load Assembly
                // scriptInstance.m_Assembly = mono_domain_assembly_open(m_MonoDomain, (m_EngineContext->GetSubsystem<Settings>()->GetResourceDirectory(ResourceDirectory::Scripts) + "\\SolusProject.dll").c_str());
                // if (!scriptInstance.m_Assembly)
                // {
                //     AURORA_ERROR(LogLayer::Scripting, "Failed to initialize Mono Assembly.");
                //   return false;
                // }

                //// A mono image is an in memory object that represents all functions and classes that are in the assembly. We will retrieve it from our assembly here.
                //scriptInstance.m_MonoImage = mono_assembly_get_image(scriptInstance.m_Assembly);
                //if (!scriptInstance.m_MonoImage)
                //{
                //    AURORA_ERROR(LogLayer::Scripting, "Failed to initialize Mono Image.");
                //    return false;
                //}

                //if (scriptInstance.m_MonoImage)
                //{
                //    mono_add_internal_call("Aurora.Debug::Log(single,Aurora.DebugType)", &Aurora::ScriptBindings::LogFloat);
                //    mono_add_internal_call("Aurora.Debug::Log(string,Aurora.DebugType)", &Aurora::ScriptBindings::LogString);
                //}

                // Check if the class containing the method exists.
                // MonoClass* controllerClass = mono_class_from_name(m_MonoImage, "SolusProject", "PlayerController");
                MonoClass* mainClass = mono_class_from_name(scriptInstance.m_MonoImage, "SolusProject", "Initializer");

                if (mainClass)
                {
                    MonoMethodDesc* mainMethodDescription = mono_method_desc_new(".Initializer:Main()", false);
                    if (mainMethodDescription)
                    {
                        // Find the mthod.
                        MonoMethod* mainMethodPointer = mono_method_desc_search_in_class(mainMethodDescription, mainClass);
                        if (mainMethodPointer)
                        {
                            // Call method.
                            scriptInstance.m_MonoObject = mono_runtime_invoke(mainMethodPointer, nullptr, nullptr, nullptr);
                        }

                        // Free description.
                        mono_method_desc_free(mainMethodDescription);
                    }
                }

                m_ScriptLibrary[++m_ScriptIndex] = scriptInstance;
                */

     return true;
       
    }

    bool Scripting::LoadScript(const std::string& filePath)
    {
        if (!m_IsAssemblyAPICompiled)
        {
            m_IsAssemblyAPICompiled = CompileAssemblyAPI();
            if (!m_IsAssemblyAPICompiled)
            {
                AURORA_ERROR(LogLayer::Scripting, "Failed to load Assembly API.");
                return false;
            }
        }

        ScriptInstance scriptInstance;
        // const std::string className = FileSystem::GetFileNameWithoutExtensionFromFilePath(filePath);
        const std::string className = filePath;

        // Get assembly.
        scriptInstance.m_Assembly = ScriptingUtilities::CompileAndLoadAssembly(m_MonoDomain, m_EngineContext->GetSubsystem<Settings>()->GetResourceDirectory(ResourceDirectory::Scripts) + "\\AuroraEngine.cs", false);
        if (!scriptInstance.m_Assembly)
        {
            AURORA_ERROR(LogLayer::Scripting, "Failed to load Assembly.");
            return false;
        }

        // Get image from script assembly.
        scriptInstance.m_MonoImage = mono_assembly_get_image(scriptInstance.m_Assembly);
        if (!scriptInstance.m_MonoImage)
        {
            AURORA_ERROR(LogLayer::Scripting, "Failed to retrieve Image from Assembly.");
            return false;
        }

        // Get class.
        scriptInstance.m_MonoClass = mono_class_from_name(scriptInstance.m_MonoImage, "", "Initializer");
        if (!scriptInstance.m_MonoClass)
        {
            mono_image_close(scriptInstance.m_MonoImage);
            AURORA_ERROR(LogLayer::Scripting, "Failed to retrieve class.");
            return false;
        }

        // Create class instance.
        scriptInstance.m_MonoObject = mono_object_new(m_MonoDomain, scriptInstance.m_MonoClass);
        if (!scriptInstance.m_MonoObject)
        {
            mono_image_close(scriptInstance.m_MonoImage);
            AURORA_ERROR(LogLayer::Scripting, "Failed to create class instance.");
            return false;
        }

        // Get methods.
        scriptInstance.m_MonoMethodStart = ScriptingUtilities::GetMethod(scriptInstance.m_MonoImage, className + ":Start()");
        scriptInstance.m_MonoMethodUpdate = ScriptingUtilities::GetMethod(scriptInstance.m_MonoImage, className + ":Update(single)");

        InvokeScriptStartMethod(&scriptInstance);
        /// Entity
        /// Transform
        /// Call default constructor.
         
        // Add script.
        m_ScriptLibrary[++m_ScriptIndex] = scriptInstance;

        // Return script ID.
        return true;
    }

    bool Scripting::CompileAssemblyAPI()
    {
        // Retrieve callbacks assembly.
        ScriptingUtilities::g_SettingsSubsystem = m_EngineContext->GetSubsystem<Settings>();
        const std::string callbacksScript = m_EngineContext->GetSubsystem<Settings>()->GetResourceDirectory(ResourceDirectory::Scripts) + "\\AuroraEngine.cs";
        MonoAssembly* callbacksAssembly = ScriptingUtilities::CompileAndLoadAssembly(m_MonoDomain, callbacksScript, false);

        if (!callbacksAssembly)
        {
            AURORA_ERROR(LogLayer::Scripting, "Failed to load Callbacks Assembly.");
            return false;
        }

        // Retrieve image from script assembly.
        MonoImage* callbacksImage = mono_assembly_get_image(callbacksAssembly);
        if (!callbacksImage)
        {
            AURORA_ERROR(LogLayer::Scripting, "Failed to retrieve Image from Callbacks Assembly.");
            return false;
        }

        // Register static callbacks.
        ScriptBindings::RegisterMonoCallbacks(m_EngineContext); // Register our C++ functions with CS callbacks.
        AURORA_INFO(LogLayer::Scripting, "Successfully initialized Mono API and registered Callbacks.");
        return true;
    }

    bool Scripting::InvokeScriptStartMethod(const ScriptInstance* scriptInstance)
    {
        if (!scriptInstance->m_MonoMethodStart || !scriptInstance->m_MonoObject)
        {
            AURORA_ERROR(LogLayer::Serialization, "Failed to invoke Start() method for Script.");
            return false;
        }

        mono_runtime_invoke(scriptInstance->m_MonoMethodStart, scriptInstance->m_MonoObject, nullptr, nullptr);
        return true;
    }

    bool Scripting::InvokeScriptUpdateMethod(const ScriptInstance* scriptInstance, float deltaTime)
    {
        if (!scriptInstance->m_MonoMethodUpdate || !scriptInstance->m_MonoObject)
        {
            AURORA_ERROR(LogLayer::Serialization, "Failed to invoke Update() method for Script.");
            return false;
        }

        // Set method argument. https://www.mono-project.com/docs/advanced/embedding/
        void* arguments[1];
        arguments[0] = &deltaTime;

        // Execute. If the method is static, we use NULL as the second argument, otherwise, we use a pointer to the object instance, and pass our argument in the 3rd parameter.
        mono_runtime_invoke(scriptInstance->m_MonoMethodUpdate, scriptInstance->m_MonoObject, arguments, nullptr);
        return true;
    }
}