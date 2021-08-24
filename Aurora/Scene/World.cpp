#include "Aurora.h"
#include "../Window/WindowContext.h"
#include "World.h"
#include "Components/Light.h"
#include "Components/Camera.h"
#include "../Renderer/Model.h"

using namespace DirectX;

namespace Aurora
{
    World::World(EngineContext* engineContext) : ISubsystem(engineContext)
    {

    }

    World::~World()
    {

    }

    bool World::Initialize()
    {
        SetWorldName(GetWorldName());
        CreateCamera();
        // CreateEnvironment();
        // CreateDirectionalLight();

        // Tick Entities.
        for (std::shared_ptr<Entity>& entity : m_Entities)
        {
            entity->Start();
        }

        return true;
    }

    void World::Tick(float deltaTime)
    {
        // If something is being loaded, don't think as entities are probably being added.
        if (IsLoading())
        {
            return;
        }

        // Tick Entities.
        {
            // Detect game toggling.
            const bool runtimeStarted = m_EngineContext->GetEngine()->EngineFlag_IsSet(EngineFlag::EngineFlag_TickGame) && m_WasInEditorMode;
            const bool runtimeStopped = !m_EngineContext->GetEngine()->EngineFlag_IsSet(EngineFlag::EngineFlag_TickGame) && !m_WasInEditorMode;
            m_WasInEditorMode = !m_EngineContext->GetEngine()->EngineFlag_IsSet(EngineFlag::EngineFlag_TickGame);

            // Started
            if (runtimeStarted)
            {
                for (std::shared_ptr<Entity>& entity : m_Entities)
                {
                    entity->Start();
                }
            }

            // Stopped
            if (runtimeStopped)
            {
                for (std::shared_ptr<Entity>& entity : m_Entities)
                {
                    entity->Stop();
                }
            }

            for (std::shared_ptr<Entity>& entity : m_Entities)
            {
                entity->Tick(deltaTime);
            }
        }

        if (m_IsSceneDirty)
        {
            // Make a copy so we can iterate while removing entities.
            std::vector<std::shared_ptr<Entity>> entitiesCopied = m_Entities;
            
            for (std::shared_ptr<Entity>& entity : entitiesCopied)
            {
                if (entity->IsPendingDestruction())
                {
                    _EntityRemove(entity);
                }
            }
        }

        m_IsSceneDirty = false;
    }

    void World::EntityRemove(const std::shared_ptr<Entity>& entity)
    {
        if (!entity)
        {
            return;
        }

        // We will simply mark the entity for destruction instead of outright deleting it. This is because the entity might still be in use. We thus remove it at the end of the frame.
        entity->MarkForDestruction();

        m_IsSceneDirty = true;
    }

    // Removes an entity and all of its children.
    void World::_EntityRemove(const std::shared_ptr<Entity>& entity)
    {
        // Remove any descendants.
        std::vector<Transform*> children = entity->GetTransform()->GetChildren();
        for (const auto& child : children)
        {
            EntityRemove(child->GetEntity()->GetPointerShared());
        }

        // Keep a reference to its parent (in case it has one).
        Transform* parentEntity = entity->GetTransform()->GetParentTransform();

        // Remove this entity.
        for (auto it = m_Entities.begin(); it < m_Entities.end();)
        {
            const std::shared_ptr<Entity> temporaryReference = *it;
            if (temporaryReference->GetObjectID() == entity->GetObjectID())
            {
                // AURORA_ERROR("Matched! Deleting...");
                it = m_Entities.erase(it);
                break;
            }

            ++it;
        }

        // If there was a parent, update it.
        if (parentEntity)
        {
            parentEntity->AcquireChildren();
        }
    }

    void World::New()
    {
        Clear();
    }

    void World::Clear()
    {
        AURORA_INFO(LogLayer::ECS, "%f", static_cast<float>(m_Entities.size()));
        for (int i = 0; i < m_Entities.size(); i++)
        {
            if (m_Entities[i]->GetEntityName() == "Directional Light" || m_Entities[i]->GetEntityName() == "Default_Camera") // Skip all of them for now.
            {
                continue;
            }
            else
            {
                m_Entities[i]->MarkForDestruction();
                EntityRemove(m_Entities[i]);
            }
        }

        // Clear our entities.
        // m_Entities.clear();

        m_IsSceneDirty = true;
    }

