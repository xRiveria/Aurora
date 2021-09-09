#include "Aurora.h"
#include "Scripting.h"
#include "ScriptBindings.h"
#include "ScriptingUtilities.h"
#include <mono/metadata/object.h>
#include <mono/metadata/threads.h>
#include "ScriptInstance.h"


namespace Aurora
{ 
    static MonoDomain*   s_CurrentMonoDomain = nullptr;
    static MonoDomain*   s_NewMonoDomain = nullptr;
    static EngineContext* s_EngineContext = nullptr;

    static MonoAssembly* s_ApplicationAssembly = nullptr;
    MonoImage* s_ApplicationAssemblyImage = nullptr;

    // Core Assembly
    static std::string s_AuroraRuntimeAssemblyPath;
    static MonoAssembly* s_AuroraCoreAssembly = nullptr;
    MonoImage* s_AuroraCoreAssemblyImage = nullptr;

    // Path
    static bool s_PostLoadCleanup = false;

    // Data
    static std::vector<ScriptInstance> m_ScriptInstanceLibrary;
    static std::string m_DefaultScriptNamespace = "AuroraEngine";

    Scripting::Scripting(EngineContext* engineContext) : ISubsystem(engineContext)
    {
        
    }

    Scripting::~Scripting()
    {
        if (m_ScriptDomain)
        {
            MonoObject* exception = nullptr;
            mono_domain_try_unload(m_ScriptDomain, &exception);
            m_ScriptDomain = nullptr;
        }


        // if (m_MonoDomain)
        // {
        //    mono_jit_cleanup(m_MonoDomain);
        //    m_MonoDomain = nullptr;
        // }    
    }

    bool Scripting::Initialize()
    {
        // Tell Mono where its files are.
        std::string resourceDirectory = m_EngineContext->GetSubsystem<Settings>()->GetResourceDirectory(ResourceDirectory::Scripts);
        mono_set_dirs(resourceDirectory.c_str(), ".");

        // Create the main mono application domain, which initializes the default framework version.
        MonoDomain* monoDomain = mono_jit_init_version("Aurora Engine", "v4.0.30319");
        if (!monoDomain)
        {
            AURORA_ERROR(LogLayer::Scripting, "Failed to initialize Mono Domain.");
            return false;
        }

        m_EngineContext->GetSubsystem<Settings>()->RegisterExternalLibrary("Mono", "6.12.0.122", "https://www.mono-project.com");

        // Initialize Runtime Assembly
        LoadAuroraRuntimeAssembly(m_EngineContext->GetSubsystem<Settings>()->GetResourceDirectory(ResourceDirectory::Scripts) + "\\AuroraEngine.dll");

        // Manual loading for now.
        s_EngineContext = m_EngineContext;
        LoadApplicationAssembly(m_EngineContext->GetSubsystem<Settings>()->GetResourceDirectory(ResourceDirectory::Scripts) + "\\Initializer.dll");
        InitializeScriptInstance(m_EngineContext->GetSubsystem<Settings>()->GetResourceDirectory(ResourceDirectory::Scripts) + "\\Initializer.cs");

        return true;     
    }

    void Scripting::Tick(float deltaTime)
    {
        /*
        if (!m_IsReloading)
        {
            for (int i = 0; i < m_ScriptLibrary.size(); i++)
            {
                InvokeScriptUpdateMethod(&m_ScriptLibrary[i], deltaTime);
            }
        }
        else
        {
            return;
        }
        */
    }

    bool Scripting::LoadAuroraRuntimeAssembly(const std::string& assemblyPath)
    {
        s_AuroraRuntimeAssemblyPath = assemblyPath;
        if (s_CurrentMonoDomain) // If our current domain exists, create a new domain to reload assemblies.
        {
            s_NewMonoDomain = mono_domain_create_appdomain((char*)"Aurora Runtime", nullptr);
            mono_domain_set(s_NewMonoDomain, false);
            s_PostLoadCleanup = true; // Ensure that we do post loading cleanup for the newly created domain.
        }
        else // Else if our current domain doesn't exist, create it.
        {
            s_CurrentMonoDomain = mono_domain_create_appdomain((char*)"Aurora Runtime", nullptr);
            mono_domain_set(s_CurrentMonoDomain, false);
            s_PostLoadCleanup = false;
        }

        s_AuroraCoreAssembly = LoadMonoAssembly(s_AuroraRuntimeAssemblyPath);
        if (!s_AuroraCoreAssembly)
        {
            AURORA_ERROR(LogLayer::Scripting, "Failed to load runtime assembly.");
            return false;
        }

        s_AuroraCoreAssemblyImage = GetMonoAssemblyImage(s_AuroraCoreAssembly);

        /// Load Exception Method
        /// Load Entity Class

        return true;
    }

