#include "Aurora.h"
#include "Material.h"
#include "../Resource/ResourceCache.h"
#include "../Renderer/Renderer.h"
#include "../Graphics/DX11_Refactored/DX11_Texture.h"

namespace Aurora
{
    Material::Material(EngineContext* engineContext) : AuroraResource(engineContext, ResourceType::ResourceType_Material)
    {
        // Initialize Properties
        SetProperty(MaterialSlot::MaterialSlot_Roughness, 0.9f);
        SetProperty(MaterialSlot::MaterialSlot_Metallic, 0.0f);
        SetProperty(MaterialSlot::MaterialSlot_Normal, 0.0f);
        SetProperty(MaterialSlot::MaterialSlot_Occlusion, 0.0f);
    }

    bool Material::SaveToFile(const std::string& filePath)
    {
        std::unique_ptr<FileSerializer> fileSerializer = std::make_unique<FileSerializer>(m_EngineContext);
        if (fileSerializer->BeginSerialization("Material"))
        {
            fileSerializer->AddProperty("Albedo_Color", m_AlbedoColor);
            fileSerializer->AddProperty("Roughness_Multiplier", GetProperty(MaterialSlot::MaterialSlot_Roughness));
            fileSerializer->AddProperty("Normal_Multiplier", GetProperty(MaterialSlot::MaterialSlot_Normal));
            fileSerializer->AddProperty("Metallic_Multiplier", GetProperty(MaterialSlot::MaterialSlot_Metallic));
            fileSerializer->AddProperty("Occlusion_Multiplier", GetProperty(MaterialSlot::MaterialSlot_Occlusion));

            fileSerializer->AddMapKey("Textures");
            fileSerializer->AddProperty("Textures_Count", static_cast<uint32_t>(m_Textures.size()));

            uint32_t i = 0;
            for (const auto& texture : m_Textures)
            {
                std::string keyName = "Texture_" + std::to_string(i);
                fileSerializer->AddMapKey(keyName);

                fileSerializer->AddProperty("Texture_Type", static_cast<uint32_t>(texture.first));
                fileSerializer->AddProperty("Texture_Name", texture.second ? texture.second->GetResourceName() : "");
                fileSerializer->AddProperty("Texture_Path", texture.second ? texture.second->GetResourceFilePathNative() : "");
     
                fileSerializer->EndMapKey();
                i++;
            }

            fileSerializer->EndMapKey();
        }

        return fileSerializer->EndSerialization(GetResourceFilePathNative());
    }

    bool Material::LoadFromFile(const std::string& filePath)
    {
        std::unique_ptr<FileSerializer> fileSerializer = std::make_unique<FileSerializer>(m_EngineContext);

        if (fileSerializer->LoadFromFile(filePath))
        {
            SetResourceFilePath(filePath);

            if (fileSerializer->ValidateFileType("Material"))
            {
                fileSerializer->GetProperty("Albedo_Color", &m_AlbedoColor);
                fileSerializer->GetProperty("Roughness_Multiplier", &GetProperty(MaterialSlot::MaterialSlot_Roughness));
                fileSerializer->GetProperty("Normal_Multiplier", &GetProperty(MaterialSlot::MaterialSlot_Normal));
                fileSerializer->GetProperty("Metallic_Multiplier", &GetProperty(MaterialSlot::MaterialSlot_Metallic));
                fileSerializer->GetProperty("Occlusion_Multiplier", &GetProperty(MaterialSlot::MaterialSlot_Occlusion));
            }

            if (fileSerializer->ValidateKey("Textures"))
            {
                uint32_t textureCount = 0;
                std::string materialName;
                std::string materialPath;
                uint32_t materialSlot = 0;
                
                fileSerializer->GetPropertyFromSubNode("Textures", "Textures_Count", &textureCount);

                for (int i = 0; i < textureCount; i++)
                {
                    std::string keyName = "Texture_" + std::to_string(i);
                    fileSerializer->GetPropertyFromSubNode("Textures", keyName, "Texture_Type", &materialSlot);
                    const MaterialSlot slot = static_cast<MaterialSlot>(materialSlot);
                    fileSerializer->GetPropertyFromSubNode("Textures", keyName, "Texture_Name", &materialName);
                    fileSerializer->GetPropertyFromSubNode("Textures", keyName, "Texture_Path", &materialPath);
                    
                    if (materialPath != "")
                    {
                        // If the texture happens to be loaded, get a reference to it.
                        std::shared_ptr<DX11_Texture> texture = m_EngineContext->GetSubsystem<ResourceCache>()->GetResourceByName<DX11_Texture>(materialName);
                        // If there is no texture (it's not loaded yet), load it.

                        if (!texture)
                        {
                            m_EngineContext->GetSubsystem<Threading>()->Execute([this, texture, materialPath, slot](JobInformation jobInformation) mutable
                            {
                                texture = m_EngineContext->GetSubsystem<ResourceCache>()->Load<DX11_Texture>(materialPath);
                                SetTextureSlot(slot, texture, GetProperty(slot));
                            });
                        }
                        else
                        {
                            SetTextureSlot(slot, texture, GetProperty(slot));
                        }
                    }
                }
            }
        }

        return true;
    }

    void Material::SetTextureSlot(const MaterialSlot materialSlot, const std::shared_ptr<DX11_Texture>& texture, float parameterMultiplier)
    {
        if (texture)
        {
            // In order for the material to guarentee serialization/deserialization, we cache the texture.
            const std::shared_ptr<DX11_Texture> textureCached = m_EngineContext->GetSubsystem<ResourceCache>()->CacheResource(texture);
            m_Textures[materialSlot] = textureCached != nullptr ? textureCached : texture; // If some oddity happens with caching, we will simply use the raw texture as it is.
            m_MaterialFlags |= materialSlot;

            SetProperty(materialSlot, parameterMultiplier);
        }
        else
        {
            m_Textures.erase(materialSlot);
            m_MaterialFlags &= ~materialSlot;
        }

        // Serialize our changes.
       // m_EngineContext->GetSubsystem<Threading>()->Execute([this](JobInformation jobInformation)
       // {
            SaveToFile(GetResourceFilePathNative());
       // });
    }

    void Material::SetAlbedoColor(const XMFLOAT4& albedoColor)
    {
        /// If an object switches from opaque to transparent or vice versa, make the world resolve so the renderer goes through the entities and make the ones that use this material render in the correct mode.
        m_AlbedoColor = albedoColor;
    }
}