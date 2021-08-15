#include "Aurora.h"
#include "Material.h"
#include "../Resource/ResourceCache.h"
#include "../Resource/Importers/Importer_Image.h"

namespace Aurora
{
    Material::Material(EngineContext* engineContext, Entity* entity, uint32_t componentID) : IComponent(engineContext, entity, componentID)
    {

    }

    void Material::Serialize(SerializationStream& outputStream)
    {
        outputStream << YAML::Key << "MaterialComponent";
        outputStream << YAML::BeginMap;

        outputStream << YAML::Key << "BaseColor" << YAML::Value << m_BaseColor;
        outputStream << YAML::Key << "Roughness" << YAML::Value << m_Roughness;
        outputStream << YAML::Key << "Metalness" << YAML::Value << m_Metalness;

        if (m_Textures[TextureSlot::BaseColorMap] != nullptr)
        {
            outputStream << YAML::Key << "ColorMapPath" << YAML::Value << m_Textures[TextureSlot::BaseColorMap]->m_FilePath;
        }

        if (m_Textures[TextureSlot::MetalnessMap] != nullptr)
        {
            outputStream << YAML::Key << "MetallicMapPath" << YAML::Value << m_Textures[TextureSlot::MetalnessMap]->m_FilePath;
        }

        if (m_Textures[TextureSlot::MetalnessMap] != nullptr)
        {
            outputStream << YAML::Key << "RoughnessMapPath" << YAML::Value << m_Textures[TextureSlot::RoughnessMap]->m_FilePath;
        }

        if (m_Textures[TextureSlot::NormalMap] != nullptr)
        {
            outputStream << YAML::Key << "NormalMapPath" << YAML::Value << m_Textures[TextureSlot::NormalMap]->m_FilePath;
        }

        if (m_Textures[TextureSlot::OcclusionMap] != nullptr)
        {
            outputStream << YAML::Key << "OcclusionMapPath" << YAML::Value << m_Textures[TextureSlot::OcclusionMap]->m_FilePath;
        }

        outputStream << YAML::EndMap;
    }

    void Material::Deserialize(SerializationNode& inputNode)
    {
        m_BaseColor = inputNode["BaseColor"].as<XMFLOAT4>();
        m_Roughness = inputNode["Roughness"].as<float>();
        m_Metalness = inputNode["Metalness"].as<float>();

        if (inputNode["ColorMapPath"])
        {
            std::string filePath = inputNode["ColorMapPath"].as<std::string>();
            m_EngineContext->GetSubsystem<ResourceCache>()->LoadTexture(filePath, m_Textures[TextureSlot::BaseColorMap]);
        }

        if (inputNode["MetallicMapPath"])
        {
            std::string filePath = inputNode["MetallicMapPath"].as<std::string>();
            m_EngineContext->GetSubsystem<ResourceCache>()->LoadTexture(filePath, m_Textures[TextureSlot::MetalnessMap]);
        }

        if (inputNode["RoughnessMapPath"])
        {
            std::string filePath = inputNode["RoughnessMapPath"].as<std::string>();
            m_EngineContext->GetSubsystem<ResourceCache>()->LoadTexture(filePath, m_Textures[TextureSlot::RoughnessMap]);
        }

        if (inputNode["NormalMapPath"])
        {
            std::string filePath = inputNode["NormalMapPath"].as<std::string>();
            m_EngineContext->GetSubsystem<ResourceCache>()->LoadTexture(filePath, m_Textures[TextureSlot::NormalMap]);
        }

        if (inputNode["OcclusionMapPath"])
        {
            std::string filePath = inputNode["OcclusionMapPath"].as<std::string>();
            m_EngineContext->GetSubsystem<ResourceCache>()->LoadTexture(filePath, m_Textures[TextureSlot::OcclusionMap]);
        }
    }
}