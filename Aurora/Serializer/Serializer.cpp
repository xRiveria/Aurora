#include "Aurora.h"
#include "Serializer.h"
#include "SerializerUtilities.h"
#include "../Scene/World.h"
#include <yaml-cpp/yaml.h>
#include <fstream>

namespace Aurora
{
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

            SerializeEntity(out, entity);
        }

        out << YAML::EndSeq; // End Array.
        out << YAML::EndMap;

        std::ofstream fout("../Resources/Scenes/TestScene.aurora");
        fout << out.c_str();
        AURORA_INFO("Scene Successfully Serialized.");
    }

    void Serializer::SerializeEntity(YAML::Emitter& outputStream, Entity* entity)
    {
        outputStream << YAML::BeginMap; // Entity

        outputStream << YAML::Key << "Entity";
        outputStream << YAML::Value << entity->GetObjectID(); /// Can be different?

        // All entities have a name. We treat is as a component in our serializing system for consistency.

        if (true) // If our name component exists...
        {
            outputStream << YAML::Key << "Name Component";
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
}