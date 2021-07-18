#pragma once
#include "EngineContext.h"
#include "../Graphics/RHI_Implementation.h"
#include "../Graphics/RHI_GraphicsDevice.h"
#include "../Resource/ResourceCache.h"
#include "../Scene/Components/Camera.h"
#include "RendererEnums.h"
#include "ShaderCompiler.h"
#include "../Resource/Importers/Importer_Model.h"

using namespace DirectX;

namespace Aurora
{
    /// Makeshift
    class CubeImage
    {
    public:
        CubeImage() {};
        ~CubeImage() {};

        int BytesPerPixel() const { return m_Channels * sizeof(unsigned char); }
        int Pitch() const { return m_Width * BytesPerPixel(); }
       
        int m_Width;
        int m_Height;
        int m_Channels = 4; // Default
        std::unique_ptr<unsigned char> m_Pixels;
    };

    class Renderer : public ISubsystem
    {
    public:
        Renderer(EngineContext* engineContext);
        ~Renderer();
        
        bool Initialize() override;
        void Tick(float deltaTime) override;
        void RenderScene();
        void DrawSkybox();
        void DrawDebugWorld(Entity* entity);

        // Texture Cube
        void PrepareSkyboxResources();
        ComPtr<ID3D11ShaderResourceView> m_CubeSRV;
        std::vector<std::shared_ptr<CubeImage>> m_CubemapTextures;

        RHI_Shader m_SkyboxVS;
        RHI_Shader m_SkyboxPS;

        void Present();

        // Shenanigans

    public:
        void CreateTexture();

    public:
        /// New Abstraction
        void ResizeBuffers();
        // Depth Buffer
        RHI_Texture m_RenderTarget_GBuffer[GBuffer_Types::GBuffer_Count];

        RHI_Texture m_DepthBuffer_Main; // Used for depth-testing. Can be used for MSAA.
        RHI_RenderPass m_RenderPass_DepthPrePass;
        RHI_RenderPass m_RenderPass_Main;
        uint32_t m_MSAA_SampleCount = 1;
        const uint32_t GetMSAASampleCount() const { return m_MSAA_SampleCount; }

        void LoadStates();
        void LoadBuffers();
        void LoadShaders();
        bool LoadShader(Shader_Stage shaderStage, RHI_Shader& shader, const std::string& fileName, Shader_Model minimumShaderModel = Shader_Model::ShaderModel_5_0);
        void LoadDefaultTextures();

        void LoadPipelineStates();
        // void ReloadShaders();  // Fire shader reload event. Calls LoadShaders() again.

        // const RHI_GPU_Buffer* GetConstantBuffer(CB_Types bufferType) { return &RendererGlobals::g_ConstantBuffers[bufferType]; }
        void UpdateCameraConstantBuffer(const std::shared_ptr<Entity>& camera, RHI_CommandList commandList);
        void BindConstantBuffers(Shader_Stage shaderStage, RHI_CommandList commandList);

    private:
        ShaderCompiler::ShaderCompiler m_ShaderCompiler;
        
    public:
        std::shared_ptr<DX11_GraphicsDevice> m_GraphicsDevice;

        RHI_SwapChain m_SwapChain;
        RHI_Shader m_VertexShader;
        RHI_Shader m_PixelShader;
        RHI_GPU_Buffer m_VertexBuffer;

        RHI_Sampler m_Standard_Texture_Sampler;
        
        // Camera
        std::shared_ptr<Entity> m_Camera;

        // Default Textures
        ResourceCache* m_ResourceCache;
        std::shared_ptr<AuroraResource> m_DefaultWhiteTexture;

        // Lock
        std::vector<std::pair<std::shared_ptr<AuroraResource>, bool>> m_DeferredMipGenerations;
    };
}