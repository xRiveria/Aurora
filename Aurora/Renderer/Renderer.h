#pragma once
#include "EngineContext.h"
#include "ISubsystem.h"
#include "../Graphics/RHI_Implementation.h"
#include "../Graphics/RHI_GraphicsDevice.h"

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

    private:
        /// New Abstraction
        std::shared_ptr<DX11_GraphicsDevice> m_GraphicsDevice;

        // Swapchain
        SwapChain m_SwapChain;





        // ID3D11Device* m_Device = nullptr; // Device is used to create resources and enumerate the capabilities of a display adapter. Each application must have one device.
        // ID3D11DeviceContext* m_DeviceContext = nullptr; // Device context contains the circumstance or setting in which a device is used. It is used to set the pipeline state and generate rendering commands using resources owned by the device. 2 types of contexts exist, one of immediate rendering and the other for deferred rendering.
        
        // IDXGISwapChain* m_SwapChain = nullptr;

        // ID3D11RenderTargetView* m_RenderTargetView = nullptr;
        ID3D11DepthStencilView* m_DepthStencilView = nullptr;
        ID3D11Texture2D* m_DepthStencilBuffer = nullptr;

        ID3D11InputLayout* m_InputLayout = nullptr;
        UINT m_VertexStride = 0;
        UINT m_VertexOffset = 0;
        UINT m_VertexCount = 0;
        ID3D11Buffer* m_VertexBuffer = nullptr;
        ID3D11Buffer* m_PixelBuffer = nullptr;
        ID3D11VertexShader* m_VertexShader = nullptr;
        ID3D11PixelShader* m_PixelShader = nullptr;

        ID3D11RasterizerState* m_RasterizerState_Wireframe = nullptr;
    };
}