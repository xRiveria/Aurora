#include "Aurora.h"
#include "Material.h"
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

        if (m_Textures[TextureSlot::BaseColorMap].m_Resource != nullptr)
        {
            outputStream << YAML::Key << "ColorMapPath" << YAML::Value << m_Textures[TextureSlot::BaseColorMap].m_FilePath;
        }

        if (m_Textures[TextureSlot::MetalnessMap].m_Resource != nullptr)
        {
            outputStream << YAML::Key << "MetallicMapPath" << YAML::Value << m_Textures[TextureSlot::MetalnessMap].m_FilePath;
        }

        if (m_Textures[TextureSlot::MetalnessMap].m_Resource != nullptr)
        {
            outputStream << YAML::Key << "RoughnessMapPath" << YAML::Value << m_Textures[TextureSlot::RoughnessMap].m_FilePath;
        }

        if (m_Textures[TextureSlot::NormalMap].m_Resource != nullptr)
        {
            outputStream << YAML::Key << "NormalMapPath" << YAML::Value << m_Textures[TextureSlot::NormalMap].m_FilePath;
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
            m_Textures[TextureSlot::BaseColorMap].m_Resource = m_EngineContext->GetSubsystem<ResourceCache>()->LoadTexture(filePath, FileSystem::GetFileNameFromFilePath(filePath));
            m_Textures[TextureSlot::BaseColorMap].m_FilePath = filePath;
        }

        if (inputNode["MetallicMapPath"])
        {
            std::string filePath = inputNode["MetallicMapPath"].as<std::string>();
            m_Textures[TextureSlot::MetalnessMap].m_Resource = m_EngineContext->GetSubsystem<ResourceCache>()->LoadTexture(filePath, FileSystem::GetFileNameFromFilePath(filePath));
            m_Textures[TextureSlot::MetalnessMap].m_FilePath = filePath;
        }

        if (inputNode["RoughnessMapPath"])
        {
            std::string filePath = inputNode["RoughnessMapPath"].as<std::string>();
            m_Textures[TextureSlot::RoughnessMap].m_Resource = m_EngineContext->GetSubsystem<ResourceCache>()->LoadTexture(filePath, FileSystem::GetFileNameFromFilePath(filePath));
            m_Textures[TextureSlot::RoughnessMap].m_FilePath = filePath;
        }

        if (inputNode["NormalMapPath"])
        {
            std::string filePath = inputNode["NormalMapPath"].as<std::string>();
            m_Textures[TextureSlot::NormalMap].m_Resource = m_EngineContext->GetSubsystem<ResourceCache>()->LoadTexture(filePath, FileSystem::GetFileNameFromFilePath(filePath));
            m_Textures[TextureSlot::NormalMap].m_FilePath = filePath;
        }
    }
}