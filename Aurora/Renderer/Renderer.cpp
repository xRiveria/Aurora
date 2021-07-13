#include "Aurora.h"
#include "Renderer.h"
#include "../Window/WindowContext.h"
#include "../Graphics/DX11/DX11_Utilities.h" //Temporary
#include "ShaderInternals.h"
#include "RendererResources.h"
#include "../Scene/Components/Light.h"
#include "../Scene/Components/Mesh.h"

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
        m_Importer_Model = std::make_shared<Importer_Model>(m_EngineContext);
        m_ShaderCompiler.Initialize();
        LoadShaders();
        LoadStates();
        LoadBuffers();
        LoadPipelineStates();
        ResizeBuffers();

        RHI_SwapChain_Description swapchainDescription;
        swapchainDescription.m_Width = static_cast<uint32_t>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowWidth(0));
        swapchainDescription.m_Height = static_cast<uint32_t>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowHeight(0));

        m_GraphicsDevice->CreateSwapChain(&swapchainDescription, &m_SwapChain);

        CreateTexture();

        m_Camera = m_EngineContext->GetSubsystem<World>()->GetEntityByName("Default_Camera");
        m_Camera->GetComponent<Camera>()->SetPosition(3.0f, 3.0f, -10.0f);
        m_Camera->GetComponent<Camera>()->ComputePerspectiveMatrix(90.0f, static_cast<float>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowWidth(0)) / static_cast<float>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowHeight(0)), 0.1f, 1000.0f);

        m_Importer_Model->Load("../Resources/Models/Hollow_Knight/v3.obj", "../Resources/Models/Hollow_Knight/textures/None_2_Base_Color.png");
        m_Importer_Model->Load("../Resources/Models/Sword/weapon1.obj", "../Resources/Models/Sword/TextureWeapon1.png");

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

    void Renderer::UpdateCameraConstantBuffer(const std::shared_ptr<Entity>& camera, RHI_CommandList commandList)
    {
        ConstantBufferData_Camera constantBuffer;

        XMStoreFloat4x4(&constantBuffer.g_Camera_ViewProjection, camera->GetComponent<Camera>()->GetViewProjectionMatrix());
        XMStoreFloat4x4(&constantBuffer.g_Camera_View, camera->GetComponent<Camera>()->GetViewMatrix());
        XMStoreFloat4x4(&constantBuffer.g_Camera_Projection, camera->GetComponent<Camera>()->GetProjectionMatrix());
        XMStoreFloat4x4(&constantBuffer.g_Camera_InverseViewProjection, XMMatrixInverse(nullptr, camera->GetComponent<Camera>()->GetViewProjectionMatrix()));
        XMStoreFloat3(&constantBuffer.g_Camera_Position, camera->GetComponent<Camera>()->GetPosition());

        m_GraphicsDevice->UpdateBuffer(&RendererGlobals::g_ConstantBuffers[CB_Types::CB_Camera], &constantBuffer, commandList);
    }

    void Renderer::BindConstantBuffers(Shader_Stage shaderStage, RHI_CommandList commandList)
    {
        m_GraphicsDevice->BindConstantBuffer(shaderStage, &RendererGlobals::g_ConstantBuffers[CB_Types::CB_Camera], CB_GETBINDSLOT(ConstantBufferData_Camera), commandList);
    }

    inline void Renderer::ResizeBuffers()
    {
        float resolutionScale = 1.0f;
        XMUINT2 internalResolution = XMUINT2(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowWidth(0) * resolutionScale, m_EngineContext->GetSubsystem<WindowContext>()->GetWindowHeight(0) * resolutionScale);
    
        // Render Targets - GBuffers
        {
            // Color
            RHI_Texture_Description gBufferDescription;
            gBufferDescription.m_BindFlags = Bind_Flag::Bind_Render_Target | Bind_Flag::Bind_Shader_Resource;
            if (GetMSAASampleCount() == 1)
            {
                gBufferDescription.m_BindFlags |= Bind_Flag::Bind_Unordered_Access;
            }
            gBufferDescription.m_Width = internalResolution.x;
            gBufferDescription.m_Height = internalResolution.y;
            gBufferDescription.m_SampleCount = GetMSAASampleCount();
            gBufferDescription.m_Format = Format::FORMAT_R11G11B10_FLOAT;

            m_GraphicsDevice->CreateTexture(&gBufferDescription, nullptr, &m_RenderTarget_GBuffer[GBuffer_Types::GBuffer_Color]);
            AURORA_INFO("GBuffer_Color Texture Creation Success.");

            // Normal, Roughness
            gBufferDescription.m_BindFlags = Bind_Flag::Bind_Render_Target | Bind_Flag::Bind_Shader_Resource;
            gBufferDescription.m_Format = Format::FORMAT_R8G8B8A8_UNORM;

            m_GraphicsDevice->CreateTexture(&gBufferDescription, nullptr, &m_RenderTarget_GBuffer[GBuffer_Types::GBuffer_Normal_Roughness]);
            AURORA_INFO("GBuffer_Normal_Roughness Texture Creation Success.");
        }

        // Depth Buffers
        {
            RHI_Texture_Description depthBufferDescription;
            depthBufferDescription.m_Width = internalResolution.x;
            depthBufferDescription.m_Height = internalResolution.y;
            depthBufferDescription.m_SampleCount = GetMSAASampleCount();
            depthBufferDescription.m_Layout = Image_Layout::Image_Layout_DepthStencil_ReadOnly;
            depthBufferDescription.m_Format = Format::FORMAT_R32G8X24_TYPELESS;
            depthBufferDescription.m_BindFlags = Bind_Flag::Bind_Depth_Stencil | Bind_Flag::Bind_Shader_Resource;

            m_GraphicsDevice->CreateTexture(&depthBufferDescription, nullptr, &m_DepthBuffer_Main);
            AURORA_INFO("DepthBuffer_Main Texture Creation Success.");
        }
        

        // Render Passes - GBuffer
        {
            // Main (GBuffer Color, Normal & Roughtness)
            RHI_RenderPass_Description renderPassDescription;

            // Main (GBuffer Color, Normal & Roughness)
            renderPassDescription.m_Attachments.clear();
            renderPassDescription.m_Attachments.push_back(RHI_RenderPass_Attachment::RenderTarget(&m_RenderTarget_GBuffer[GBuffer_Types::GBuffer_Color], RHI_RenderPass_Attachment::LoadOperation_DontCare));
            renderPassDescription.m_Attachments.push_back(RHI_RenderPass_Attachment::RenderTarget(&m_RenderTarget_GBuffer[GBuffer_Types::GBuffer_Normal_Roughness], RHI_RenderPass_Attachment::LoadOperation_DontCare));
            renderPassDescription.m_Attachments.push_back(RHI_RenderPass_Attachment::DepthStencil(&m_DepthBuffer_Main, RHI_RenderPass_Attachment::LoadOperation_Load, RHI_RenderPass_Attachment::StoreOperation_Store, Image_Layout::Image_Layout_Shader_Resource, Image_Layout::Image_Layout_DepthStencil_ReadOnly, Image_Layout::Image_Layout_DepthStencil_ReadOnly));

            m_GraphicsDevice->CreateRenderPass(&renderPassDescription, &m_RenderPass_Main);
            AURORA_INFO("Main Render Pass Creation Success.");      
        }
    }

    void Renderer::Tick(float deltaTime)
    {
        if (m_RenderTarget_GBuffer[GBuffer_Types::GBuffer_Color].m_Description.m_Width != m_EngineContext->GetSubsystem<WindowContext>()->GetWindowWidth(0) ||
            m_RenderTarget_GBuffer[GBuffer_Types::GBuffer_Color].m_Description.m_Height != m_EngineContext->GetSubsystem<WindowContext>()->GetWindowHeight(0))
        {
            ResizeBuffers();

            RHI_SwapChain_Description swapchainDescription;
            swapchainDescription.m_Width = static_cast<uint32_t>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowWidth(0));
            swapchainDescription.m_Height = static_cast<uint32_t>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowHeight(0));

            m_GraphicsDevice->CreateSwapChain(&swapchainDescription, &m_SwapChain);
        }

        UpdateCameraConstantBuffer(m_Camera, 0);
        BindConstantBuffers(Shader_Stage::Vertex_Shader, 0);
        BindConstantBuffers(Shader_Stage::Pixel_Shader, 0);

        ID3D11SamplerState* samplerState = DX11_Utility::ToInternal(&m_Standard_Texture_Sampler)->m_Resource.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->PSSetSamplers(0, 1, &samplerState);

        /// Rendering to Texture
        //==============================================================================================================
        D3D11_VIEWPORT viewportInfo = { 0, 0, m_EngineContext->GetSubsystem<WindowContext>()->GetWindowWidth(0), m_EngineContext->GetSubsystem<WindowContext>()->GetWindowHeight(0), 0.0f, 1.0f };
        m_GraphicsDevice->m_DeviceContextImmediate->RSSetViewports(1, &viewportInfo);

        auto ourTexture = DX11_Utility::ToInternal(&m_RenderTarget_GBuffer[GBuffer_Types::GBuffer_Color]);
        ID3D11RenderTargetView* renderTargetViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
        renderTargetViews[0] = ourTexture->m_RenderTargetView.Get();

        ID3D11DepthStencilView* ourDepthStencilTexture = DX11_Utility::ToInternal(&m_DepthBuffer_Main)->m_DepthStencilView.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->OMSetRenderTargets(1, renderTargetViews, ourDepthStencilTexture);

        float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        m_GraphicsDevice->m_DeviceContextImmediate->ClearRenderTargetView(ourTexture->m_RenderTargetView.Get(), color);
        m_GraphicsDevice->m_DeviceContextImmediate->ClearDepthStencilView(ourDepthStencilTexture, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        
        ///==============================

        RenderScene();
        DrawDebugWorld(m_Camera.get());

        /// ==================================
        auto internalState = DX11_Utility::ToInternal(&m_SwapChain);

         // Clear the backbuffer to black for the new frame.
        float backgroundColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

        ID3D11RenderTargetView* RTV = internalState->m_RenderTargetView.Get();   
        m_GraphicsDevice->m_DeviceContextImmediate->OMSetRenderTargets(1, &RTV, 0); // Set depth as well here if it exists.    
        // Present is called from our Editor.
    }

    void Renderer::RenderScene()
    {
        std::vector<std::shared_ptr<Entity>> sceneEntities = m_EngineContext->GetSubsystem<World>()->EntityGetAll();
        std::vector<Mesh> meshComponents;
        for (auto& entity : sceneEntities)
        {
            if (entity->HasComponent<Mesh>())
            {
                meshComponents.push_back(*entity->GetComponent<Mesh>());
            }
        }

        m_GraphicsDevice->BindPipelineState(&RendererGlobals::m_PSO_Object_Wire, 0);

        for (auto& meshComponent : meshComponents)
        {
            UINT offset = 0;
            UINT modelStride = 8 * sizeof(float);
            ID3D11ShaderResourceView* shaderResourceView = DX11_Utility::ToInternal(&meshComponent.m_BaseTexture->m_Texture)->m_ShaderResourceView.Get();
            m_GraphicsDevice->m_DeviceContextImmediate->PSSetShaderResources(0, 1, &shaderResourceView);

            ConstantBufferData_Camera constantBuffer;
            XMStoreFloat4x4(&constantBuffer.g_ObjectMatrix, meshComponent.GetEntity()->m_ObjectMatrix * m_Camera->GetComponent<Camera>()->GetViewProjectionMatrix());
            XMStoreFloat4x4(&constantBuffer.g_WorldMatrix, meshComponent.GetEntity()->m_ObjectMatrix);
            m_GraphicsDevice->UpdateBuffer(&RendererGlobals::g_ConstantBuffers[CB_Types::CB_Camera], &constantBuffer, 0);

            ID3D11Buffer* vertexBuffer = (ID3D11Buffer*)DX11_Utility::ToInternal(&meshComponent.m_VertexBuffer_Position)->m_Resource.Get();
            m_GraphicsDevice->m_DeviceContextImmediate->IASetVertexBuffers(0, 1, &vertexBuffer, &modelStride, &offset);
            m_GraphicsDevice->Draw((UINT)meshComponent.m_VertexPositions.size(), 0, 0);
        }
    }

    void Renderer::DrawDebugWorld(Entity* entity)
    {
        m_GraphicsDevice->BindPipelineState(&RendererGlobals::m_PSO_Object_Debug[DebugRenderer_Type::DebugRenderer_Grid], 0);
        // Bind Grid Pipeline
        Camera* camera = entity->GetComponent<Camera>();

        static float col = 0.7f;
        static uint32_t gridVertexCount = 0;
        static RHI_GPU_Buffer gridBuffer;

        if (!gridBuffer.IsValid())
        {
            const float h = 0.01f; // avoid z-fight with zero plane
            const int a = 20;
            XMFLOAT4 verts[((a + 1) * 2 + (a + 1) * 2) * 2];

            int count = 0;
            for (int i = 0; i <= a; ++i)
            {
                verts[count++] = XMFLOAT4(i - a * 0.5f, h, -a * 0.5f, 1);
                verts[count++] = (i == a / 2 ? XMFLOAT4(0, 0, 1, 1) : XMFLOAT4(col, col, col, 1));

                verts[count++] = XMFLOAT4(i - a * 0.5f, h, +a * 0.5f, 1);
                verts[count++] = (i == a / 2 ? XMFLOAT4(0, 0, 1, 1) : XMFLOAT4(col, col, col, 1));
            }
            for (int j = 0; j <= a; ++j)
            {
                verts[count++] = XMFLOAT4(-a * 0.5f, h, j - a * 0.5f, 1);
                verts[count++] = (j == a / 2 ? XMFLOAT4(1, 0, 0, 1) : XMFLOAT4(col, col, col, 1));

                verts[count++] = XMFLOAT4(+a * 0.5f, h, j - a * 0.5f, 1);
                verts[count++] = (j == a / 2 ? XMFLOAT4(1, 0, 0, 1) : XMFLOAT4(col, col, col, 1));
            }

            gridVertexCount = ARRAYSIZE(verts) / 2;

            RHI_GPU_Buffer_Description bufferDescription;
            bufferDescription.m_Usage = Usage::Immutable;
            bufferDescription.m_ByteWidth = sizeof(verts);
            bufferDescription.m_BindFlags = Bind_Flag::Bind_Vertex_Buffer;
            bufferDescription.m_CPUAccessFlags = 0;

            RHI_Subresource_Data initializationData;
            initializationData.m_SystemMemory = verts;

            m_GraphicsDevice->CreateBuffer(&bufferDescription, &initializationData, &gridBuffer); 
        }

        ConstantBufferData_Misc miscBuffer;
        XMStoreFloat4x4(&miscBuffer.g_Transform, camera->GetViewProjectionMatrix());
        miscBuffer.g_Color = float4(1, 1, 1, 1);

        Aurora::Light* component = m_EngineContext->GetSubsystem<Aurora::World>()->GetEntityByName("Directional_Light")->GetComponent<Aurora::Light>();
        miscBuffer.g_Light_Color = { component->m_Color.x, component->m_Color.y, component->m_Color.z, 0 };
        miscBuffer.g_Light_Position = { component->m_Position.x, component->m_Position.y, component->m_Position.z, 0 };

        m_GraphicsDevice->UpdateBuffer(&RendererGlobals::g_ConstantBuffers[CB_Types::CB_Misc], &miscBuffer, 0);
        m_GraphicsDevice->BindConstantBuffer(Shader_Stage::Vertex_Shader, &RendererGlobals::g_ConstantBuffers[CB_Types::CB_Misc], CB_GETBINDSLOT(ConstantBufferData_Misc), 0);
        m_GraphicsDevice->BindConstantBuffer(Shader_Stage::Pixel_Shader, &RendererGlobals::g_ConstantBuffers[CB_Types::CB_Misc], CB_GETBINDSLOT(ConstantBufferData_Misc), 0);
        
        uint32_t offset = 0;
        const uint32_t stride = sizeof(XMFLOAT4) + sizeof(XMFLOAT4);
        ID3D11Buffer* vertexBufferDebug = (ID3D11Buffer*)DX11_Utility::ToInternal(&gridBuffer)->m_Resource.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->IASetVertexBuffers(0, 1, &vertexBufferDebug, &stride, &offset);
        m_GraphicsDevice->Draw(gridVertexCount, 0, 0);
    }

    void Renderer::Present()
    {
        auto internalState = DX11_Utility::ToInternal(&m_SwapChain);
        internalState->m_SwapChain->Present(1, 0);
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