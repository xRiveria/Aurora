#pragma once
#include "EngineContext.h"
#include "ISubsystem.h"
#include "../Graphics/RHI_Implementation.h"
#include "../Graphics/RHI_GraphicsDevice.h"
#include "../Resource/ResourceCache.h"

using namespace DirectX;

namespace Aurora
{
    struct ConstantBuffer_PerObject
    {
        XMMATRIX m_WVP;
        XMVECTOR m_Color;
    };

    class Renderer : public ISubsystem
    {
    public:
        Renderer(EngineContext* engineContext);
        ~Renderer();
        
        bool Initialize() override;
        void Tick(float deltaTime) override;

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
        RHI_Texture m_DepthTexture;

        std::shared_ptr<AuroraResource> m_PyramidTexture = nullptr;
        RHI_Sampler m_Standard_Texture_Sampler;

        /// Future Abstraction
        //========================================================== 
        ID3D11InputLayout* m_InputLayout = nullptr;
        UINT m_VertexStride = 0;
        UINT m_VertexOffset = 0;
        UINT m_VertexCount = 0;

        ID3D11RasterizerState* m_RasterizerState_Wireframe = nullptr;
    };
}