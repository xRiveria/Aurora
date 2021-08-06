#pragma once
#include <vector>
#include "../Resource/ResourceUtilities.h"
#include "../Graphics/RHI_Implementation.h"
#include "../Scene/Entity.h"

namespace Aurora
{
    enum class EnvironmentType
    {
        EnvironmentType_Cubemap,
        EnvironmentType_Equirectangular
    };

    class EngineContext;
    class Renderer;

    class Environment
    {
    public:
        Environment(EngineContext* engineContext);
        void CreateRenderResources();

        void Render();

    public:
        // Skybox
        ID3D11Texture2D* m_SkyIBLTexture = nullptr;
        ID3D11ShaderResourceView* m_SkySRVTexture = nullptr;

        ID3D11ShaderResourceView* m_SkySRV;
        ID3D11RasterizerState* m_SkyRasterizerState;
        ID3D11DepthStencilState* m_SkyDepthState;
        std::shared_ptr<Entity> m_SkyboxEntity;




        ID3D11ShaderResourceView* m_ShaderResourceView;
        RHI_Shader m_CubemapShaderVS;
        RHI_Shader m_CubemapShaderPS;

        std::vector<std::shared_ptr<AuroraResource>> m_CubemapFaces;

        int m_SkyboxCubemapMappingIndex = TEXSLOT_RENDERER_SKYCUBE_MAP;
        Renderer* m_Renderer;
        EngineContext* m_EngineContext;

    };
}