    void World::SetSceneDirty()
    {
        m_IsSceneDirty = true;
    }

    bool World::IsLoading()
    {
        return false;
    }

    std::shared_ptr<Entity> World::EntityCreate(bool isActive)
    {
        std::shared_ptr<Entity> entity = m_Entities.emplace_back(std::make_shared<Entity>(m_EngineContext));
        entity->SetActive(isActive);
        return entity;
    }

    bool World::EntityExists(const std::shared_ptr<Entity>& entity)
    {
        if (!entity)
        {
            return false;
        }

        return GetEntityByID(entity->GetObjectID()) != nullptr;
    }

    const std::vector<std::shared_ptr<Entity>>& World::GetEntitiesByComponent(ComponentType componentType)
    {
        std::vector<std::shared_ptr<Entity>> componentEntities;

        for (const std::shared_ptr<Entity>& entity : m_Entities)
        {
            if (entity->HasComponent(componentType))
            {
                componentEntities.emplace_back(entity);
            }
        }

        return componentEntities;
    }

    const std::shared_ptr<Entity>& World::GetEntityByName(const std::string& entityName)
    {
        for (const std::shared_ptr<Entity>& entity : m_Entities)
        {
            if (entity->GetEntityName() == entityName)
            {
                return entity;
            }
        }
        
        AURORA_ERROR(LogLayer::ECS, "Entity not found!");
        static std::shared_ptr<Entity> emptyEntity;
        return emptyEntity;
    }

    const std::shared_ptr<Entity>& World::GetEntityByID(uint32_t entityID)
    {
        for (const std::shared_ptr<Entity>& entity : m_Entities)
        {
            if (entity->GetObjectID() == entityID)
            {
                return entity;
            }
        }

        static std::shared_ptr<Entity> emptyEntity;
        return emptyEntity;
    }

    std::vector<std::shared_ptr<Entity>> World::EntityGetRoots()
    {
        std::vector<std::shared_ptr<Entity>> rootEntities;

        for (const std::shared_ptr<Entity>& entity : m_Entities)
        {
            if (entity != nullptr)
            {
                if (entity->GetTransform()->IsRootTransform())
                {
                    rootEntities.emplace_back(entity);
                }
            }
        }

        return rootEntities;
    }

    bool World::SerializeScene(const std::string& filePath)
    {
        const Stopwatch stopwatch("Scene Saving", false);

        // Add scene file extension to the file path if its missing.
        std::string filePathIn = filePath;
        if (FileSystem::GetExtensionFromFilePath(filePath) != EXTENSION_SCENE)
        {
            filePathIn += EXTENSION_SCENE;
        }

        m_WorldName = FileSystem::GetFileNameWithoutExtensionFromFilePath(filePathIn);
        m_WorldFilePath = filePathIn;
        m_EngineContext->GetSubsystem<WindowContext>()->SetCurrentContextTitle_Scene(m_WorldName);

        /// Notify all subsystems that we are now saving data. For now, just our cache.
        m_EngineContext->GetSubsystem<ResourceCache>()->SaveResourcesToFiles();

        // Create a scene file.
        std::unique_ptr<BinarySerializer> fileSerializer = std::make_unique<BinarySerializer>(filePathIn, SerializerFlag::SerializerMode_Write);
        if (!fileSerializer->IsStreamOpen())
        {
            AURORA_ERROR(LogLayer::Serialization, "Failed to open file stream for scene serialization.");
            return false;
        }

        // Only save root entities as they will also save their descendants.
        auto rootEntities = EntityGetRoots();
        const uint32_t rootEntityCount = static_cast<uint32_t>(rootEntities.size());

        /// Progress tracking.

        // Save root entity count.
        fileSerializer->Write(rootEntityCount);

        // Save root entity IDs.
        for (const auto& rootEntity : rootEntities)
        {
            fileSerializer->Write(rootEntity->GetObjectID());
        }

        // Save the root entities themselves.
        for (const auto& rootEntity : rootEntities)
        {
            rootEntity->Serialize(fileSerializer.get());
            /// Progress tracking.
        }

        /// Notify subsystems waiting for us to finish.

        return true;
    }

