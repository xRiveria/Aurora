#include "Aurora.h"
#include "Renderer.h"
#include "../Resource/ResourceCache.h"
#include "../Window/WindowContext.h"
#include "../Graphics/DX11/DX11_Utilities.h" //Temporary
#include "ShaderInternals.h"
#include "RendererResources.h"
#include "../Scene/Components/Light.h"
#include "../Resource/Importers/Importer_Image.h"
#include "../Time/Timer.h"
#include "../Graphics/DX11_Refactored/DX11_Texture.h"
#include <DirectXMath.h>
#include "../Graphics/DX11/Skybox.h"
#include <iostream>
#include "../Input/Input.h"
#include "../Scene/Components/Renderable.h"
#include "Model.h"

namespace Aurora
{
    Renderer::Renderer(EngineContext* engineContext) : ISubsystem(engineContext)
    {

    }

    Renderer::~Renderer()
    {

    }

    /*
    bool Renderer::Initialize()
    {
        m_GraphicsDevice = std::make_shared<DX11_GraphicsDevice>(m_EngineContext, true); // Previous core API context. We will keep this around for now.
        m_DeviceContext = std::make_shared<DX11_Context>(m_GraphicsDevice->m_Device, m_GraphicsDevice->m_DeviceContextImmediate); // New core API context. To replace in due time.

        m_ShaderCompiler.Initialize();
        LoadShaders(); /// Yet to migrate.


    }
    */



    bool Renderer::Initialize()
    {
        m_GraphicsDevice = std::make_shared<DX11_GraphicsDevice>(m_EngineContext, true); // Previous core API context. We will keep this around for now.
        m_DeviceContext = std::make_shared<DX11_Context>(m_EngineContext, m_GraphicsDevice->m_Device, m_GraphicsDevice->m_DeviceContextImmediate); // New core API context. To replace in due time.

        m_ShaderCompiler.Initialize();
        LoadShaders(); /// Yet to migrate.
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

        m_Camera = m_EngineContext->GetSubsystem<World>()->m_CameraPointer;
        m_Camera->GetComponent<Transform>()->Translate({ 0.0f, 4.0f, 0.0f });
        m_Camera->GetComponent<Camera>()->ComputePerspectiveMatrix(90.0f, m_RenderWidth / m_RenderHeight, 0.1f, 1000.0f);
        m_Camera->GetComponent<Camera>()->ComputeViewMatrix();

        m_DirectionalLight = m_EngineContext->GetSubsystem<World>()->EntityCreate();
        m_DirectionalLight->SetEntityName("Directional Light");
        m_DirectionalLight->m_Transform->Translate({ 0.01, 4, 0 });

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

        m_DeviceContext->Initialize();

        m_Skybox = std::make_shared<Skybox>(m_EngineContext);
        m_Skybox->InitializeResources();

        InitializeShaders();
        return true;
    }

