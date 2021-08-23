#pragma once
#include "EngineContext.h"
#include "ISubsystem.h"
#include "Entity.h"
#include "../Serializer/Serializer.h"
#include "../Resource/ResourceCache.h"

namespace Aurora
{
    class World : public ISubsystem
    {
    public:
        World(EngineContext* engineContext);
        ~World();

        bool Initialize() override;
        void Tick(float deltaTime) override;

        // Loading/Deserializing
        bool SerializeScene(const std::string& filePath);
        bool DeserializeScene(const std::string& filePath);

        // Entity
        void New();
        void Clear();
        void SetSceneDirty();

        bool IsLoading();
        
        // Entities
        std::shared_ptr<Entity> EntityCreate(bool isActive = true);
        // std::shared_ptr<Entity> EntityClone(AuroraResource* auroraResource, bool isActive = true);
        bool EntityExists(const std::shared_ptr<Entity>& entity);
        void EntityRemove(const std::shared_ptr<Entity>& entity);

        const std::vector<std::shared_ptr<Entity>>& GetEntitiesByComponent(ComponentType componentType);
        const std::shared_ptr<Entity>& GetEntityByName(const std::string& entityName);
        const std::shared_ptr<Entity>& GetEntityByID(uint32_t entityID);
        const std::vector<std::shared_ptr<Entity>>& EntityGetAll() const { return m_Entities; }
        std::vector<std::shared_ptr<Entity>> EntityGetRoots();

        bool CreateDefaultObject(DefaultObjectType defaultObjectType);

        void SetWorldName(const std::string& worldName);
        std::string GetWorldName() const { return m_WorldName; }  
        std::string GetWorldFilePath() const { return m_WorldFilePath; }

    public:
        Entity* m_CameraPointer;

    private:
        void _EntityRemove(const std::shared_ptr<Entity>& entity);

        // Default Components
        void CreateDirectionalLight();
        void CreateCamera();
        void CreateEnvironment();

    private:
        std::string m_WorldName = "Untitled_Scene";  // Or Scene Name.
        std::string m_WorldFilePath = "Unknown";

        bool m_IsSceneDirty = true;
        bool m_WasInEditorMode = false;

        std::vector<std::shared_ptr<Entity>> m_Entities;
    };
}