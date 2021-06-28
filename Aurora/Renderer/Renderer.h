#pragma once
#include "EngineContext.h"
#include "ISubsystem.h"
#include "../Graphics/RHI_Implementation.h"
#include "../Graphics/RHI_GraphicsDevice.h"
#include "../Resource/ResourceCache.h"
#include "../Scene/Components/Camera.h"
#include "RendererEnums.h"

using namespace DirectX;

namespace Aurora
{
    // Ensure 16-byte alignment.
    struct CB_VertexShader
    {
        XMMATRIX m_MVP = XMMatrixIdentity();
        XMMATRIX m_WorldMatrix = XMMatrixIdentity();
    };

    struct CB_PixelLight
    {
        XMFLOAT3 m_AmbientLightColor;
        float m_AmbientLightStrength;
    };

    struct TransformComponent
    {
        XMMATRIX m_ModelMatrix = XMMatrixIdentity();
        XMMATRIX m_ProjectionMatrix = XMMatrixIdentity();
        XMMATRIX m_ViewMatrix = XMMatrixIdentity();
    };

    class Renderer : public ISubsystem
    {
    public:
        Renderer(EngineContext* engineContext);
        ~Renderer();
        
        bool Initialize() override;
        void Tick(float deltaTime) override;
        void DrawModel(); /// Pause.
        void Present();

        // Shenanigans
        void DrawDebugWorld(RHI_CommandList commandList);

    private:
        void CompileShaders();
        void CreateBuffers();
        void CreateRasterizerStates();
        void CreateDepth();
        void CreateTexture();
        

    public:
        bool m_DrawGridHelper = true;

        std::shared_ptr<DX11_GraphicsDevice> m_GraphicsDevice;

        RHI_SwapChain m_SwapChain;
        RHI_Shader m_VertexShader;
        RHI_Shader m_PixelShader;
        RHI_GPU_Buffer m_VertexBuffer;

        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_DepthStencilState;
        RHI_Texture m_DepthTexture;

        std::shared_ptr<AuroraResource> m_PyramidTexture = nullptr;
        RHI_Sampler m_Standard_Texture_Sampler;
        
        /// Entity Encapsulation
        TransformComponent m_Transform;
        CB_VertexShader transform;
        CB_PixelLight light;

        RHI_Mapping m_ConstantBufferMapping;
        RHI_GPU_Buffer m_ConstantBuffer_VertexTransform;

        RHI_GPU_Buffer m_ConstantBuffer_PixelLight;

        // Camera
        std::shared_ptr<Camera> m_Camera;

        /// Future Abstraction
        //========================================================== 
        ID3D11InputLayout* m_InputLayout = nullptr;
        UINT m_VertexStride = 0;
        UINT m_VertexOffset = 0;
        UINT m_VertexCount = 0;

        ID3D11RasterizerState* m_RasterizerState_Wireframe = nullptr;
    };
}