    int Renderer::BindMaterialTexture(MaterialSlot slotType, int slotIndex, Material* material)
    {
        // Remember that our slot type's enum corresponds to our shader material.
        if (material->m_Textures[slotType])
        {
            m_GraphicsDevice->m_DeviceContextImmediate->PSSetShaderResources(slotIndex, 1, material->m_Textures[slotType]->GetShaderResourceView().GetAddressOf());
        }

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

        constantBuffer.g_Material.g_ObjectColor = materialComponent->GetAlbedoColor();
        constantBuffer.g_Material.g_Roughness = materialComponent->m_Properties[MaterialSlot::MaterialSlot_Roughness];
        constantBuffer.g_Material.g_Metalness = materialComponent->m_Properties[MaterialSlot::MaterialSlot_Metallic];
        if (m_Camera != nullptr)
        {
            constantBuffer.g_Camera_Position = m_Camera->GetComponent<Transform>()->m_TranslationLocal;
        }

        constantBuffer.g_Texture_BaseColorMap_Index = BindMaterialTexture(MaterialSlot::MaterialSlot_Albedo, m_BaseMap, materialComponent);
        constantBuffer.g_Texture_NormalMap_Index = BindMaterialTexture(MaterialSlot::MaterialSlot_Normal, m_NormalMapIndex, materialComponent);
        constantBuffer.g_Texture_MetalnessMap_Index = BindMaterialTexture(MaterialSlot::MaterialSlot_Metallic, m_MetalMapIndex, materialComponent);
        constantBuffer.g_Texture_RoughnessMap_Index = BindMaterialTexture(MaterialSlot::MaterialSlot_Roughness, m_RoughnessMapIndex, materialComponent);
        constantBuffer.g_Texture_AOMap_Index = BindMaterialTexture(MaterialSlot::MaterialSlot_Occlusion, m_AOMapIndex, materialComponent);

        constantBuffer.g_Texture_IrradianceMap_Index = BindSkyboxTexture(TEXSLOT_RENDERER_SKYCUBE_IRRADIANCE, m_Skybox->m_IrradianceMapTexture->GetShaderResourceView().Get());
        constantBuffer.g_Texture_PrefilterMap_Index = BindSkyboxTexture(TEXSLOT_RENDERER_SKYCUBE_PREFILTER, m_Skybox->m_EnvironmentTexture->GetShaderResourceView().Get());
        constantBuffer.g_Texture_BRDFLUT_Index = BindSkyboxTexture(TEXSLOT_RENDERER_SKYCUBE_BRDF_LUT, m_Skybox->m_SpecularPrefilterBRDFLUT->GetShaderResourceView().Get());

        constantBuffer.g_Texture_DepthShadowMap_Index = BindSkyboxTexture(m_DepthShadowMappingIndex, m_DeviceContext->m_ShadowDepthTexture->GetShaderResourceView().Get());

        m_GraphicsDevice->UpdateBuffer(&g_ConstantBuffers[CB_Types::CB_Material], &constantBuffer, 0);
    }

    void Renderer::UpdateEntityConstantBuffer(Entity* entity)
    {
        ConstantBufferData_Entity entityConstantBuffer;

        XMStoreFloat4x4(&entityConstantBuffer.g_ModelMatrix, XMLoadFloat4x4(&entity->GetComponent<Transform>()->m_WorldMatrix));

        m_GraphicsDevice->UpdateBuffer(&g_ConstantBuffers[CB_Types::CB_Entity], &entityConstantBuffer, 0);
    }

    void Renderer::UpdateCameraConstantBuffer(Entity* camera, RHI_CommandList commandList)
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

    void Renderer::BindConstantBuffers(RHI_Shader_Stage shaderStage, RHI_CommandList commandList)
    {
        m_GraphicsDevice->BindConstantBuffer(shaderStage, &g_ConstantBuffers[CB_Types::CB_Camera], CB_GETBINDSLOT(ConstantBufferData_Camera), 0);
        m_GraphicsDevice->BindConstantBuffer(shaderStage, &g_ConstantBuffers[CB_Types::CB_Material], CB_GETBINDSLOT(ConstantBufferData_Material), 0);
        m_GraphicsDevice->BindConstantBuffer(shaderStage, &g_ConstantBuffers[CB_Types::CB_Frame], CB_GETBINDSLOT(ConstantBufferData_Frame), 0);
    }

    inline void Renderer::ResizeBuffers()
    {
        float resolutionScale = 1.0f;
        XMUINT2 internalResolution = XMUINT2(m_RenderWidth, m_RenderHeight);

        m_DeviceContext->m_RenderWidth = m_RenderWidth;
        m_DeviceContext->m_RenderHeight = m_RenderHeight;
    }

