#pragma once
#include "EngineContext.h"
#include "../Resource/AuroraResource.h"
#include "../Graphics/RHI_Implementation.h"
#include "../Graphics/RHI_GraphicsDevice.h"
#include "../Scene/Components/Camera.h"
#include "RendererEnums.h"
#include "ShaderCompiler.h"
#include "../_Shaders/ShaderUtilities.h"
#include "../Resource/Importers/Importer_Model.h"
#include "../Scene/Components/Material.h"
#include "../Scene/Components/Mesh.h"
#include "../Graphics/DX11_Refactored/DX11_Context.h"
#include "../Graphics/DX11_Refactored/DX11_Texture.h"

using namespace DirectX;

namespace Aurora
{
    class Skybox;
    class ResourceCache;

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

    public:
        void BindConstantBuffers(RHI_Shader_Stage shaderStage, RHI_CommandList commandList);

        void UpdateEntityConstantBuffer(Entity* entity);
        void UpdateCameraConstantBuffer(const std::shared_ptr<Entity>& camera, RHI_CommandList commandList);
        void UpdateLightConstantBuffer();
        void UpdateMaterialConstantBuffer(Material* materialComponent);
        int BindMaterialTexture(TextureSlot slotType, int slotIndex, Material* material);
        int BindSkyboxTexture(int slotNumber, RHI_Texture* texture);
        int BindSkyboxTexture(int slotNumber, ID3D11ShaderResourceView* shaderResourceView);

    public:
        /// New Abstraction
        void ResizeBuffers();
        // Depth Buffer

        std::shared_ptr<Entity> m_DirectionalLight;


        void LoadStates();
        void LoadBuffers();
        void LoadShaders();
        bool LoadShader(RHI_Shader_Stage shaderStage, RHI_Shader& shader, const std::string& fileName, Shader_Model minimumShaderModel = Shader_Model::ShaderModel_5_0);
        void LoadDefaultTextures();

        void LoadPipelineStates();
        void SetRenderDimensions(float width, float height) { m_RenderWidth = width; m_RenderHeight = height; }

    private:
        ShaderCompiler::ShaderCompiler m_ShaderCompiler;

    public:
        float m_RenderWidth = 1280;
        float m_RenderHeight = 1080;
        float m_LightBias = 0.055f;

    public:
        RHI_GPU_Buffer        g_ConstantBuffers[CB_Types::CB_Count];
        std::shared_ptr<DX11_Context> m_DeviceContext;

        std::shared_ptr<Skybox> m_Skybox;
        std::shared_ptr<DX11_GraphicsDevice> m_GraphicsDevice;

        RHI_SwapChain m_SwapChain;
        RHI_Shader m_VertexShader;
        RHI_Shader m_PixelShader;
        RHI_Shader m_SimpleDepthShaderVS;
        RHI_Shader m_SimpleDepthShaderPS;
        RHI_Shader m_BloomVS;
        RHI_Shader m_BloomPS;

        int m_DepthShadowMappingIndex = 36;
        int m_BaseMap = TEXSLOT_RENDERER_BASECOLOR_MAP;
        int m_NormalMapIndex = TEXSLOT_RENDERER_NORMAL_MAP;
        int m_MetalMapIndex = TEXSLOT_RENDERER_METALNESS_MAP;
        int m_RoughnessMapIndex = TEXSLOT_RENDERER_ROUGHNESS_MAP;

        RHI_Sampler m_Standard_Texture_Sampler;
        RHI_Sampler m_Depth_Texture_Sampler;

        // Camera
        std::shared_ptr<Entity> m_Camera;

        // Default Textures
        ResourceCache* m_ResourceCache;
        std::shared_ptr<AuroraResource> m_DefaultWhiteTexture;

        // Entities
        std::vector<std::shared_ptr<Entity>> m_SceneEntities;

        RHI_PipelineState m_PSO_Object_Wire; // Right now we're using this for everything.
    };
}