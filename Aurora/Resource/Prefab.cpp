#include "Aurora.h"
#include "Prefab.h"
#include "../Scene/World.h"

namespace Aurora
{
    Prefab::Prefab(EngineContext* engineContext) : AuroraResource(engineContext, ResourceType::ResourceType_Prefab)
    {

    }

    void Prefab::AppendToPrefab(Entity* entity)
    {
        m_PrefabEntities.emplace_back(entity);
    }

    bool Prefab::SaveToFile(const std::string& filePath)
    {
        // Add prefab file extension to the file path if it is missing.
        std::string finalFilePath = filePath;
        if (FileSystem::GetExtensionFromFilePath(filePath) != EXTENSION_PREFAB)
        {
            finalFilePath += EXTENSION_PREFAB;
        }

        m_ObjectName = FileSystem::GetFileNameFromFilePath(filePath);
        SetResourceFilePath(finalFilePath);

        // Create a prefab file.
        std::unique_ptr<BinarySerializer> fileSerializer = std::make_unique<BinarySerializer>(finalFilePath, SerializerFlag::SerializerMode_Write);
        if (!fileSerializer->IsStreamOpen())
        {
            AURORA_ERROR(LogLayer::Serialization, "Failed to open file stream for scene serialization.");
            return false;
        }

        // Save root entities as this will also save their descendants.
        fileSerializer->Write(static_cast<uint32_t>(m_PrefabEntities.size()));

        // Save the entity themselves.
        for (Entity* entity : m_PrefabEntities)
        {
            entity->Serialize(fileSerializer.get());
        }

        AURORA_INFO(LogLayer::Serialization, "Successfully Serialized: %s", finalFilePath.c_str());

        return true;
    }

    bool Prefab::LoadFromFile(const std::string& filePath)
    {
        if (filePath.empty() || FileSystem::IsDirectory(filePath))
        {
            AURORA_ERROR(LogLayer::Serialization, "Invalid Prefab File Path: %s", filePath.c_str());
            return false;
        }

        if (FileSystem::GetExtensionFromFilePath(filePath) == EXTENSION_PREFAB)
        {
            SetResourceFilePath(filePath);

            // Open file.
            std::unique_ptr<BinarySerializer> binaryDeserializer = std::make_unique<BinarySerializer>(filePath, SerializerFlag::SerializerMode_Read);
            if (!binaryDeserializer->IsStreamOpen())
            {
                return false;
            }

            // Load root entity count.
            const uint32_t rootEntityCount = binaryDeserializer->ReadAs<uint32_t>();

            std::vector<uint32_t> entityIDs;

            // Deserialize root entities.
            for (uint32_t i = 0; i < rootEntityCount; i++)
            {
                std::shared_ptr<Entity> entity = m_EngineContext->GetSubsystem<World>()->EntityCreate();              
                entity->Deserialize(binaryDeserializer.get(), nullptr);

                // Generate a new ID for the spawned prefab.
                entity->SetObjectID(entity->GenerateObjectID());
                for (auto& child : entity->GetTransform()->GetChildren())
                {
                    child->GetEntity()->SetObjectID(child->GetEntity()->GenerateObjectID());
                }

                /// Tracker.
            }
        }
        else
        {
            AURORA_ERROR(LogLayer::Serialization, "Invalid Prefab File Path: %s", filePath.c_str());
            return false;
        }

        return true;
    }
}