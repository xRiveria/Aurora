#pragma once
#include "EngineContext.h"
#include "../Graphics/RHI_Implementation.h"
#include "../Graphics/RHI_GraphicsDevice.h"
#include "../Resource/ResourceCache.h"
#include "../Scene/Components/Camera.h"
#include "RendererEnums.h"
#include "ShaderCompiler.h"
#include "../Resource/Importers/Importer_Model.h"
#include "../Scene/Components/Material.h"
#include "../Scene/Components/Mesh.h"

using namespace DirectX;

namespace Aurora
{
    class Skybox;

    class Renderer : public ISubsystem
    {
    public:
        Renderer(EngineContext* engineContext);
        ~Renderer();
        
        bool Initialize() override;
        void Tick(float deltaTime) override;
        void RenderScene();
        void DrawDebugWorld(Entity* entity);

        void Present();
        void CreateTexture();

        // Shenanigans

    public:
        // Bindings
        void BindConstantBuffers(Shader_Stage shaderStage, RHI_CommandList commandList);

        void UpdateCameraConstantBuffer(const std::shared_ptr<Entity>& camera, RHI_CommandList commandList);
        void UpdateLightConstantBuffer();
        void UpdateMaterialConstantBuffer(Material* materialComponent);
        int BindMaterialTexture(TextureSlot slotType, Material* material);
        int BindSkyboxTexture(int slotNumber, RHI_Texture* texture);
        int BindSkyboxTexture(int slotNumber, ID3D11ShaderResourceView* shaderResourceView);

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
        void LoadSkyPipelineState(RHI_Shader* vertexShader, RHI_Shader* pixelShader);

        void LoadPipelineStates();
        void SetRenderDimensions(float width, float height) { m_RenderWidth = width; m_RenderHeight = height; }

    private:
        ShaderCompiler::ShaderCompiler m_ShaderCompiler;

    public:
        float m_RenderWidth = 1280;
        float m_RenderHeight = 1080;
        std::shared_ptr<Skybox> m_Skybox;
        RHI_PipelineState m_PSO_Object_Sky;

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

        // Entities
        std::vector<std::shared_ptr<Entity>> m_SceneEntities;
    };
}