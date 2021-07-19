#include "Aurora.h"
#include "Renderer.h"
#include "../Window/WindowContext.h"
#include "../Graphics/DX11/DX11_Utilities.h" //Temporary
#include "ShaderInternals.h"
#include "RendererResources.h"
#include "../Scene/Components/Light.h"
#include "../Scene/Components/Mesh.h"
#include "../Scene/Components/Material.h"
#include "../Resource/Importers/Importer_Image.h"

using namespace DirectX;

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
        ResizeBuffers();
        LoadDefaultTextures();

        RHI_SwapChain_Description swapchainDescription;
        swapchainDescription.m_Width = static_cast<uint32_t>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowWidth(0));
        swapchainDescription.m_Height = static_cast<uint32_t>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowHeight(0));

        m_GraphicsDevice->CreateSwapChain(&swapchainDescription, &m_SwapChain);

        CreateTexture();

        m_ResourceCache = m_EngineContext->GetSubsystem<ResourceCache>();
        PrepareSkyboxResources();

        m_Camera = m_EngineContext->GetSubsystem<World>()->GetEntityByName("Default_Camera");
        m_Camera->GetComponent<Camera>()->SetPosition(3.0f, 3.0f, -10.0f);
        m_Camera->GetComponent<Camera>()->ComputePerspectiveMatrix(90.0f, static_cast<float>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowWidth(0)) / static_cast<float>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowHeight(0)), 0.1f, 1000.0f);

        m_ResourceCache->LoadModel("../Resources/Models/Hollow_Knight/source/v3.obj");
        m_ResourceCache->LoadModel("../Resources/Models/Skybox/skybox.obj", "Skybox");
        m_EngineContext->GetSubsystem<World>()->GetEntityByName("defaultobject")->GetComponent<Transform>()->Scale({ 35, 35, 35 });

        auto lightEntity1 = m_EngineContext->GetSubsystem<World>()->EntityCreate();
        lightEntity1->SetName("PL 1");
        lightEntity1->AddComponent<Light>();

        auto lightEntity2 = m_EngineContext->GetSubsystem<World>()->EntityCreate();
        lightEntity2->SetName("PL 2");
        lightEntity2->AddComponent<Light>();

        auto lightEntity3 = m_EngineContext->GetSubsystem<World>()->EntityCreate();
        lightEntity3->SetName("PL 3");
        lightEntity3->AddComponent<Light>();

        auto lightEntity4 = m_EngineContext->GetSubsystem<World>()->EntityCreate();
        lightEntity4->SetName("PL 4");
        lightEntity4->AddComponent<Light>();
        

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
            m_Camera->GetComponent<Camera>()->ComputePerspectiveMatrix(90.0f, static_cast<float>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowWidth(0)) / static_cast<float>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowHeight(0)), 0.1f, 1000.0f);
        }

        UpdateCameraConstantBuffer(m_Camera, 0);
        BindConstantBuffers(Shader_Stage::Vertex_Shader, 0);
        BindConstantBuffers(Shader_Stage::Pixel_Shader, 0);

        ID3D11SamplerState* samplerState = DX11_Utility::ToInternal(&m_Standard_Texture_Sampler)->m_Resource.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->PSSetSamplers(0, 1, &samplerState);
        m_GraphicsDevice->m_DeviceContextImmediate->PSSetSamplers(1, 1, &samplerState);


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

        // Retrieve Scene Entities
        m_SceneEntities = m_EngineContext->GetSubsystem<World>()->EntityGetAll();
        BindLightResources();
        // Bind whatever.

        RenderScene();
        DrawSkybox();
        DrawDebugWorld(m_Camera.get());

        /// ==================================
        auto internalState = DX11_Utility::ToInternal(&m_SwapChain);

        // Clear the backbuffer to black for the new frame.
        float backgroundColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

        ID3D11RenderTargetView* RTV = internalState->m_RenderTargetView.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->OMSetRenderTargets(1, &RTV, 0); // Set depth as well here if it exists.    
        // Present is called from our Editor.
    }

    void Renderer::BindLightResources() // We can only bind up to 16 point lights at this time.
    {
        std::vector<std::shared_ptr<Entity>> lightEntities;
        for (std::shared_ptr<Entity> entity : m_SceneEntities) { if (entity->HasComponent<Light>()) { lightEntities.push_back(entity); } }

        ConstantBufferData_Misc miscConstantBuffer;
        
        for (int i = 0; i < lightEntities.size(); i++) // We will update for each as many light entities we have.
        {
            XMFLOAT4 position = { lightEntities[i]->GetComponent<Transform>()->GetPosition().x, lightEntities[i]->GetComponent<Transform>()->GetPosition().y, lightEntities[i]->GetComponent<Transform>()->GetPosition().z, 1 };
            XMFLOAT4 color = { lightEntities[i]->GetComponent<Light>()->m_Color.x, lightEntities[i]->GetComponent<Light>()->m_Color.y, lightEntities[i]->GetComponent<Light>()->m_Color.z, 1 };        
            miscConstantBuffer.g_Light_Position[i] = position;
            miscConstantBuffer.g_Light_Color[i] = color;
        }

        m_GraphicsDevice->UpdateBuffer(&RendererGlobals::g_ConstantBuffers[CB_Types::CB_Misc], &miscConstantBuffer, 0);
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

        // We can play around with the below flow by binding the right pipeline depending on the material extracted from the mesh's information.
        for (auto& meshComponent : meshComponents)
        {
            UINT offset = 0;
            UINT modelStride = 8 * sizeof(float);

            m_GraphicsDevice->BindPipelineState(&RendererGlobals::m_PSO_Object_Wire, 0);

            ConstantBufferData_Camera constantBuffer;
            XMStoreFloat4x4(&constantBuffer.g_ObjectMatrix, meshComponent.GetEntity()->m_Transform->GetLocalMatrix() * m_Camera->GetComponent<Camera>()->GetViewProjectionMatrix());
            XMStoreFloat4x4(&constantBuffer.g_WorldMatrix, meshComponent.GetEntity()->m_Transform->GetLocalMatrix());
            m_GraphicsDevice->UpdateBuffer(&RendererGlobals::g_ConstantBuffers[CB_Types::CB_Camera], &constantBuffer, 0);

            ID3D11Buffer* vertexBuffer = (ID3D11Buffer*)DX11_Utility::ToInternal(&meshComponent.m_VertexBuffer_Position)->m_Resource.Get();
            m_GraphicsDevice->m_DeviceContextImmediate->IASetVertexBuffers(0, 1, &vertexBuffer, &modelStride, &offset);
            m_GraphicsDevice->BindIndexBuffer(&meshComponent.m_IndexBuffer, meshComponent.GetIndexFormat(), 0, 0);

            if (meshComponent.GetEntity()->GetObjectName() == "defaultobject")
            {
                continue;
            }
            else
            {
                if (meshComponent.GetEntity()->GetComponent<Material>()->m_Textures[TextureSlot::BaseColorMap].m_Resource->m_Texture.IsValid())
                {
                    ID3D11ShaderResourceView* shaderResourceView = DX11_Utility::ToInternal(&meshComponent.GetEntity()->GetComponent<Material>()->m_Textures[TextureSlot::BaseColorMap].m_Resource->m_Texture)->m_ShaderResourceView.Get();
                    m_GraphicsDevice->m_DeviceContextImmediate->PSSetShaderResources(0, 1, &shaderResourceView);

                    constantBuffer.g_ObjectColor = meshComponent.GetEntity()->GetComponent<Material>()->m_BaseColor;
                    m_GraphicsDevice->UpdateBuffer(&RendererGlobals::g_ConstantBuffers[CB_Types::CB_Camera], &constantBuffer, 0);
                }
            }

            m_GraphicsDevice->m_DeviceContextImmediate->DrawIndexed(meshComponent.m_Indices.size(), 0, 0);
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

    void Renderer::PrepareSkyboxResources()
    {
        // Load each of our cubemaps.
        for (int i = 0; i < 6; i++)
        {
            std::string filePath = "../Resources/Cubemaps/Space/" + std::to_string(i) + ".jpg";
            std::shared_ptr<CubeImage> resource = m_ResourceCache->m_Importer_Image->LoadCubeImage(filePath);
            m_CubemapTextures.push_back(resource);
        }

        D3D11_TEXTURE2D_DESC textureCubeDescription = {};
        textureCubeDescription.Width = m_CubemapTextures[0]->m_Width;
        textureCubeDescription.Height = m_CubemapTextures[0]->m_Height;
        textureCubeDescription.MipLevels = 1;
        textureCubeDescription.ArraySize = 6;
        textureCubeDescription.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        textureCubeDescription.SampleDesc.Count = 1;
        textureCubeDescription.SampleDesc.Quality = 0;
        textureCubeDescription.Usage = D3D11_USAGE_DEFAULT;
        textureCubeDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        textureCubeDescription.CPUAccessFlags = 0;
        textureCubeDescription.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

        D3D11_SUBRESOURCE_DATA data[6];
        for (int i = 0; i < 6; i++)
        {
            data[i].pSysMem = m_CubemapTextures[i]->m_Pixels.get();
            data[i].SysMemPitch = m_CubemapTextures[i]->Pitch();
            data[i].SysMemSlicePitch = 0;
        }

        // Create the texture resource.
        ComPtr<ID3D11Texture2D> cubeTexture;
        m_GraphicsDevice->m_Device->CreateTexture2D(&textureCubeDescription, data, &cubeTexture);

        // Create the resource view on the texture.
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDescription = {};
        srvDescription.Format = textureCubeDescription.Format;
        srvDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        srvDescription.Texture2D.MostDetailedMip = 0;
        srvDescription.Texture2D.MipLevels = 1;
        m_GraphicsDevice->m_Device->CreateShaderResourceView(cubeTexture.Get(), &srvDescription, &m_CubeSRV);

        LoadShader(Shader_Stage::Vertex_Shader, m_SkyboxVS, "SkyboxVS.hlsl");
        LoadShader(Shader_Stage::Pixel_Shader, m_SkyboxPS, "SkyboxPS.hlsl");
    }

    void Renderer::DrawSkybox()
    {
        /*
        UINT offset = 0;
        UINT modelStride = 8 * sizeof(float);

        ConstantBufferData_Camera constantBuffer;
        XMStoreFloat4x4(&constantBuffer.g_ObjectMatrix, meshComponent.GetEntity()->m_Transform->GetLocalMatrix() * m_Camera->GetComponent<Camera>()->GetViewProjectionMatrix());
        XMStoreFloat4x4(&constantBuffer.g_WorldMatrix, meshComponent.GetEntity()->m_Transform->GetLocalMatrix());
        m_GraphicsDevice->UpdateBuffer(&RendererGlobals::g_ConstantBuffers[CB_Types::CB_Camera], &constantBuffer, 0);

        ID3D11Buffer* vertexBuffer = (ID3D11Buffer*)DX11_Utility::ToInternal(&meshComponent.m_VertexBuffer_Position)->m_Resource.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->IASetVertexBuffers(0, 1, &vertexBuffer, &modelStride, &offset);
        m_GraphicsDevice->BindIndexBuffer(&meshComponent.m_IndexBuffer, meshComponent.GetIndexFormat(), 0, 0);

        auto internalState = DX11_Utility::ToInternal(&RendererGlobals::m_PSO_Object_Wire);
        m_GraphicsDevice->m_DeviceContextImmediate->OMSetDepthStencilState(internalState->m_DepthStencilState.Get(), 0);

        ID3D11VertexShader* vertexShader = static_cast<DX11_Utility::DX11_VertexShaderPackage*>(m_SkyboxVS.m_InternalState.get())->m_Resource.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->VSSetShader(vertexShader, nullptr, 0);

        ID3D11PixelShader* pixelShader = static_cast<DX11_Utility::DX11_PixelShaderPackage*>(m_SkyboxPS.m_InternalState.get())->m_Resource.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->PSSetShader(pixelShader, nullptr, 0);

        m_GraphicsDevice->m_DeviceContextImmediate->PSSetShaderResources(1, 1, &m_CubeSRV);
        AURORA_INFO("Draw Skybox!");
        m_GraphicsDevice->m_DeviceContextImmediate->DrawIndexed(meshComponent.m_Indices.size(), 0, 0);
        continue;
        */
    }
}