    bool World::DeserializeScene(const std::string& filePath)
    {
        if (!FileSystem::Exists(filePath))
        {
            AURORA_ERROR(LogLayer::Serialization, "%s was not found.", filePath.c_str());
            return false;
        }

        // Very file extension.
        if (FileSystem::GetExtensionFromFilePath(filePath) != EXTENSION_SCENE)
        {
            AURORA_ERROR(LogLayer::Serialization, "Scene files end with a (.aurora) extension. Is \"%s\" a valid scene file?.", filePath.c_str());
            return false;
        }

        // Open file.
        std::unique_ptr<BinarySerializer> binaryDeserializer = std::make_unique<BinarySerializer>(filePath, SerializerFlag::SerializerMode_Read);
        if (!binaryDeserializer->IsStreamOpen())
        {
            return false;
        }

        m_WorldName = FileSystem::GetFileNameWithoutExtensionFromFilePath(filePath);
        m_WorldFilePath = filePath;
        m_EngineContext->GetSubsystem<WindowContext>()->SetCurrentContextTitle_Scene(m_WorldName);

        const Stopwatch stopwatch("Deserializing Scene", false);

        // Clear current entities.
        Clear();


        // Notify subsystems that we are loading data.
        m_EngineContext->GetSubsystem<ResourceCache>()->LoadResourcesFromFiles();

        // Load root entity count.
        const uint32_t rootEntityCount = binaryDeserializer->ReadAs<uint32_t>();

        // Load root entity IDs.
        for (uint32_t i = 0; i < rootEntityCount; i++)
        {
            std::shared_ptr<Entity> entity = EntityCreate();
            entity->SetObjectID(binaryDeserializer->ReadAs<uint32_t>());
        }

        // Deserialize root entities.
        for (uint32_t i = 0; i < rootEntityCount; i++)
        {
            m_Entities[i]->Deserialize(binaryDeserializer.get(), nullptr);
            /// Tracker.
        }

        return true;
    }

    void World::SetWorldName(const std::string& worldName)
    {
        m_WorldName = worldName;
        m_EngineContext->GetSubsystem<WindowContext>()->SetCurrentContextTitle_Scene(GetWorldName());
    }

    void World::CreateDirectionalLight()
    {
        std::shared_ptr<Entity> entity = EntityCreate();
        entity->SetEntityName("Directional_Light");
        entity->AddComponent<Light>();
    }

    void World::CreateCamera()
    {
        std::shared_ptr<Entity> entity = EntityCreate();
        entity->SetEntityName("Default_Camera");
        entity->AddComponent<Camera>(); 

        m_CameraPointer = entity.get();
    }

    void World::CreateEnvironment()
    {

    }

    bool World::CreateDefaultObject(DefaultObjectType defaultObjectType)
    {
        switch (defaultObjectType)
        {
            case DefaultObjectType::DefaultObjectType_Cube:
                m_EngineContext->GetSubsystem<ResourceCache>()->Load<Model>("../Resources/Models/Default/Cube.fbx");
                return true;

            case DefaultObjectType::DefaultObjectType_Sphere:
                m_EngineContext->GetSubsystem<ResourceCache>()->Load<Model>("../Resources/Models/Default/Sphere.fbx");
                return true;

            case DefaultObjectType::DefaultObjectType_Plane:
                m_EngineContext->GetSubsystem<ResourceCache>()->Load<Model>("../Resources/Models/Default/Plane.fbx");
                return true;

            case DefaultObjectType::DefaultObjectType_Cylinder:
                m_EngineContext->GetSubsystem<ResourceCache>()->Load<Model>("../Resources/Models/Default/Cylinder.fbx");
                return true;

            case DefaultObjectType::DefaultObjectType_Torus:
                m_EngineContext->GetSubsystem<ResourceCache>()->Load<Model>("../Resources/Models/Default/Torus.fbx");
                return true;

            case DefaultObjectType::DefaultObjectType_Cone:
                m_EngineContext->GetSubsystem<ResourceCache>()->Load<Model>("../Resources/Models/Default/Cone.fbx");
                return true;
        }

        return false;
    }
}