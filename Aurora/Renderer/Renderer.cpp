#include "Aurora.h"
#include "Renderer.h"
#include "../Window/WindowContext.h"
#include "../Graphics/DX11/DX11_Utilities.h" //Temporary
#include "../Scene/World.h"
#include "ShaderInternals.h"
#include "RendererResources.h"

namespace Aurora
{
    Renderer::Renderer(EngineContext* engineContext) : ISubsystem(engineContext)
    {

    }

    Renderer::~Renderer()
    {

    }

    bool Renderer::Initialize()
    {
        m_GraphicsDevice = std::make_shared<DX11_GraphicsDevice>(m_EngineContext, true);
        m_ShaderCompiler.Initialize();
        LoadShaders();
        LoadStates();
        LoadBuffers();
        LoadPipelineStates();

        RHI_SwapChain_Description swapchainDescription;
        swapchainDescription.m_Width = static_cast<uint32_t>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowWidth(0));
        swapchainDescription.m_Height = static_cast<uint32_t>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowHeight(0));

        m_GraphicsDevice->CreateSwapChain(&swapchainDescription, &m_SwapChain);

        CreateDepth();
        CreateTexture();

        m_Camera = std::make_shared<Camera>(m_EngineContext);
        m_Camera->SetPosition(0.0f, 0.0f, -10.0f);
        m_Camera->ComputePerspectiveMatrix(90.0f, static_cast<float>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowWidth(0)) / static_cast<float>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowHeight(0)), 0.1f, 1000.0f);

        m_EngineContext->GetSubsystem<World>()->LoadModel("../Resources/Models/Hollow_Knight/v3.obj");
        m_GraphicsDevice->BindPipelineState(&RendererGlobals::m_PSO_Object_Wire, 0);

        // For scissor rects in our rasterizer set.
        D3D11_RECT pRects[8];
        for (uint32_t i = 0; i < 8; ++i)
        {
            pRects[i].bottom = INT32_MAX;
            pRects[i].left = INT32_MIN;
            pRects[i].right = INT32_MAX;
            pRects[i].top = INT32_MIN;
        }
        m_GraphicsDevice->m_DeviceContextImmediate->RSSetScissorRects(8, pRects);

        return true;
    }

    void Renderer::UpdateCameraConstantBuffer(const Camera& camera, RHI_CommandList commandList)
    {
        ConstantBufferData_Camera constantBuffer;

        XMStoreFloat4x4(&constantBuffer.g_Camera_ViewProjection, camera.GetViewProjectionMatrix());
        XMStoreFloat4x4(&constantBuffer.g_Camera_View, camera.GetViewMatrix());
        XMStoreFloat4x4(&constantBuffer.g_Camera_Projection, camera.GetProjectionMatrix());
        XMStoreFloat3(&constantBuffer.g_Camera_Position, camera.GetPosition());

        m_GraphicsDevice->UpdateBuffer(&RendererGlobals::g_ConstantBuffers[CB_Types::CB_Camera], &constantBuffer, commandList);
    }

    void Renderer::BindConstantBuffers(Shader_Stage shaderStage, RHI_CommandList commandList)
    {
        m_GraphicsDevice->BindConstantBuffer(shaderStage, &RendererGlobals::g_ConstantBuffers[CB_Types::CB_Camera], CB_GETBINDSLOT(ConstantBufferData_Camera), commandList);
    }

    void Renderer::Tick(float deltaTime)
    {
        UpdateCameraConstantBuffer(*m_Camera.get(), 0);
        BindConstantBuffers(Shader_Stage::Vertex_Shader, 0);
        BindConstantBuffers(Shader_Stage::Pixel_Shader, 0);

        auto internalState = DX11_Utility::ToInternal(&m_SwapChain);

        // Clear the backbuffer to black for the new frame.
        float backgroundColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

        ID3D11RenderTargetView* RTV = internalState->m_RenderTargetView.Get();
        ID3D11DepthStencilView* DSV = DX11_Utility::ToInternal(&m_DepthTexture)->m_DepthStencilView.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->ClearRenderTargetView(RTV, backgroundColor); // We use the render target view pointer to access the back buffer and clear it to an RGBA color of our choice (values between 0 and 1). If we add a depth buffer later, we will need to clear it as well.
        m_GraphicsDevice->m_DeviceContextImmediate->ClearDepthStencilView(DSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        // Viewport
        D3D11_VIEWPORT viewportInfo = { 0, 0, m_EngineContext->GetSubsystem<WindowContext>()->GetWindowWidth(0), m_EngineContext->GetSubsystem<WindowContext>()->GetWindowHeight(0), 0.0f, 1.0f };
        m_GraphicsDevice->m_DeviceContextImmediate->RSSetViewports(1, &viewportInfo);
        m_GraphicsDevice->m_DeviceContextImmediate->OMSetRenderTargets(1, &RTV, DSV); // Set depth as well here if it exists.

        ID3D11SamplerState* samplerState = DX11_Utility::ToInternal(&m_Standard_Texture_Sampler)->m_Resource.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->PSSetSamplers(0, 1, &samplerState);

        DrawModel();

        Present();
        m_Camera->Tick(deltaTime);
    }

    void Renderer::DrawModel()
    {
        World* world = m_EngineContext->GetSubsystem<World>();
        MeshComponent meshComponent = world->m_MeshComponent;

        UINT offset = 0;
        UINT modelStride = 8 * sizeof(float);

        ID3D11ShaderResourceView* shaderResourceView = DX11_Utility::ToInternal(&meshComponent.m_BaseTexture->m_Texture)->m_ShaderResourceView.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->PSSetShaderResources(0, 1, &shaderResourceView);

        ID3D11Buffer* vertexBuffer = (ID3D11Buffer*)DX11_Utility::ToInternal(&meshComponent.m_VertexBuffer_Position)->m_Resource.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->IASetVertexBuffers(0, 1, &vertexBuffer, &modelStride, &offset);

        m_GraphicsDevice->m_DeviceContextImmediate->Draw((UINT)meshComponent.m_VertexPositions.size(), 0);
    }

    void Renderer::Present()
    {
        auto internalState = DX11_Utility::ToInternal(&m_SwapChain);
        internalState->m_SwapChain->Present(1, 0);
    }

    void Renderer::CreateDepth()
    {
        RHI_Texture_Description depthStencilDescription;
        depthStencilDescription.m_Type = Texture_Type::Texture2D;
        depthStencilDescription.m_Width = static_cast<uint32_t>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowWidth(0));
        depthStencilDescription.m_Height = static_cast<uint32_t>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowHeight(0));
        depthStencilDescription.m_MipLevels = 1;
        depthStencilDescription.m_ArraySize = 1;
        depthStencilDescription.m_Format = Format::FORMAT_D24_UNORM_S8_UINT;
        depthStencilDescription.m_SampleCount = 1;
        depthStencilDescription.m_Usage = Usage::Default;
        depthStencilDescription.m_BindFlags = Bind_Flag::Bind_Depth_Stencil;
        depthStencilDescription.m_CPUAccessFlags = 0;
        depthStencilDescription.m_MiscFlags = 0;

        m_GraphicsDevice->CreateTexture(&depthStencilDescription, nullptr, &m_DepthTexture);
    }

    void Renderer::CreateTexture()
    {
        RHI_Sampler_Description samplerDescription;
        samplerDescription.m_Filter = Filter::FILTER_MIN_MAG_MIP_LINEAR;
        samplerDescription.m_AddressU = Texture_Address_Mode::Texture_Address_Wrap;
        samplerDescription.m_AddressV = Texture_Address_Mode::Texture_Address_Wrap;
        samplerDescription.m_AddressW = Texture_Address_Mode::Texture_Address_Wrap;
        samplerDescription.m_ComparisonFunction = ComparisonFunction::Comparison_Never;
        samplerDescription.m_MinLOD = 0;
        samplerDescription.m_MaxLOD = D3D11_FLOAT32_MAX;

        m_GraphicsDevice->CreateSampler(&samplerDescription, &m_Standard_Texture_Sampler);
    }
}