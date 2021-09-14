#pragma once
#include "AuroraResource.h"

// Prefabs are essentially "boxes" around various entities. It comes with the entities, arranged in whatever way we put them in the box.

namespace Aurora
{
    class Prefab : public AuroraResource
    {
    public:
        Prefab(EngineContext* engineContext);

        void AppendToPrefab(Entity* entity);

        bool SaveToFile(const std::string& filePath) override;
        bool LoadFromFile(const std::string& filePath) override;

    private:
        // A prefab is essentially made up of entities with its own component data, including transforms, meshs amongst others.
        std::vector<Entity*> m_PrefabEntities;
    };
}