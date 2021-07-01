#pragma once
#include "EngineContext.h"
#include "ISubsystem.h"
#include "Entity.h"

namespace Aurora
{
    class World : public ISubsystem
    {
    public:
        World(EngineContext* engineContext);
        ~World();

        bool Initialize() override;
        void Tick(float deltaTime) override;

        // Entity
        void New();
        void Clear();

        bool IsLoading();
        
        // Entities
        std::shared_ptr<Entity> EntityCreate(bool isActive = true);
        bool EntityExists(const std::shared_ptr<Entity>& entity);
        void EntityRemove(const std::shared_ptr<Entity>& entity);

        const std::shared_ptr<Entity>& GetEntityByName(const std::string& entityName);
        const std::shared_ptr<Entity>& GetEntityByID(uint32_t entityID);
        const std::vector<std::shared_ptr<Entity>>& EntityGetAll() const { return m_Entities; }

    private:
        void _EntityRemove(const std::shared_ptr<Entity>& entity);

        // Default Components
        void CreateDirectionalLight();
        void CreateCamera();
        void CreateEnvironment();

    private:
        std::string m_WorldName;  // Or Scene Name.

        std::vector<std::shared_ptr<Entity>> m_Entities;
    };
}