    bool Scripting::LoadApplicationAssembly(const std::string& assemblyPath)
    {
        if (s_ApplicationAssembly)
        {
            s_ApplicationAssembly = nullptr;
            s_ApplicationAssemblyImage = nullptr;
            return ReloadAssembly(assemblyPath);
        }

        MonoAssembly* applicationAssembly = LoadMonoAssembly(assemblyPath);
        if (!applicationAssembly)
        {
            AURORA_ERROR(LogLayer::Scripting, "Failed to load application assembly.");
            return false;
        }

        MonoImage* applicationAssemblyImage = GetMonoAssemblyImage(applicationAssembly);
        ScriptBindings::RegisterMonoCallbacks(); // Register our C++ functions with CS callbacks.
        AURORA_INFO(LogLayer::Scripting, "Successfully initialized Mono API and registered Callbacks.");

        if (s_PostLoadCleanup)
        {
            mono_domain_unload(s_CurrentMonoDomain);
            s_CurrentMonoDomain = s_NewMonoDomain;
            s_NewMonoDomain = nullptr;
        }

        s_ApplicationAssembly = applicationAssembly;
        s_ApplicationAssemblyImage = applicationAssemblyImage;

        return true;
    }

    bool Scripting::ReloadAssembly(const std::string& filePath)
    {
        if (!LoadAuroraRuntimeAssembly(s_AuroraRuntimeAssemblyPath))
        {
            return false;
        }

        if (!LoadApplicationAssembly(filePath))
        {
            return false;
        }

        InitializeScriptInstance(s_EngineContext->GetSubsystem<Settings>()->GetResourceDirectory(ResourceDirectory::Scripts) + "\\Initializer.cs");
        
        return true;
    }

    static bool isLoaded = false;

    bool Scripting::InitializeScriptInstance(const std::string& filePath)
    {
        // if (!m_IsAssemblyAPICompiled)
        // {
        //    m_IsAssemblyAPICompiled = CompileAssemblyAPI(m_MonoDomain);
        //    if (!m_IsAssemblyAPICompiled)
        //    {
        //        AURORA_ERROR(LogLayer::Scripting, "Failed to load Assembly API.");
        //        return false;
        //    }
        // }
        ScriptingUtilities::CompileScript(s_EngineContext->GetSubsystem<Settings>()->GetResourceDirectory(ResourceDirectory::Scripts) + "\\Initializer.cs", s_EngineContext->GetSubsystem<Settings>()->GetResourceDirectory(ResourceDirectory::Scripts) + "\\AuroraEngine.dll", s_EngineContext->GetSubsystem<Settings>());

        ScriptInstanceData scriptInstanceData;
        /// Some scripts might not have namespaces. Lets take note of that.
        scriptInstanceData.m_NamespaceName = m_DefaultScriptNamespace;
        scriptInstanceData.m_ClassName = FileSystem::GetFileNameWithoutExtensionFromFilePath(filePath);
        scriptInstanceData.m_FullReferralName = scriptInstanceData.m_NamespaceName + "." + scriptInstanceData.m_ClassName;

        scriptInstanceData.m_Class = GetMonoClass(s_ApplicationAssemblyImage, scriptInstanceData);
        scriptInstanceData.InitializeClassMethods(s_ApplicationAssemblyImage);

        ScriptInstance scriptInstance;
        scriptInstance.m_ScriptFilePath = filePath;
        scriptInstance.m_ScriptInstanceData = &scriptInstanceData;

        /// Saving of old object fields.

        // Creation of new instance.
        scriptInstance.m_MonoGCHandle = InstantiateMonoObjectInstance(scriptInstanceData);

        /// Retrieval of old object fields.
        mono_runtime_invoke(scriptInstanceData.m_OnStartMethod, scriptInstance.GetInstance(), nullptr, nullptr);

        DestroyScriptInstance(scriptInstance.m_MonoGCHandle);
        
        ///

        return true;
    }

    void Scripting::DestroyScriptInstance(uint32_t handle)
    {
        mono_gchandle_free(handle);
    }

    static std::vector<char> ReadFile(const std::string& filePath)
    {
        std::ifstream myfile(filePath.c_str(), std::ios::binary | std::ios::ate);
        std::ifstream::pos_type pos = myfile.tellg();

        std::vector<char> result(pos);
        myfile.seekg(0, std::ios::beg);
        myfile.read(&result[0], pos);

        return result;
    }

