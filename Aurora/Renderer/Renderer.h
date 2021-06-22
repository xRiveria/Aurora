#pragma once
#include "EngineContext.h"
#include "ISubsystem.h"
// Our Graphics API
#include <d3d11.h>       // Link against the Win32 Library
#include <dxgi.h>        // DirectX Graphics Interface
#include <d3dcompiler.h> // Shader Compiler

namespace Aurora
{
    class Renderer : public ISubsystem
    {
    public:
        Renderer(EngineContext* engineContext);
        
        bool Initialize() override;
        void Tick(float deltaTime) override;

    private:
        void CompileShaders();
        void CreateBuffers();

    private:
        ID3D11Device* m_Device = nullptr;
        ID3D11DeviceContext* m_DeviceContext = nullptr;
        IDXGISwapChain* m_SwapChain = nullptr;
        ID3D11RenderTargetView* m_RenderTargetView = nullptr;
        ID3D11VertexShader* m_VertexShader = nullptr;
        ID3D11InputLayout* m_InputLayout = nullptr;
        UINT m_VertexStride = 0;
        UINT m_VertexOffset = 0;
        UINT m_VertexCount = 0;
        ID3D11PixelShader* m_PixelShader = nullptr;

        ID3D11Buffer* m_VertexBuffer = nullptr;
        ID3D11Buffer* m_PixelBuffer = nullptr;
    };
}