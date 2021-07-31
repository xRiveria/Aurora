#include "Aurora.h"
#include "Serializer.h"
#include "SerializerUtilities.h"
#include "../Scene/World.h"
#include "../Scene/Components/Light.h"
#include <yaml-cpp/yaml.h>
#include <fstream>

namespace Aurora
{
    // Remember to not have spacings in your YAML keys.
    Serializer::Serializer(World* world) : m_World(world)
    {

    }

    void Serializer::SerializeEntity(YAML::Emitter& outputStream, Entity* entity)
    {
        outputStream << YAML::BeginMap; // Entity

        outputStream << YAML::Key << "Entity" << YAML::Value << entity->GetObjectID();

        // All entities have a name. We treat it as a component in our serializing system for consistency.

        if (true) // If our name component exists...
        {
            outputStream << YAML::Key << "NameComponent";
            outputStream << YAML::BeginMap; // Tag Component

            const std::string& entityName = entity->GetObjectName();
            outputStream << YAML::Key << "Name";
            outputStream << YAML::Value << entityName;

            outputStream << YAML::EndMap; // Tag Component
        }

        for (int i = 0; i < entity->GetAllComponents().size(); i++)
        {
            entity->GetAllComponents()[i]->Serialize(outputStream);
        }

        outputStream << YAML::EndMap; // Entity
    }

    void Serializer::SerializeScene(World* scene)
    {
        YAML::Emitter out;
        out << "This is a scene save file for Aurora Engine created by Ryan Tan.";

        // Begin a mapping of data.
        out << YAML::BeginMap;

        // Stores our scene name in the "Scene" key.
        out << YAML::Key << "Scene";
        out << YAML::Value << "Unnamed Scene";

        // Stores our entities.
        out << YAML::Key << "Entities";
        out << YAML::Value << YAML::BeginSeq; // Begin a sequence (array?).

        for (int i = 0; i < scene->EntityGetAll().size(); i++)
        {
            Entity* entity = scene->EntityGetAll()[i].get();
            if (!entity)
            {
                return;
            }

            if (entity->m_EntityType == EntityType::Skybox || entity->GetObjectName() == "Directional Light" || entity->GetObjectName() == "Default_Camera") // Skip all of them for now.
            {
                continue; 
            }

            SerializeEntity(out, entity);
        }

        out << YAML::EndSeq; // End Array.
        out << YAML::EndMap;

        std::ofstream fout("../Resources/Scenes/DefaultScene.aurora");
        fout << out.c_str();
        AURORA_INFO("Scene Successfully Serialized.");
    }

    bool Serializer::DeserializeScene(const std::string& filePath)
    {
        // Localize file.
        YAML::Node sceneData = YAML::LoadFile(filePath); // sceneData now contains our scene's data as read from the stream.
        if (!sceneData["Scene"]) // Is this really a scene file? If there's no scene node within, we return immediately.
        {
            return false;
        }

        std::string sceneName = sceneData["Scene"].as<std::string>();

        AURORA_INFO("Deserializing Scene: %s.", sceneName.c_str());

        YAML::Node entities = sceneData["Entities"];
        if (entities) // Ensure that we the Entities node exist.
        {
            for (auto entity : entities) // Remember that we serialized them as a sequence. Hence, we are retrieving them via indexing like an array.
            {
                uint64_t entityID = entity["Entity"].as<uint64_t>();

                std::string entityName;
                YAML::Node nameNode = entity["NameComponent"];
                if (nameNode)
                {
                    entityName = nameNode["Name"].as<std::string>(); // Retrieve the name value from its Key.
                }
                
                AURORA_INFO("Deserialized Entity \"%s\" with ID: %f", entityName.c_str(), static_cast<float>(entityID));

                // ==== Components ====

                std::shared_ptr<Entity> deserializedEntity = m_World->EntityCreate();
                deserializedEntity->SetName(entityName);
                deserializedEntity->SetObjectID(entityID);

                // Entities will always have transforms.
                YAML::Node transformNode = entity["TransformComponent"];
                if (transformNode)
                {
                    Transform* transformComponent = deserializedEntity->AddComponent<Transform>();
                    transformComponent->Deserialize(transformNode);
                }

                YAML::Node lightNode = entity["LightComponent"];
                if (lightNode)
                {
                    Light* lightComponent = deserializedEntity->AddComponent<Light>();
                    lightComponent->Deserialize(lightNode);
                }

                // Mesh
                YAML::Node meshNode = entity["MeshComponent"];
                if (meshNode)
                {
                    Mesh* meshComponent = deserializedEntity->AddComponent<Mesh>();
                    meshComponent->Deserialize(meshNode);
                }

                // A mesh comes with a material precreated with it. Hence, we don't need to add any as they're already added. Likewise, we don't have to bother with it if a mesh doesn't exist.
                if (meshNode)
                {
                    YAML::Node materialNode = entity["MaterialComponent"];
                    if (materialNode)
                    {
                        Material* materialComponent = deserializedEntity->AddComponent<Material>();
                        materialComponent->Deserialize(materialNode);
                    }
                }
            }
        }

        return true;
    }
}