    MonoAssembly* Scripting::LoadAssemblyFromFile(const std::string& assemblyPath)
    {
        AURORA_INFO(LogLayer::Scripting, "Loading Assembly for: %s", assemblyPath.c_str());

        // Read the DLL file into memory.
        std::vector<char> data = ReadFile(assemblyPath);

        // Open as image.
        MonoImageOpenStatus status;
        MonoImage* image = mono_image_open_from_data_full((char*)&data[0], data.size(), true, &status, false);
        if (status != MONO_IMAGE_OK || image == 0)
        {
            AURORA_ERROR(LogLayer::Scripting, "Failed to load Mono Image from DLL file...");
            return nullptr;
        }

        MonoAssembly* monoAssembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, false);
        if (status != MONO_IMAGE_OK || monoAssembly == 0)
        {
            AURORA_ERROR(LogLayer::Scripting, "Failed to load Mono Assembly from DLL file...");
            return nullptr;
        }

        mono_image_close(image);
        return monoAssembly;
    }

    MonoAssembly* Scripting::LoadMonoAssembly(const std::string& assemblyPath)
    {
        MonoAssembly* monoAssembly = LoadAssemblyFromFile(assemblyPath.c_str());
        if (!monoAssembly)
        {
            AURORA_ERROR(LogLayer::Scripting, "Could not load Mono Assembly.");
            return nullptr;
        }
        else
        {
            AURORA_INFO(LogLayer::Scripting, "Successfully loaded Mono Assembly.");
            return monoAssembly;
        }
    }

    MonoImage* Scripting::GetMonoAssemblyImage(MonoAssembly* assembly)
    {
        MonoImage* monoImage = mono_assembly_get_image(assembly);
        if (!monoImage)
        {
            AURORA_ERROR(LogLayer::Scripting, "Mono_Assembly_Get_Image failed.");
            return nullptr;
        }

        return monoImage;
    }

    MonoMethod* Scripting::GetMonoMethod(MonoImage* image, const std::string& methodDescription)
    {
        MonoMethodDesc* monoDescription = mono_method_desc_new(methodDescription.c_str(), NULL);
        if (!monoDescription)
        {
            AURORA_ERROR(LogLayer::Scripting, "Mono_Method_Desc_New failed.");
            return nullptr;
        }

        MonoMethod* monoMethod = mono_method_desc_search_in_image(monoDescription, image);
        if (!monoMethod)
        {
            AURORA_ERROR(LogLayer::Scripting, "Mono_Method_Desc_Search_In_Image failed.");
            return nullptr;
        }

        return monoMethod;
    }

    MonoClass* Scripting::GetMonoClass(MonoImage* image, const ScriptInstanceData& scriptInstanceData)
    {
        MonoClass* monoClass = mono_class_from_name(image, scriptInstanceData.m_NamespaceName.c_str(), scriptInstanceData.m_ClassName.c_str());
        if (!monoClass)
        {
            AURORA_ERROR(LogLayer::Scripting, "Mono_Class_From_Name failed.");
            return nullptr;
        }

        return monoClass;
    }

    uint32_t Scripting::InstantiateMonoObjectInstance(ScriptInstanceData& scriptInstanceData)
    {
        MonoObject* monoObjectInstance = mono_object_new(s_CurrentMonoDomain, scriptInstanceData.m_Class);
        if (!monoObjectInstance)
        {
            AURORA_ERROR(LogLayer::Scripting, "Mono_Object_New failed.");
            return 0;
        }

        mono_runtime_object_init(monoObjectInstance);
        uint32_t monoGCHandle = mono_gchandle_new(monoObjectInstance, false);

        return monoGCHandle;
    }














    /*
    * 
    *     bool Scripting::CompileAssemblyAPI(MonoDomain* monoDomain)
    {
        // Retrieve callbacks assembly.
        ScriptingUtilities::g_SettingsSubsystem = m_EngineContext->GetSubsystem<Settings>();
        const std::string callbacksScript = m_EngineContext->GetSubsystem<Settings>()->GetResourceDirectory(ResourceDirectory::Scripts) + "\\AuroraEngine.cs";
        auto pair = ScriptingUtilities::CompileAndLoadAssembly(monoDomain, callbacksScript, false);

        if (!pair.first)
        {
            AURORA_ERROR(LogLayer::Scripting, "Failed to load Callbacks Assembly.");
            return false;
        }

        // Retrieve image from script assembly.
        if (!pair.second)
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
        if (!m_IsReloading)
        {
            if (!scriptInstance->m_MonoMethodStart || !scriptInstance->m_MonoObject)
            {
                AURORA_ERROR(LogLayer::Serialization, "Failed to invoke Start() method for Script.");
                return false;
            }

            mono_runtime_invoke(scriptInstance->m_MonoMethodStart, scriptInstance->m_MonoObject, nullptr, nullptr);
            return true;
        }

        return false;
    }

    bool Scripting::InvokeScriptUpdateMethod(const ScriptInstance* scriptInstance, float deltaTime)
    {
        if (!m_IsReloading)
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

        return false;
    }
    */
}