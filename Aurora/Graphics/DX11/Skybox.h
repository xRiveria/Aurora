#pragma once
#include "EngineContext.h"
#include "../RHI_Implementation.h"
#include "../Renderer/Renderer.h"
#include "Cubemap.h"

namespace Aurora
{
    class Skybox
    {
    public:
        Skybox(EngineContext* engineContext);
        ~Skybox();

        bool InitializeResources();
        bool Render() const;
        bool RenderForResources() const;

    public:
        bool CreateCubeMap();
        void BindMesh() const;

        std::shared_ptr<Entity> m_SkyboxEntity;

        RHI_Shader m_SkyVertexShader;
        RHI_Shader m_SkyPixelShader;

        RHI_Shader m_RectToCubemapVSShader;
        RHI_Shader m_RectToCubemapPSShader;

        int m_SkyboxCubemapMappingIndex = TEXSLOT_RENDERER_SKYCUBE_MAP;
        int m_SkyboxHDRMappingIndex = TEXSLOT_RENDERER_SKYHDR_MAP;

        std::shared_ptr<AuroraResource> m_SkyHDR = nullptr;
        Cubemap* m_Cubemap = nullptr;

        EngineContext* m_EngineContext;
        Renderer* m_Renderer;
    };
}