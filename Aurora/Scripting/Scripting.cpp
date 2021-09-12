#include "Aurora.h"
#include "Scripting.h"
#include "ScriptBindings.h"
#include "ScriptingUtilities.h"
#include <mono/metadata/object.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/attrdefs.h>
#include "../Scene/World.h"

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
    static std::vector<ScriptInstanceData> m_ScriptInstanceLibrary;
    static std::unordered_map<std::string, ScriptClassData> s_ScriptClassMap;
    std::unordered_map<std::string, std::string> Scripting::s_PublicFieldStringValues;
    ScriptMap Scripting::s_ScriptInstanceMap;

    static std::string m_DefaultScriptNamespace = "Aurora";

    FieldType GetAuroraFieldType(MonoType* monoType)
    {
        int typeInternal = mono_type_get_type(monoType);

        switch (typeInternal)
        {
        case MONO_TYPE_R4: return FieldType::Float;
        case MONO_TYPE_I4: return FieldType::Integer;
        case MONO_TYPE_U4: return FieldType::UnsignedInteger;
        case MONO_TYPE_STRING: return FieldType::String;
        case MONO_TYPE_CLASS:
        {
            /// Class Implementations
            break;
        }

        case MONO_TYPE_VALUETYPE:
        {
            char* typeName = mono_type_get_name(monoType);
            if (strcmp(typeName, "Aurora.Vector2") == 0) { return FieldType::Vector2; }
            if (strcmp(typeName, "Aurora.Vector3") == 0) { return FieldType::Vector3; }
            if (strcmp(typeName, "Aurora.Vector4") == 0) { return FieldType::Vector4; }
        }
        }

        return FieldType::None;
    }

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

        s_EngineContext = m_EngineContext;

        // Initialize Runtime Assembly
        LoadAuroraRuntimeAssembly(m_EngineContext->GetSubsystem<Settings>()->GetResourceDirectory(ResourceDirectory::Scripts) + "\\AuroraScript_Core.dll");
        // Initialize Application Assembly
        LoadApplicationAssembly(m_EngineContext->GetSubsystem<Aurora::Settings>()->GetResourceDirectory(Aurora::ResourceDirectory::Scripts) + "\\Sandbox.dll");

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
            // return ReloadAssembly(assemblyPath);
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

        if (s_ScriptInstanceMap.size())
        {
            const uint32_t sceneID = 0;
            World* worldSubsystem = s_EngineContext->GetSubsystem<World>();
            if (const auto& scriptInstanceMap = s_ScriptInstanceMap.find(sceneID); scriptInstanceMap != s_ScriptInstanceMap.end())
            {
                for (auto& [entityID, scriptInstance] : scriptInstanceMap->second)
                {
                    if (Entity* entity = worldSubsystem->GetEntityByID(entityID).get())
                    {
                        InitializeScriptInstance(entity->GetComponent<Script>());
                    }
                }
            }
        }
        
        return true;
    }

    void Scripting::ShutdownScriptInstance(Script* scriptComponent)
    {
        // Clear entity instance data.

        
        ScriptFieldMapping& fieldMap = scriptComponent->GetFieldMap();
        if (fieldMap.find(scriptComponent->GetModuleName()) != fieldMap.end())
        {
            fieldMap.erase(scriptComponent->GetModuleName());
        }
    }

    bool Scripting::InitializeScriptInstance(Script* scriptComponent)
    {
        uint32_t entityID = scriptComponent->GetEntity()->GetObjectID();
        std::string& moduleName = scriptComponent->GetModuleName();
        if (moduleName.empty())
        {
            return false;
        }

        if (!ModuleExists(moduleName))
        {
            AURORA_ERROR(LogLayer::Scripting, "Non-existent script is attempting to be compiled. Aborting... %s", moduleName.c_str());
            return false;
        }
      
        ScriptClassData& scriptClassData = s_ScriptClassMap[moduleName];
        scriptClassData.m_FullReferralName = moduleName;

        // If a namespace exists...
        if (moduleName.find('.') != std::string::npos)
        {
            scriptClassData.m_NamespaceName = moduleName.substr(0, moduleName.find_last_of('.'));
            scriptClassData.m_ClassName = moduleName.substr(moduleName.find_last_of('.') + 1);
        }
        else
        {
            scriptClassData.m_ClassName = moduleName;
        }

        scriptClassData.m_Class = GetMonoClass(s_ApplicationAssemblyImage, scriptClassData);
        scriptClassData.InitializeClassMethods(s_ApplicationAssemblyImage);

        ScriptInstance& scriptInstance = s_ScriptInstanceMap[0][entityID];
        ScriptInstanceData& scriptInstanceData = scriptInstance.m_ScriptClassData;
        scriptInstanceData.m_ScriptClassData = &scriptClassData; // Saves the created instance data above into our script instance.

        ScriptFieldMapping& scriptFieldMapping = scriptComponent->GetFieldMap();
        std::unordered_map<std::string, PublicField>& fieldMap = scriptFieldMapping[moduleName];

        // Save the old fields in field map. The point of this is that if you reload the script instance, the values of fields and properties are preserved as long as the reloaded entity has the same fields and the same type.
        std::unordered_map<std::string, PublicField> oldFields;
        oldFields.reserve(fieldMap.size());
        for (auto& [fieldName, field] : fieldMap)
        {
            oldFields.emplace(fieldName, std::move(field));
        }

        // Creation of new instance.
        scriptInstanceData.m_MonoGCHandle = InstantiateMonoObjectInstance(scriptClassData);

        // Call object constructor IF there are default parameters to be passed in. Else, our instantiator above alr calls it. 
        // InvokeMethod(scriptInstanceData.GetInstance(), scriptInstanceData.m_ScriptClassData->m_Constructor, nullptr);

        // Clear old fields and retrieve new (public) fields.
        fieldMap.clear();
        {
            MonoClassField* iterator;
            void* pointer = 0;
            while ((iterator = mono_class_get_fields(scriptClassData.m_Class, &pointer)) != nullptr)
            {
                const char* fieldName = mono_field_get_name(iterator);
                uint32_t fieldFlags = mono_field_get_flags(iterator);

                // Skip entirely if the field isn't public.
                if ((fieldFlags & MONO_FIELD_ATTR_PUBLIC) == 0)
                {
                    continue;
                }

                MonoType* fieldType = mono_field_get_type(iterator);
                FieldType auroraFieldType = GetAuroraFieldType(fieldType);
                
                char* fieldTypeName = mono_type_get_name(fieldType);

                auto oldField = oldFields.find(fieldName);
                if ((oldField != oldFields.end()) && (oldField->second.m_FieldTypeName == fieldTypeName)) // Field found, proceed.
                {
                    fieldMap.emplace(fieldName, std::move(oldFields.at(fieldName))); // Place in new map.
                    PublicField& field = fieldMap.at(fieldName);
                    field.m_MonoClassField = iterator;
                    continue;
                }

                /// To Do: Attributes
                MonoCustomAttrInfo* attribute = mono_custom_attrs_from_field(scriptClassData.m_Class, iterator);

                // New field. Hence, we add it to the field list.
                PublicField field = { fieldName, fieldTypeName, auroraFieldType };
                field.m_MonoClassField = iterator;
                field.CopyStoredValueFromRuntime(scriptInstanceData);
                fieldMap.emplace(fieldName, std::move(field));
            }
        }

        // Retrieve new public property fields.
        {
            MonoProperty* iterator;
            void* pointer = 0;
            while ((iterator = mono_class_get_properties(scriptClassData.m_Class, &pointer)) != nullptr)
            {
                const char* propertyName = mono_property_get_name(iterator);

                if (oldFields.find(propertyName) != oldFields.end()) // Found property...
                {
                    fieldMap.emplace(propertyName, std::move(oldFields.at(propertyName)));
                    PublicField& field = fieldMap.at(propertyName);
                    field.m_MonoProperty = iterator;
                    continue;
                }

                MonoMethod* propertySetter = mono_property_get_set_method(iterator);
                MonoMethod* propertyGetter = mono_property_get_get_method(iterator);

                uint32_t setterFlags = 0;
                uint32_t getterFlags = 0;

                bool isReadOnly = false;
                MonoType* monoType = nullptr;

                if (propertySetter)
                {
                    void* i = nullptr;
                    MonoMethodSignature* signature = mono_method_signature(propertySetter);
                    setterFlags = mono_method_get_flags(propertySetter, nullptr);
                    isReadOnly = (setterFlags & MONO_METHOD_ATTR_PRIVATE) != 0;
                    monoType = mono_signature_get_params(signature, &i);
                }

                if (propertyGetter)
                {
                    MonoMethodSignature* signature = mono_method_signature(propertyGetter);
                    getterFlags = mono_method_get_flags(propertyGetter, nullptr);

                    if (monoType != nullptr)
                    {
                        monoType = mono_signature_get_return_type(signature);
                    }

                    if ((getterFlags & MONO_METHOD_ATTR_PRIVATE) != 0) // If the field is private...
                    {
                        continue;
                    }
                }

                if ((setterFlags & MONO_METHOD_ATTR_STATIC) != 0) // If the field is static...
                {
                    continue;
                }

                FieldType auroraFieldType = GetAuroraFieldType(monoType);
                /// If it is an entity type... skip.

                char* typeName = mono_type_get_name(monoType);

                PublicField field = { propertyName, typeName, auroraFieldType, isReadOnly };
                field.m_MonoProperty = iterator;
                field.CopyStoredValueFromRuntime(scriptInstanceData);
                fieldMap.emplace(propertyName, std::move(field));
            }
        }

        PrintMonoClassFields(scriptClassData.m_Class);
        PrintMonoClassMethods(scriptClassData.m_Class);

        DestroyScriptInstance(scriptInstanceData.m_MonoGCHandle);
        
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

    MonoClass* Scripting::GetMonoClass(MonoImage* image, const ScriptClassData& scriptInstanceData)
    {
        MonoClass* monoClass = mono_class_from_name(image, scriptInstanceData.m_NamespaceName.c_str(), scriptInstanceData.m_ClassName.c_str());
        if (!monoClass)
        {
            AURORA_ERROR(LogLayer::Scripting, "Mono_Class_From_Name failed.");
            return nullptr;
        }

        return monoClass;
    }

    uint32_t Scripting::InstantiateMonoObjectInstance(ScriptClassData& scriptInstanceData)
    {
        MonoObject* monoObjectInstance = mono_object_new(s_CurrentMonoDomain, scriptInstanceData.m_Class);
        if (!monoObjectInstance)
        {
            AURORA_ERROR(LogLayer::Scripting, "Mono_Object_New failed.");
            return 0;
        }

        // Invoke argumentless constructor.
        mono_runtime_object_init(monoObjectInstance);
        uint32_t monoGCHandle = mono_gchandle_new(monoObjectInstance, false);

        return monoGCHandle;
    }

    void Scripting::PrintMonoClassMethods(MonoClass* monoClass)
    {
        MonoMethod* iterator;
        void* pointer = 0;

        while ((iterator = mono_class_get_methods(monoClass, &pointer)) != nullptr)
        {
            AURORA_INFO(LogLayer::Scripting, "-------------------");
            const char* methodName = mono_method_get_name(iterator);
            MonoMethodDesc* methodDescription = mono_method_desc_from_method(iterator);

            AURORA_INFO(LogLayer::Scripting, "Method Name: %s", methodName);
            AURORA_INFO(LogLayer::Scripting, "Full Name: %s", mono_method_full_name(iterator, true));
        }
    }

    void Scripting::PrintMonoClassFields(MonoClass* monoClass)
    {
        MonoClassField* iterator;
        void* pointer = 0;

        while ((iterator = mono_class_get_fields(monoClass, &pointer)) != nullptr)
        {
            AURORA_INFO(LogLayer::Scripting, "-------------------");
            const char* fieldName = mono_field_get_name(iterator);
            AURORA_INFO(LogLayer::Scripting, "Field Name: %s", fieldName);
        }
    }

    bool Scripting::ModuleExists(const std::string& moduleName)
    {
        if (!s_ApplicationAssemblyImage) // If no assembly is currently loaded...
        {
            return false;
        }

        std::string namespaceName, className;
        if (moduleName.find('.') != std::string::npos)
        {
            namespaceName = moduleName.substr(0, moduleName.find_last_of('.'));
            className = moduleName.substr(moduleName.find_last_of(".") + 1);
        }
        else
        {
            className = moduleName;
        }

        MonoClass* monoClass = mono_class_from_name(s_ApplicationAssemblyImage, namespaceName.c_str(), className.c_str());
        if (!monoClass)
        {
            return false;
        }

        return true;
    }

    std::string Scripting::StripNamespace(const std::string& namespaceName, const std::string& moduleName)
    {
        std::string name = moduleName;
        size_t position = name.find(namespaceName + ".");
        if (position == 0) // Namespace will begin at position 0 as the syntax for modules are Aurora.Derp
        {
            name.erase(position, namespaceName.length() + 1);
        }

        return name;
    }

    MonoObject* Scripting::InvokeMethod(MonoObject* monoObject, MonoMethod* monoMethod, void** parameters)
    {
        MonoObject* exceptionPointer = nullptr;
        MonoObject* result = mono_runtime_invoke(monoMethod, monoObject, parameters, &exceptionPointer);
        if (exceptionPointer)
        {
            /// More in depth checking?
            AURORA_ERROR(LogLayer::Scripting, "An error occured with method invocation.");
            return nullptr;
        }

        return result;
    }
}