    void Renderer::Tick(float deltaTime)
    {
        Stopwatch widgetStopwatch("Renderer Pass", true);

        if (static_cast<float>(m_DeviceContext->m_MultisampleFramebuffer->m_RenderTargetTexture->GetWidth()) != m_RenderWidth ||
            static_cast<float>(m_DeviceContext->m_MultisampleFramebuffer->m_RenderTargetTexture->GetHeight()) != m_RenderHeight)
        {
            ResizeBuffers();
            m_DeviceContext->ResizeBuffers();

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

            AURORA_INFO(LogLayer::Graphics, "Window Size Resized");
        }

        BindConstantBuffers(RHI_Shader_Stage::Vertex_Shader, 0);
        BindConstantBuffers(RHI_Shader_Stage::Pixel_Shader, 0);

        UpdateLightConstantBuffer();
        if (m_Camera != nullptr)
        {
            UpdateCameraConstantBuffer(m_Camera, 0);
        }

        ID3D11SamplerState* samplerState = DX11_Utility::ToInternal(&m_Standard_Texture_Sampler)->m_Resource.Get();
        ID3D11SamplerState* samplerState2 = DX11_Utility::ToInternal(&m_Depth_Texture_Sampler)->m_Resource.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->PSSetSamplers(0, 1, &samplerState);
        m_GraphicsDevice->m_DeviceContextImmediate->PSSetSamplers(1, 1, &samplerState2);
        m_GraphicsDevice->m_DeviceContextImmediate->PSSetSamplers(3, 1, m_Skybox->m_DefaultSampler->GetSampler().GetAddressOf());
        m_GraphicsDevice->m_DeviceContextImmediate->PSSetSamplers(4, 1, m_Skybox->m_SpecularBRDFSampler->GetSampler().GetAddressOf());

        // Retrieve Scene Entities
        m_SceneEntities = m_EngineContext->GetSubsystem<World>()->EntityGetAll();


        //============== Depth Buffer Pass ==================
        m_GraphicsDevice->BindPipelineState(&m_PSO_Object_Wire, 0);
        m_DeviceContext->BindRasterizerState(RasterizerState_Types::RasterizerState_Shadow);
        ID3D11VertexShader* vertexShader = static_cast<DX11_Utility::DX11_VertexShaderPackage*>(m_SimpleDepthShaderVS.m_InternalState.get())->m_Resource.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->VSSetShader(vertexShader, nullptr, 0);

        ID3D11PixelShader* pixelShader = static_cast<DX11_Utility::DX11_PixelShaderPackage*>(m_SimpleDepthShaderPS.m_InternalState.get())->m_Resource.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->PSSetShader(pixelShader, nullptr, 0);

        float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        m_GraphicsDevice->m_DeviceContextImmediate->OMSetRenderTargets(0, nullptr, m_DeviceContext->m_ShadowDepthTexture->GetDepthStencilView().Get());
        m_GraphicsDevice->m_DeviceContextImmediate->ClearDepthStencilView(m_DeviceContext->m_ShadowDepthTexture->GetDepthStencilView().Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        RenderScene();

        // ============= Bloom Extraction Pass =================== 
        // Bloom Threashold and stuff
        m_GraphicsDevice->BindPipelineState(&m_PSO_Object_Wire, 0);

        ID3D11VertexShader* bloomVertexShader = static_cast<DX11_Utility::DX11_VertexShaderPackage*>(m_BloomVS.m_InternalState.get())->m_Resource.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->VSSetShader(bloomVertexShader, nullptr, 0);

        ID3D11PixelShader* bloomPixelShader = static_cast<DX11_Utility::DX11_PixelShaderPackage*>(m_BloomPS.m_InternalState.get())->m_Resource.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->PSSetShader(bloomPixelShader, nullptr, 0);

        m_GraphicsDevice->m_DeviceContextImmediate->OMSetRenderTargets(1, m_DeviceContext->m_BloomRenderTexture->GetRenderTargetView().GetAddressOf(), m_DeviceContext->m_DummyDepthTexture->GetDepthStencilView().Get());
        m_GraphicsDevice->m_DeviceContextImmediate->ClearRenderTargetView(m_DeviceContext->m_BloomRenderTexture->GetRenderTargetView().Get(), color);
        m_GraphicsDevice->m_DeviceContextImmediate->ClearDepthStencilView(m_DeviceContext->m_DummyDepthTexture->GetDepthStencilView().Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        RenderScene();

        /*
        // ============ Blur Pass ==================
        // We will blur the image 10 times, 5 horizontally and 5 vertically.
        const int blurPassCount = 10;
        for (int i = 0; i < blurPassCount; ++i)
        {
            const int isHorizontal = i % 2; // Alternate between 0 and 1
            const DX11_Texture* pingPongTexture = m_DeviceContext->m_Blur_PingPongTexture[1 - isHorizontal].get();
            const int textureWidth = pingPongTexture->GetWidth();
            const int textureHeight = pingPongTexture->GetHeight();

            /// Set Shader
            /// Set Constant
            /// Texture
            /// Set Sampler State
            /// Draw
        }
        */

        // ================ Scene Pass ==================================
        m_GraphicsDevice->BindPipelineState(&m_PSO_Object_Wire, 0);

        m_GraphicsDevice->m_DeviceContextImmediate->OMSetRenderTargets(1, m_DeviceContext->m_MultisampleFramebuffer->m_RenderTargetTexture->GetRenderTargetView().GetAddressOf(), m_DeviceContext->m_MultisampleFramebuffer->m_DepthStencilTexture->GetDepthStencilView().Get());

        m_GraphicsDevice->m_DeviceContextImmediate->ClearRenderTargetView(m_DeviceContext->m_MultisampleFramebuffer->m_RenderTargetTexture->GetRenderTargetView().Get(), color);
        m_GraphicsDevice->m_DeviceContextImmediate->ClearDepthStencilView(m_DeviceContext->m_MultisampleFramebuffer->m_DepthStencilTexture->GetDepthStencilView().Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        ///==============================
        RenderScene();
        m_Skybox->Render();
        DrawDebugWorld(m_Camera);
        Pass_Lines();

        m_DeviceContext->ResolveFramebuffer(m_DeviceContext->m_MultisampleFramebuffer.get(), m_DeviceContext->m_ResolveFramebuffer.get(), DXGI_FORMAT_R16G16B16A16_FLOAT);

        /// ==================================

        auto internalState = DX11_Utility::ToInternal(&m_SwapChain);

        // Clear the backbuffer to black for the new frame.
        float backgroundColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

        ID3D11RenderTargetView* RTV = internalState->m_RenderTargetView.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->OMSetRenderTargets(1, &RTV, 0); // Set depth as well here if it exists.    
        // Present is called from our Editor.

        TickPrimitives(deltaTime);
    }

    void Renderer::RenderScene()
    {
        m_GraphicsDevice->BindConstantBuffer(RHI_Shader_Stage::Vertex_Shader, &g_ConstantBuffers[CB_Types::CB_Entity], CB_GETBINDSLOT(ConstantBufferData_Entity), 0);

        /// Render Queue Feature?
        std::vector<std::shared_ptr<Entity>> sceneEntities = m_EngineContext->GetSubsystem<World>()->EntityGetAll();
        std::vector<Renderable> renderableComponents;
        for (auto& entity : sceneEntities)
        {
            if (entity->IsActive())
            {
                if (entity->HasComponent<Renderable>())
                {
                    renderableComponents.push_back(*entity->GetComponent<Renderable>());
                }
            }
        }

        // We can play around with the below flow by binding the right pipeline depending on the material extracted from the mesh's information.
        for (auto& renderableComponent : renderableComponents)
        {
            UpdateEntityConstantBuffer(renderableComponent.GetEntity());

            UINT offset = 0;
            UINT modelStride = 8 * sizeof(float);

            m_DeviceContext->BindVertexBuffer(renderableComponent.GetGeometryModel()->GetVertexBuffer());
            m_DeviceContext->BindIndexBuffer(renderableComponent.GetGeometryModel()->GetIndexBuffer());

            if (renderableComponent.GetMaterial())
            {
                UpdateMaterialConstantBuffer(renderableComponent.GetMaterial());
            }

            m_DeviceContext->DrawIndexed(renderableComponent.GetGeometryModel()->GetIndexBuffer());
        }
    }

    void Renderer::DrawDebugWorld(Entity* entity)
    {
        Stopwatch stopwatch("Debug World Pass", true);
        m_GraphicsDevice->BindPipelineState(&RendererGlobals::m_PSO_Object_Debug[DebugRenderer_Type::DebugRenderer_Grid], 0);
        // Bind Grid Pipeline
        if (Camera* camera = entity->GetComponent<Camera>())
        {
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
            m_GraphicsDevice->BindConstantBuffer(RHI_Shader_Stage::Vertex_Shader, &g_ConstantBuffers[CB_Types::CB_Misc], CB_GETBINDSLOT(ConstantBufferData_Misc), 0);
            m_GraphicsDevice->BindConstantBuffer(RHI_Shader_Stage::Pixel_Shader, &g_ConstantBuffers[CB_Types::CB_Misc], CB_GETBINDSLOT(ConstantBufferData_Misc), 0);

            uint32_t offset = 0;
            const uint32_t stride = sizeof(XMFLOAT4) + sizeof(XMFLOAT4);
            ID3D11Buffer* vertexBufferDebug = (ID3D11Buffer*)DX11_Utility::ToInternal(&gridBuffer)->m_Resource.Get();
            m_GraphicsDevice->m_DeviceContextImmediate->IASetVertexBuffers(0, 1, &vertexBufferDebug, &stride, &offset);
            m_GraphicsDevice->Draw(gridVertexCount, 0, 0);
        }
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

    // ======================================================================================

    void Renderer::InitializeShaders()
    {
        LoadShader(RHI_Shader_Stage::Vertex_Shader, m_ColorShaderVertex, "ColorVS.hlsl");
        LoadShader(RHI_Shader_Stage::Pixel_Shader, m_ColorShaderPixel, "ColorPS.hlsl");

        DX11_Utility::DX11_VertexShaderPackage* vertexInternal = static_cast<DX11_Utility::DX11_VertexShaderPackage*>(m_ColorShaderVertex.m_InternalState.get());
        m_ColorInputLayout = m_DeviceContext->CreateInputLayout(RHI_Vertex_Type::VertexType_PositionColor, vertexInternal->m_ShaderCode);

        AURORA_INFO(LogLayer::Graphics, "Successfully created Color Shaders.");
    }

    void Renderer::Pass_Lines()
    {
        Stopwatch stopwatch("Lines Pass", true);
        m_GraphicsDevice->BindPipelineState(&RendererGlobals::m_PSO_Object_Debug[DebugRenderer_Type::DebugRenderer_Grid], 0);

        const bool isLineDrawingEnabled = !m_Lines_DepthDisabled.empty() || !m_Lines_DepthEnabled.empty(); // Any kind of lines, physics, user debug or whatsoever.
        const bool drawLinesState = isLineDrawingEnabled;

        if (!drawLinesState)
        {
            return;
        }

        // Draw Lines
        {
            uint32_t lineVertexBufferSize = static_cast<uint32_t>(m_Lines_DepthEnabled.size());
            if (lineVertexBufferSize != 0)
            {
                m_Lines_VertexBuffer = m_DeviceContext->CreateVertexBuffer(RHI_Vertex_Type::VertexType_PositionColor, m_Lines_DepthEnabled);
                // Grow vertex buffer (if needed).
                if (lineVertexBufferSize > m_Lines_VertexBuffer->GetVertexCount() || m_Lines_VertexBuffer == nullptr)
                {
                    m_Lines_VertexBuffer = m_DeviceContext->CreateVertexBuffer(RHI_Vertex_Type::VertexType_PositionColor, m_Lines_DepthEnabled);
                }

                // Update vertex buffer.
                memcpy(m_Lines_VertexBuffer->Map(), m_Lines_DepthEnabled.data(), lineVertexBufferSize);
                m_Lines_VertexBuffer->Unmap();

                // Set render state.
                m_DeviceContext->BindRasterizerState(RasterizerState_Types::RasterizerState_CullBackWireframe);
                m_DeviceContext->BindPrimitiveTopology(RHI_Primitive_Topology::LineList);
                m_DeviceContext->BindInputLayout(m_ColorInputLayout.get());
                m_DeviceContext->BindVertexBuffer(m_Lines_VertexBuffer.get());

                ID3D11VertexShader* shaderInternalVS = static_cast<DX11_Utility::DX11_VertexShaderPackage*>(m_ColorShaderVertex.m_InternalState.get())->m_Resource.Get();
                m_GraphicsDevice->m_DeviceContextImmediate->VSSetShader(shaderInternalVS, nullptr, 0);
                ID3D11PixelShader* shaderInternalPS = static_cast<DX11_Utility::DX11_PixelShaderPackage*>(m_ColorShaderPixel.m_InternalState.get())->m_Resource.Get();
                m_GraphicsDevice->m_DeviceContextImmediate->PSSetShader(shaderInternalPS, nullptr, 0);

                m_GraphicsDevice->m_DeviceContextImmediate->Draw(m_Lines_VertexBuffer->GetVertexCount(), 0);
            }
        }
    }

    void Renderer::TickPrimitives(const float deltaTime)
    {
        // Remove lines which have expired.
        uint32_t end = static_cast<uint32_t>(m_Lines_DepthDisabled_Duration.size()); 
        for (uint32_t i = 0; i < end; i++)
        {
            m_Lines_DepthDisabled_Duration[i] -= deltaTime;

            if (m_Lines_DepthDisabled_Duration[i] <= 0.0f)
            {
                m_Lines_DepthDisabled_Duration.erase(m_Lines_DepthDisabled_Duration.begin() + i); // Remove corresponding line duration.
                m_Lines_DepthDisabled.erase(m_Lines_DepthDisabled.begin() + i); // Remove corresponding line.
                i--;
                end--;
            }
        }

        end = static_cast<uint32_t>(m_Lines_DepthEnabled_Duration.size());
        for (uint32_t i = 0; i < end; i++)
        {
            m_Lines_DepthEnabled_Duration[i] -= deltaTime;
            
            if (m_Lines_DepthEnabled_Duration[i] <= 0.0f)
            {
                m_Lines_DepthEnabled_Duration.erase(m_Lines_DepthEnabled_Duration.begin() + i);
                m_Lines_DepthEnabled.erase(m_Lines_DepthEnabled.begin() + i);
                i--;
                end--;
            }
        }
    }

    void Renderer::DrawLine(const XMFLOAT3& fromPoint, const XMFLOAT3& toPoint, const XMFLOAT4& fromColor, const XMFLOAT4& toColor, const float duration /* = 0.0f */, const bool depthEnabled /* = true */)
    {
        if (depthEnabled)
        {
            m_Lines_DepthEnabled.emplace_back(fromPoint, fromColor);
            m_Lines_DepthEnabled_Duration.emplace_back(duration);

            m_Lines_DepthEnabled.emplace_back(toPoint, toColor);
            m_Lines_DepthEnabled_Duration.emplace_back(duration);
        }
        else
        {
            m_Lines_DepthDisabled.emplace_back(fromPoint, fromColor);
            m_Lines_DepthDisabled_Duration.emplace_back(duration);

            m_Lines_DepthDisabled.emplace_back(toPoint, toColor);
            m_Lines_DepthDisabled_Duration.emplace_back(duration);
        }
    }
}