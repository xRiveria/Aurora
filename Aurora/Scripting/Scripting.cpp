#include "Aurora.h"
#include "Scripting.h"
#include "ScriptBindings.h"
#include "ScriptingUtilities.h"
#include <mono/metadata/object.h>
#include <mono/metadata/threads.h>

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

        mono_domain_set(m_MonoDomain, false);
        mono_thread_set_main(mono_thread_current());

        m_EngineContext->GetSubsystem<Settings>()->RegisterExternalLibrary("Mono", "6.12.0.122", "https://www.mono-project.com");

        // Currently our class name.
        LoadScript(m_EngineContext->GetSubsystem<Settings>()->GetResourceDirectory(ResourceDirectory::Scripts) + "\\Initializer.cs");

        return true;     
    }

    bool Scripting::HotReload()
    {
        m_IsReloading = true;

        MonoDomain* newDomain = mono_domain_create_appdomain((char*)"RandomDomain", NULL);
        mono_domain_set(newDomain, false);

        // Reload DLLs
        for (int i = 0; i < m_ScriptLibrary.size(); i++)
        {
            m_ScriptLibrary[i].Reload(newDomain, m_EngineContext);
            InvokeScriptStartMethod(&m_ScriptLibrary[i]);
        }

        //// unload 
        MonoDomain* domainToUnload = mono_domain_get();
        if (domainToUnload && domainToUnload != mono_get_root_domain())
        {
            mono_domain_set(mono_get_root_domain(), false);
            //mono_thread_pop_appdomain_ref();
            mono_domain_unload(domainToUnload);
        }
      
        m_IsReloading = false;
        return true;
    }

    void Scripting::Tick(float deltaTime)
    {
        for (int i = 0; i < m_ScriptLibrary.size(); i++)
        {
            InvokeScriptUpdateMethod(&m_ScriptLibrary[i], deltaTime);       
        }
    }

    bool Scripting::LoadScript(const std::string& filePath)
    {
        if (!m_IsAssemblyAPICompiled)
        {
            m_IsAssemblyAPICompiled = CompileAssemblyAPI(m_MonoDomain);
            if (!m_IsAssemblyAPICompiled)
            {
                AURORA_ERROR(LogLayer::Scripting, "Failed to load Assembly API.");
                return false;
            }
        }

        ScriptInstance scriptInstance;
        const std::string className = FileSystem::GetFileNameWithoutExtensionFromFilePath(filePath);
        scriptInstance.m_FilePath = filePath;

        // Get assembly.
        scriptInstance.m_Assembly = ScriptingUtilities::CompileAndLoadAssembly(m_MonoDomain, filePath, true);
      
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
        m_ScriptLibrary[m_ScriptIndex++] = scriptInstance;

        // Return script ID.
        return true;
        
    }

    bool Scripting::CompileAssemblyAPI(MonoDomain* monoDomain)
    {
        // Retrieve callbacks assembly.
        ScriptingUtilities::g_SettingsSubsystem = m_EngineContext->GetSubsystem<Settings>();
        const std::string callbacksScript = m_EngineContext->GetSubsystem<Settings>()->GetResourceDirectory(ResourceDirectory::Scripts) + "\\AuroraEngine.cs";
        MonoAssembly* callbacksAssembly = ScriptingUtilities::CompileAndLoadAssembly(monoDomain, callbacksScript, false);

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