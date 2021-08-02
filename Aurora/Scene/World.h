#pragma once
#include "EngineContext.h"
#include "ISubsystem.h"
#include "Entity.h"
#include "../Serializer/Serializer.h"

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
        std::vector<std::shared_ptr<Entity>> EntityGetRoots();

        // Default Components - In the future, we ought to load the model into memory at runtime, set to inactive and simply duplicate it when ready.
        std::shared_ptr<Entity> CreateDefaultObject(DefaultObjectType defaultObjectType);

        // Serializing
        void SerializeScene(const std::string& filePath);
        void DeserializeScene(const std::string& filePath);

        void SetWorldName(const std::string& worldName);
        std::string GetWorldName() const { return m_WorldName; }  

    private:
        void _EntityRemove(const std::shared_ptr<Entity>& entity);

        // Default Components
        void CreateDirectionalLight();
        void CreateCamera();
        void CreateEnvironment();

    private:
        std::shared_ptr<Serializer> m_Serializer;
        std::string m_WorldName = "Untitled_Scene";  // Or Scene Name.

        bool m_IsSceneDirty = false;

        std::vector<std::shared_ptr<Entity>> m_Entities;
    };
}