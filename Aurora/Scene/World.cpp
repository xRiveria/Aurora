#include "Aurora.h"
#include "../Window/WindowContext.h"
#include "World.h"
#include "Components/Light.h"
#include "Components/Mesh.h"
#include "Components/Camera.h"

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

        m_Serializer = std::make_shared<Serializer>(m_EngineContext, this);

        return true;
    }

    void World::Tick(float deltaTime)
    {
        // If something is being loaded, don't think as entities are probably being added.
        if (IsLoading())
        {
            return;
        }

        for (std::shared_ptr<Entity>& entity : m_Entities)
        {
            entity->Tick(deltaTime);
        }

        if (m_IsSceneDirty)
        {
            // Make a copy so we can iterate while remove entities.
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
            if (m_Entities[i]->m_EntityType == EntityType::Skybox || m_Entities[i]->GetObjectName() == "Directional Light" || m_Entities[i]->GetObjectName() == "Default_Camera") // Skip all of them for now.
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

    const std::shared_ptr<Entity>& World::GetEntityByName(const std::string& entityName)
    {
        for (const std::shared_ptr<Entity>& entity : m_Entities)
        {
            if (entity->GetObjectName() == entityName)
            {
                return entity;
            }
        }
        
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
            if (entity->GetTransform()->IsRootTransform())
            {
                rootEntities.emplace_back(entity);
            }
        }

        return rootEntities;
    }

    bool World::CreateDefaultObject(DefaultObjectType defaultObjectType)
    {
        m_EngineContext->GetSubsystem<ResourceCache>()->m_Resources.push_back(std::make_shared<AuroraResource>());
        auto& object = m_EngineContext->GetSubsystem<ResourceCache>()->m_Resources.back();

        switch (defaultObjectType)
        {
            case DefaultObjectType::DefaultObjectType_Cube:
                m_EngineContext->GetSubsystem<ResourceCache>()->LoadModel("../Resources/Models/Default/Cube.fbx", object, false);
                return true;

            case DefaultObjectType::DefaultObjectType_Sphere:
                m_EngineContext->GetSubsystem<ResourceCache>()->LoadModel("../Resources/Models/Default/Sphere.fbx", object, false);
                return true;

            case DefaultObjectType::DefaultObjectType_Plane:
                m_EngineContext->GetSubsystem<ResourceCache>()->LoadModel("../Resources/Models/Default/Plane.fbx", object, false);
                return true;

            case DefaultObjectType::DefaultObjectType_Cylinder:
                m_EngineContext->GetSubsystem<ResourceCache>()->LoadModel("../Resources/Models/Default/Cylinder.fbx", object, false);
                return true;

            case DefaultObjectType::DefaultObjectType_Torus:
                m_EngineContext->GetSubsystem<ResourceCache>()->LoadModel("../Resources/Models/Default/Torus.fbx", object, false);
                return true;

            case DefaultObjectType::DefaultObjectType_Cone:
                m_EngineContext->GetSubsystem<ResourceCache>()->LoadModel("../Resources/Models/Default/Cone.fbx", object, false);
                return true;
        }
        
        return false;
    }

    void World::SerializeScene(const std::string& filePath)
    {
        m_Serializer->SerializeScene(filePath);
    }

    void World::DeserializeScene(const std::string& filePath)
    {
        m_Serializer->DeserializeScene(filePath);
    }

    void World::SetWorldName(const std::string& worldName)
    {
        m_WorldName = worldName;
        m_EngineContext->GetSubsystem<WindowContext>()->SetCurrentContextTitle_Scene(GetWorldName());
    }

    void World::CreateDirectionalLight()
    {
        std::shared_ptr<Entity> entity = EntityCreate();
        entity->SetName("Directional_Light");
        entity->AddComponent<Light>();
    }

    void World::CreateCamera()
    {
        std::shared_ptr<Entity> entity = EntityCreate();
        entity->SetName("Default_Camera");
        entity->AddComponent<Camera>(); 
    }

    void World::CreateEnvironment()
    {

    }
}