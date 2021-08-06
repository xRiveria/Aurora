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
#include "../Time/Timer.h"
#include <DirectXMath.h>
#include "../Graphics/DX11/Skybox.h"
#include <iostream>
#include "../Input/Input.h"
#include "Environment.h"

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

        m_Camera = m_EngineContext->GetSubsystem<World>()->GetEntityByName("Default_Camera");
        m_Camera->GetComponent<Transform>()->Translate({ 0.0f, 4.0f, 0.0f });
        m_Camera->GetComponent<Camera>()->ComputePerspectiveMatrix(90.0f, m_RenderWidth / m_RenderHeight, 0.1f, 1000.0f);
        m_Camera->GetComponent<Camera>()->ComputeViewMatrix();

        m_DirectionalLight = m_EngineContext->GetSubsystem<World>()->EntityCreate();
        m_DirectionalLight->SetName("Directional Light");
        m_DirectionalLight->m_Transform->Translate({ 0.01, 4, 0 });

        // auto derp = m_EngineContext->GetSubsystem<ResourceCache>()->LoadModel("../Resources/Models/Skybox/skybox.obj", "Derp");
        // derp->GetComponent<Transform>()->Translate({ 0.0f, 4.0f, 0.0f });

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

        D3D11_VIEWPORT viewportInfo = { 0, 0, (float)1280.0, (float)1080.0, 0.0f, 1.0f };
        m_GraphicsDevice->m_DeviceContextImmediate->RSSetViewports(1, &viewportInfo);
       
        m_Skybox = std::make_shared<Skybox>(m_EngineContext);
        m_Skybox->InitializeResources();
        return true;
    }

    int Renderer::BindMaterialTexture(TextureSlot slotType, Material* material)
    {
        // Remember that our slot type's enum corresponds to our shader material.
        ID3D11ShaderResourceView* shaderResourceView = DX11_Utility::ToInternal(&material->m_Textures[slotType].m_Resource->m_Texture)->m_ShaderResourceView.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->PSSetShaderResources((int)slotType, 1, &shaderResourceView);

        return (int)slotType;
    }

    int Renderer::BindSkyboxTexture(int slotNumber, RHI_Texture* texture)
    {
        // Remember that our slot type's enum corresponds to our shader material.
        ID3D11ShaderResourceView* shaderResourceView = DX11_Utility::ToInternal(texture)->m_ShaderResourceView.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->PSSetShaderResources(slotNumber, 1, &shaderResourceView);

        return slotNumber;
    }

    int Renderer::BindSkyboxTexture(int slotNumber, ID3D11ShaderResourceView* shaderResourceView)
    {
        m_GraphicsDevice->m_DeviceContextImmediate->PSSetShaderResources(slotNumber, 1, &shaderResourceView);

        return slotNumber;
    }

    void Renderer::UpdateMaterialConstantBuffer(Material* materialComponent)
    {
        ConstantBufferData_Material constantBuffer;

        constantBuffer.g_Material.g_ObjectColor = materialComponent->m_BaseColor;
        constantBuffer.g_Material.g_Roughness = materialComponent->m_Roughness;
        constantBuffer.g_Material.g_Metalness = materialComponent->m_Metalness;
        if (m_Camera != nullptr)
        {
            constantBuffer.g_Camera_Position = m_Camera->GetComponent<Transform>()->m_TranslationLocal;
        }

        constantBuffer.g_Texture_BaseColorMap_Index = BindMaterialTexture(TextureSlot::BaseColorMap, materialComponent);
        constantBuffer.g_Texture_NormalMap_Index = BindMaterialTexture(TextureSlot::NormalMap, materialComponent);
        constantBuffer.g_Texture_MetalnessMap_Index = BindMaterialTexture(TextureSlot::MetalnessMap, materialComponent);
        constantBuffer.g_Texture_RoughnessMap_Index = BindMaterialTexture(TextureSlot::RoughnessMap, materialComponent);

        constantBuffer.g_Texture_IrradianceMap_Index = BindSkyboxTexture(TEXSLOT_RENDERER_SKYCUBE_IRRADIANCE, m_Skybox->m_IrradianceMapTexture.m_SRV.Get());
        constantBuffer.g_Texture_PrefilterMap_Index = BindSkyboxTexture(TEXSLOT_RENDERER_SKYCUBE_PREFILTER, m_Skybox->m_EnvironmentTexture.m_SRV.Get());
        constantBuffer.g_Texture_BRDFLUT_Index = BindSkyboxTexture(TEXSLOT_RENDERER_SKYCUBE_BRDF_LUT, m_Skybox->m_SpecularPrefilterBRDFLUT.m_SRV.Get());

        constantBuffer.g_Texture_DepthShadowMap_Index = BindSkyboxTexture(m_DepthShadowMappingIndex, &m_ShadowDepthMap);

        m_GraphicsDevice->UpdateBuffer(&g_ConstantBuffers[CB_Types::CB_Material], &constantBuffer, 0);
    }

    void Renderer::UpdateEntityConstantBuffer(Entity* entity)
    {
        ConstantBufferData_Entity entityConstantBuffer;

        XMStoreFloat4x4(&entityConstantBuffer.g_ModelMatrix, XMLoadFloat4x4(&entity->GetComponent<Transform>()->m_WorldMatrix));

        m_GraphicsDevice->UpdateBuffer(&g_ConstantBuffers[CB_Types::CB_Entity], &entityConstantBuffer, 0);
    }

    void Renderer::UpdateCameraConstantBuffer(const std::shared_ptr<Entity>& camera, RHI_CommandList commandList)
    {
        ConstantBufferData_Camera constantBuffer;

        if (camera != nullptr)
        {
            XMStoreFloat4x4(&constantBuffer.g_Camera_ViewProjection, camera->GetComponent<Camera>()->GetViewProjectionMatrix());
            XMStoreFloat4x4(&constantBuffer.g_Camera_View, camera->GetComponent<Camera>()->GetViewMatrix());
            XMStoreFloat4x4(&constantBuffer.g_Camera_Projection, camera->GetComponent<Camera>()->GetProjectionMatrix());
            XMStoreFloat4x4(&constantBuffer.g_Camera_InverseViewProjection, XMMatrixInverse(nullptr, camera->GetComponent<Camera>()->GetViewProjectionMatrix()));
        }
        constantBuffer.g_Light_Bias = m_LightBias;
        // XMStoreFloat3(&constantBuffer.g_Camera_Position, camera->GetComponent<Camera>()->m_Position);

        m_GraphicsDevice->UpdateBuffer(&g_ConstantBuffers[CB_Types::CB_Camera], &constantBuffer, commandList);
    }

    void Renderer::UpdateLightConstantBuffer() // We can only bind up to 16 point lights at this time.
    {
        std::vector<std::shared_ptr<Entity>> lightEntities;
        for (std::shared_ptr<Entity> entity : m_SceneEntities)
        {
            if (entity->IsActive())
            {
                if (entity->HasComponent<Light>())
                {
                    lightEntities.push_back(entity);
                }
            }
        }

        ConstantBufferData_Frame miscConstantBuffer;

        // Our Ortho Projection Matrix for Directional Light Source Modelling
        float nearPlane = 1.0f, farPlane = 50.0f;

        miscConstantBuffer.g_Light_Count = lightEntities.size();
        for (int i = 0; i < lightEntities.size(); i++) // We will update for each as many light entities we have.
        {
            XMFLOAT4 position = { lightEntities[i]->GetComponent<Transform>()->m_TranslationLocal.x, lightEntities[i]->GetComponent<Transform>()->m_TranslationLocal.y, lightEntities[i]->GetComponent<Transform>()->m_TranslationLocal.z, 1 };
            XMFLOAT4 color = { lightEntities[i]->GetComponent<Light>()->m_Color.x, lightEntities[i]->GetComponent<Light>()->m_Color.y, lightEntities[i]->GetComponent<Light>()->m_Color.z, 0.0f };
            miscConstantBuffer.g_Light_Position[i] = position;
            miscConstantBuffer.g_Light_Color[i] = color;
            miscConstantBuffer.g_Light_Color[i].w = lightEntities[i]->GetComponent<Light>()->m_Intensity;
        }

        XMFLOAT3 lookAtPosition = { 0, 0, 0 };
        XMFLOAT3 upPosition = { 0.0f, 1.0f, 0.0f };

        XMFLOAT3 lightTranslation = m_DirectionalLight->GetComponent<Transform>()->m_TranslationLocal;
        XMMATRIX viewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&lightTranslation), { 0.0, 0.0, 0.0 }, { 0, 1, 0 });
        XMMATRIX orthographicMatrix = XMMatrixOrthographicLH(20, 20, -20, 20);

        XMMATRIX lightSpaceMatrix = viewMatrix * orthographicMatrix; // This will render everything from the light's perspective.

        XMStoreFloat4x4(&miscConstantBuffer.g_LightSpaceMatrix, lightSpaceMatrix);

        m_GraphicsDevice->UpdateBuffer(&g_ConstantBuffers[CB_Types::CB_Frame], &miscConstantBuffer, 0);
    }

    void Renderer::BindConstantBuffers(Shader_Stage shaderStage, RHI_CommandList commandList)
    {
        m_GraphicsDevice->BindConstantBuffer(shaderStage, &g_ConstantBuffers[CB_Types::CB_Camera], CB_GETBINDSLOT(ConstantBufferData_Camera), 0);
        m_GraphicsDevice->BindConstantBuffer(shaderStage, &g_ConstantBuffers[CB_Types::CB_Material], CB_GETBINDSLOT(ConstantBufferData_Material), 0);
        m_GraphicsDevice->BindConstantBuffer(shaderStage, &g_ConstantBuffers[CB_Types::CB_Frame], CB_GETBINDSLOT(ConstantBufferData_Frame), 0);
    }

    inline void Renderer::ResizeBuffers()
    {
        float resolutionScale = 1.0f;
        XMUINT2 internalResolution = XMUINT2(m_RenderWidth, m_RenderHeight);

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
            gBufferDescription.m_Format = Format::FORMAT_R32G32B32A32_FLOAT; // Floating point for tonemapping.

            m_GraphicsDevice->CreateTexture(&gBufferDescription, nullptr, &m_RenderTarget_GBuffer[GBuffer_Types::GBuffer_Color]);
            AURORA_INFO("GBuffer_Color Texture Creation Success.");

            // Normal, Roughness
            gBufferDescription.m_BindFlags = Bind_Flag::Bind_Render_Target | Bind_Flag::Bind_Shader_Resource;
            gBufferDescription.m_Format = Format::FORMAT_R8G8B8A8_UNORM;

            m_GraphicsDevice->CreateTexture(&gBufferDescription, nullptr, &m_RenderTarget_GBuffer[GBuffer_Types::GBuffer_Normal_Roughness]);
            AURORA_INFO("GBuffer_Normal_Roughness Texture Creation Success.");
        }

        // Render Targets - Bloom
        {
            // Bloom
            RHI_Texture_Description bloomBufferDescription;
            bloomBufferDescription.m_BindFlags = Bind_Flag::Bind_Render_Target | Bind_Flag::Bind_Shader_Resource;
            if (GetMSAASampleCount() == 1)
            {
                bloomBufferDescription.m_BindFlags |= Bind_Flag::Bind_Unordered_Access;
            }
            bloomBufferDescription.m_Width = internalResolution.x;
            bloomBufferDescription.m_Height = internalResolution.y;
            bloomBufferDescription.m_SampleCount = GetMSAASampleCount();
            bloomBufferDescription.m_Format = Format::FORMAT_R32G32B32A32_FLOAT; // Floating point for tonemapping.

            m_GraphicsDevice->CreateTexture(&bloomBufferDescription, nullptr, &m_RenderTarget_GBuffer[GBuffer_Types::GBuffer_Bloom]);
            AURORA_INFO("GBuffer_Bloom Texture Creation Success.");
        }

        // Render Targets - PingPong1
        {
            // Bloom
            RHI_Texture_Description bloomBufferDescription;
            bloomBufferDescription.m_BindFlags = Bind_Flag::Bind_Render_Target | Bind_Flag::Bind_Shader_Resource;
            if (GetMSAASampleCount() == 1)
            {
                bloomBufferDescription.m_BindFlags |= Bind_Flag::Bind_Unordered_Access;
            }
            bloomBufferDescription.m_Width = internalResolution.x;
            bloomBufferDescription.m_Height = internalResolution.y;
            bloomBufferDescription.m_SampleCount = GetMSAASampleCount();
            bloomBufferDescription.m_Format = Format::FORMAT_R32G32B32A32_FLOAT;

            m_GraphicsDevice->CreateTexture(&bloomBufferDescription, nullptr, &m_RenderTarget_GBuffer[GBuffer_Types::GBuffer_BloomPingPong1]);
            AURORA_INFO("GBuffer_PingPong1 Texture Creation Success.");
        }

        // Render Targets - PingPong2
        {
            // Bloom
            RHI_Texture_Description bloomBufferDescription;
            bloomBufferDescription.m_BindFlags = Bind_Flag::Bind_Render_Target | Bind_Flag::Bind_Shader_Resource;
            if (GetMSAASampleCount() == 1)
            {
                bloomBufferDescription.m_BindFlags |= Bind_Flag::Bind_Unordered_Access;
            }
            bloomBufferDescription.m_Width = internalResolution.x;
            bloomBufferDescription.m_Height = internalResolution.y;
            bloomBufferDescription.m_SampleCount = GetMSAASampleCount();
            bloomBufferDescription.m_Format = Format::FORMAT_R32G32B32A32_FLOAT; 

            m_GraphicsDevice->CreateTexture(&bloomBufferDescription, nullptr, &m_RenderTarget_GBuffer[GBuffer_Types::GBuffer_BloomPingPong2]);
            AURORA_INFO("GBuffer_PingPong2 Texture Creation Success.");
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

            RHI_Texture_Description shadowDepthBufferDescription;
            shadowDepthBufferDescription.m_Width = internalResolution.x;
            shadowDepthBufferDescription.m_Height = internalResolution.y;
            shadowDepthBufferDescription.m_SampleCount = GetMSAASampleCount();
            shadowDepthBufferDescription.m_Layout = Image_Layout::Image_Layout_DepthStencil_ReadOnly;
            shadowDepthBufferDescription.m_Format = Format::FORMAT_R32G8X24_TYPELESS;
            shadowDepthBufferDescription.m_BindFlags = Bind_Flag::Bind_Depth_Stencil | Bind_Flag::Bind_Shader_Resource;

            m_GraphicsDevice->CreateTexture(&shadowDepthBufferDescription, nullptr, &m_ShadowDepthMap);
            AURORA_INFO("Shadow Depth Buffer Texture Creation Success.");
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
        if (m_RenderTarget_GBuffer[GBuffer_Types::GBuffer_Color].m_Description.m_Width  != m_RenderWidth ||
            m_RenderTarget_GBuffer[GBuffer_Types::GBuffer_Color].m_Description.m_Height != m_RenderHeight)
        {
            ResizeBuffers();

            RHI_SwapChain_Description swapchainDescription;
            swapchainDescription.m_Width = static_cast<uint32_t>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowWidth(0));
            swapchainDescription.m_Height = static_cast<uint32_t>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowHeight(0));

            m_GraphicsDevice->CreateSwapChain(&swapchainDescription, &m_SwapChain);
            if (m_Camera != nullptr)
            {
                m_Camera->GetComponent<Camera>()->ComputePerspectiveMatrix(90.0f, m_RenderWidth / m_RenderHeight, 0.1f, 1000.0f);
            }

            D3D11_VIEWPORT viewportInfo = { 0, 0, m_RenderWidth, m_RenderHeight, 0.0f, 1.0f };
            m_GraphicsDevice->m_DeviceContextImmediate->RSSetViewports(1, &viewportInfo);

            AURORA_INFO("RESIZED");
        }

        BindConstantBuffers(Shader_Stage::Vertex_Shader, 0);
        BindConstantBuffers(Shader_Stage::Pixel_Shader, 0);

        UpdateLightConstantBuffer();
        if (m_Camera != nullptr)
        {
            UpdateCameraConstantBuffer(m_Camera, 0);
        }

        ID3D11SamplerState* samplerState = DX11_Utility::ToInternal(&m_Standard_Texture_Sampler)->m_Resource.Get();
        ID3D11SamplerState* samplerState2 = DX11_Utility::ToInternal(&m_Depth_Texture_Sampler)->m_Resource.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->PSSetSamplers(0, 1, &samplerState);
        m_GraphicsDevice->m_DeviceContextImmediate->PSSetSamplers(1, 1, &samplerState2);
        m_GraphicsDevice->m_DeviceContextImmediate->PSSetSamplers(3, 1, m_Skybox->m_DefaultSampler.GetAddressOf());


        /// Rendering to Texture
        //==============================================================================================================
        auto ourTexture = DX11_Utility::ToInternal(&m_RenderTarget_GBuffer[GBuffer_Types::GBuffer_Color]);
        auto ourBloomTexture = DX11_Utility::ToInternal(&m_RenderTarget_GBuffer[GBuffer_Types::GBuffer_Bloom]);

        ID3D11RenderTargetView* renderTargetViews[2];
        renderTargetViews[0] = ourTexture->m_RenderTargetView.Get();
        renderTargetViews[1] = ourBloomTexture->m_RenderTargetView.Get();

        // Retrieve Scene Entities
        m_SceneEntities = m_EngineContext->GetSubsystem<World>()->EntityGetAll();


        //============== Depth Buffer Pass ==================
        m_GraphicsDevice->BindPipelineState(&m_PSO_Object_Wire, 0);
        ID3D11VertexShader* vertexShader = static_cast<DX11_Utility::DX11_VertexShaderPackage*>(m_SimpleDepthShaderVS.m_InternalState.get())->m_Resource.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->VSSetShader(vertexShader, nullptr, 0);

        ID3D11PixelShader* pixelShader = static_cast<DX11_Utility::DX11_PixelShaderPackage*>(m_SimpleDepthShaderPS.m_InternalState.get())->m_Resource.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->PSSetShader(pixelShader, nullptr, 0);

        float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        ID3D11DepthStencilView* shadowMapTexture = DX11_Utility::ToInternal(&m_ShadowDepthMap)->m_DepthStencilView.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->OMSetRenderTargets(0, nullptr, shadowMapTexture);
        m_GraphicsDevice->m_DeviceContextImmediate->ClearDepthStencilView(shadowMapTexture, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        RenderScene();

        // ================ Scene Pass ==================================

        ID3D11DepthStencilView* ourDepthStencilTexture = DX11_Utility::ToInternal(&m_DepthBuffer_Main)->m_DepthStencilView.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->OMSetRenderTargets(2, renderTargetViews, ourDepthStencilTexture);

        m_GraphicsDevice->m_DeviceContextImmediate->ClearRenderTargetView(ourBloomTexture->m_RenderTargetView.Get(), color);
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
        m_GraphicsDevice->BindConstantBuffer(Shader_Stage::Vertex_Shader, &g_ConstantBuffers[CB_Types::CB_Entity], CB_GETBINDSLOT(ConstantBufferData_Entity), 0);

        m_GraphicsDevice->BindPipelineState(&m_PSO_Object_Wire, 0);
        /// Render Queue Feature?
        std::vector<std::shared_ptr<Entity>> sceneEntities = m_EngineContext->GetSubsystem<World>()->EntityGetAll();
        std::vector<Mesh> meshComponents;
        for (auto& entity : sceneEntities)
        {
            if (entity->IsActive() && entity->m_EntityType == EntityType::Renderable)
            {
                if (entity->HasComponent<Mesh>())
                {
                    meshComponents.push_back(*entity->GetComponent<Mesh>());
                }
            }
        }

        // We can play around with the below flow by binding the right pipeline depending on the material extracted from the mesh's information.
        for (auto& meshComponent : meshComponents)
        {
            UpdateEntityConstantBuffer(meshComponent.GetEntity());

            UINT offset = 0;
            UINT modelStride = 8 * sizeof(float);

            ID3D11Buffer* vertexBuffer = (ID3D11Buffer*)DX11_Utility::ToInternal(&meshComponent.m_VertexBuffer_Position)->m_Resource.Get();
            m_GraphicsDevice->m_DeviceContextImmediate->IASetVertexBuffers(0, 1, &vertexBuffer, &modelStride, &offset);
            m_GraphicsDevice->BindIndexBuffer(&meshComponent.m_IndexBuffer, meshComponent.GetIndexFormat(), 0, 0);

            if (meshComponent.GetEntity()->HasComponent<Material>())
            {
                UpdateMaterialConstantBuffer(meshComponent.GetEntity()->GetComponent<Material>());
            }

            m_GraphicsDevice->m_DeviceContextImmediate->DrawIndexed(meshComponent.m_Indices.size(), 0, 0);
        }

        m_Skybox->Render();
    }

    void Renderer::DrawDebugWorld(Entity* entity)
    {
        Stopwatch stopwatch("Debug World Pass");
        m_GraphicsDevice->BindPipelineState(&RendererGlobals::m_PSO_Object_Debug[DebugRenderer_Type::DebugRenderer_Grid], 0);
        // Bind Grid Pipeline
        Camera* camera = entity->GetComponent<Camera>();

        static float col = 0.7f;
        static uint32_t gridVertexCount = 0;
        static RHI_GPU_Buffer gridBuffer;

        if (!gridBuffer.IsValid())
        {
            const float h = 0.01f; // avoid z-fight with zero plane
            const int a = 40;
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

        m_GraphicsDevice->UpdateBuffer(&g_ConstantBuffers[CB_Types::CB_Misc], &miscBuffer, 0);
        m_GraphicsDevice->BindConstantBuffer(Shader_Stage::Vertex_Shader, &g_ConstantBuffers[CB_Types::CB_Misc], CB_GETBINDSLOT(ConstantBufferData_Misc), 0);
        m_GraphicsDevice->BindConstantBuffer(Shader_Stage::Pixel_Shader, &g_ConstantBuffers[CB_Types::CB_Misc], CB_GETBINDSLOT(ConstantBufferData_Misc), 0);

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

        samplerDescription.m_Filter = Filter::FILTER_MIN_MAG_MIP_LINEAR;
        samplerDescription.m_AddressU = Texture_Address_Mode::Texture_Address_Clamp;
        samplerDescription.m_AddressV = Texture_Address_Mode::Texture_Address_Clamp;
        samplerDescription.m_AddressW = Texture_Address_Mode::Texture_Address_Clamp;
        samplerDescription.m_BorderColor[0] = 1.0f;
        samplerDescription.m_BorderColor[1] = 1.0f;
        samplerDescription.m_BorderColor[2] = 1.0f;
        samplerDescription.m_BorderColor[3] = 1.0f;
        samplerDescription.m_ComparisonFunction = ComparisonFunction::Comparison_Never;
        samplerDescription.m_MinLOD = 0;
        samplerDescription.m_MaxLOD = D3D11_FLOAT32_MAX;

        m_GraphicsDevice->CreateSampler(&samplerDescription, &m_Depth_Texture_Sampler);
    }

    void Renderer::LoadSkyPipelineState(RHI_Shader* vertexShader, RHI_Shader* pixelShader)
    {
        RHI_PipelineState_Description skyPipelineDescription;

        skyPipelineDescription.m_VertexShader = vertexShader;
        skyPipelineDescription.m_PixelShader = pixelShader;
        skyPipelineDescription.m_InputLayout = &RendererGlobals::g_InputLayouts[InputLayout_Types::OnDemandTriangle];
        skyPipelineDescription.m_RasterizerState = &RendererGlobals::g_RasterizerStates[RS_Types::RS_Front];
        skyPipelineDescription.m_BlendState = &RendererGlobals::g_BlendStates[BS_Types::BS_Opaque];
        skyPipelineDescription.m_DepthStencilState = &RendererGlobals::g_DepthStencilStates[DS_Types::DS_Default];

        m_GraphicsDevice->CreatePipelineState(&skyPipelineDescription, &m_PSO_Object_Sky);
    }
}