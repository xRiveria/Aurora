#include "Aurora.h"
#include "World.h"
#include "Components/Light.h"

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
        CreateCamera();
        // CreateEnvironment();
        CreateDirectionalLight();

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

        for (std::shared_ptr<Entity>& entity : m_Entities)
        {
            entity->Tick(deltaTime);
        }
    }

    void World::New()
    {
        Clear();
    }

    void World::Clear()
    {
        // Clear our entities.
        m_Entities.clear();
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

    void World::EntityRemove(const std::shared_ptr<Entity>& entity)
    {
        if (!entity)
        {
            return;
        }

        // We will simply mark the entity for destruction instead of outright deleting it. This is because the entity might still be in use. We thus remove it at the end of the frame.
        entity->MarkForDestruction();

        /// Fire remove event.
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