#pragma once
#include "EngineContext.h"
#include "ISubsystem.h"
#include "../Graphics/RHI_Implementation.h"
#include "../Graphics/RHI_GraphicsDevice.h"
#include "../Resource/ResourceCache.h"
#include "../Scene/Components/Camera.h"

using namespace DirectX;

namespace Aurora
{
    // Ensure 16-byte alignment.
    struct CB_VertexShader
    {
        XMMATRIX m_MVP = XMMatrixIdentity();
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

    private:
        void CompileShaders();
        void CreateBuffers();
        void CreateRasterizerStates();
        void CreateDepth();
        void CreateTexture();

    public:
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
        RHI_Mapping m_ConstantBufferMapping;
        RHI_GPU_Buffer m_ConstantBuffer_VertexTransform;

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