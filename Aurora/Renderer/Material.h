#pragma once
#include "../Resource/AuroraResource.h"

namespace Aurora
{
    // These material properties correspond to our shader binding points.
    enum MaterialSlot : uint32_t
    {
        MaterialSlot_Unknown =   0 << 0,
        MaterialSlot_Albedo =    1 << 0,
        MaterialSlot_Normal  =   1 << 1,
        MaterialSlot_Metallic =  1 << 2,
        MaterialSlot_Roughness = 1 << 3,
        MaterialSlot_Occlusion = 1 << 4
    };

    class Material : public AuroraResource
    {
    public:
        Material(EngineContext* engineContext);

        bool SaveToFile(const std::string& filePath) override;
        bool LoadFromFile(const std::string& filePath) override;

        // Textures
        void SetTextureSlot(const MaterialSlot materialSlot, const std::shared_ptr<DX11_Texture>& texture, float parameterMultiplier = 1.0f);

        // Parameters
        void SetAlbedoColor(const XMFLOAT4& albedoColor);
        XMFLOAT4 GetAlbedoColor() const { return m_AlbedoColor; }
        float& GetProperty(const MaterialSlot materialSlot) { return m_Properties[materialSlot]; }
        void SetProperty(const MaterialSlot materialSlot, const float value) { m_Properties[materialSlot] = value; }

    public:
        XMFLOAT4 m_AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

        std::unordered_map<MaterialSlot, std::shared_ptr<DX11_Texture>> m_Textures;
        std::unordered_map<MaterialSlot, float> m_Properties;
        uint32_t m_MaterialFlags = 0;
    };
}