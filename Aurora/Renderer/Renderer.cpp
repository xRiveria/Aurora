#include "Aurora.h"
#include "Renderer.h"
#include "../Window/WindowContext.h"
#include "../Graphics/DX11/DX11_Utilities.h" //Temporary
#include "ShaderInternals.h"
#include "RendererResources.h"
#include "../Scene/Components/Light.h"
#include "../Scene/Components/Mesh.h"
#include "../Scene/Components/Material.h"

namespace Aurora
{
    Renderer::Renderer(EngineContext* engineContext) : ISubsystem(engineContext), m_WeatherSystem(engineContext)
    {

    }

    Renderer::~Renderer()
    {

    }

    bool Renderer::Initialize()
    {
        m_GraphicsDevice = std::make_shared<DX11_GraphicsDevice>(m_EngineContext, true);

        // Importers
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

    bool Renderer::PostInitialize()
    {
        m_Importer_Model->LoadModel_OBJ("../Resources/Models/Hollow_Knight/v3.obj");  // New way of adding models.
        m_Importer_Model->LoadModel_OBJ("../Resources/Models/Sword/weapon1.obj");

        return true;
    }

    void Renderer::UpdateCameraConstantBuffer(const std::shared_ptr<Entity>& camera, Entity* meshEntity, RHI_CommandList commandList)
    {
        ConstantBufferData_Camera constantBuffer;

        if (meshEntity != nullptr)
        {
            XMStoreFloat4x4(&constantBuffer.g_ObjectWorldMatrix, meshEntity->m_Transform);
        }
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

        UpdateCameraConstantBuffer(m_Camera, 0, 0);
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
        
        DrawSky();
        RenderMeshes();
        DrawDebugWorld(m_Camera.get());

        /// ==================================
        auto internalState = DX11_Utility::ToInternal(&m_SwapChain);

         // Clear the backbuffer to black for the new frame.
        float backgroundColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

        ID3D11RenderTargetView* RTV = internalState->m_RenderTargetView.Get();   
        m_GraphicsDevice->m_DeviceContextImmediate->OMSetRenderTargets(1, &RTV, 0); // Set depth as well here if it exists.    
        // Present is called from our Editor.
    }

    void Renderer::RenderMeshes()
    {
        World* world = m_EngineContext->GetSubsystem<World>();

        // All meshes that exist in the scene.
        std::vector<Mesh*> meshComponents;
        for (int i = 0; i < world->EntityGetAll().size(); i++)
        {
            Entity* entity = world->EntityGetAll()[i].get();
            if (entity->HasComponent<Mesh>())
            {
                meshComponents.push_back(entity->GetComponent<Mesh>());
            }
        }

        for (Mesh* mesh : meshComponents)
        {;
            UpdateCameraConstantBuffer(m_Camera, mesh->GetEntity(), 0);  /// We will be using this to update the entity's world matrix for now.

            // Bind Index Buffer.
            m_GraphicsDevice->BindIndexBuffer(&mesh->m_Index_Buffer, mesh->GetIndexFormat(), 0, 0);
            
            const RHI_GPU_Buffer* vertexBuffers[] =
            {
                &mesh->m_Vertex_Buffer_Position,
                &mesh->m_Vertex_Buffer_UV0,
                &mesh->m_Vertex_Buffer_UV1,
                &mesh->m_Vertex_Buffer_Color,
                &mesh->m_Vertex_Buffer_Tangent
            };

            uint32_t strides[] =
            {
                sizeof(Vertex_Position),
                sizeof(Vertex_TexCoord),
                sizeof(Vertex_TexCoord),
                sizeof(Vertex_Color),
                sizeof(Vertex_Tangent)
            };

            uint32_t offsets[] =
            {
                0,
                0,
                0,
                0,
                0,
            };

            static_assert(ARRAYSIZE(vertexBuffers) == Object_VertexInput::InputSlot_Count, "This layout must conform to Object_VertexInput enum.");
            static_assert(ARRAYSIZE(vertexBuffers) == ARRAYSIZE(strides), "Mismatch between vertex buffers and strides.");
            static_assert(ARRAYSIZE(vertexBuffers) == ARRAYSIZE(offsets), "Mismatch between vertex buffers and offsets.");

            m_GraphicsDevice->BindVertexBuffers(vertexBuffers, 0, ARRAYSIZE(vertexBuffers), strides, offsets, 0);
            m_GraphicsDevice->BindPipelineState(&RendererGlobals::m_PSO_Object, 0);

            m_GraphicsDevice->m_DeviceContextImmediate->Draw((UINT)mesh->m_Vertex_Positions.size(), 0);

            for (const Mesh::MeshSubset& subset : mesh->m_Subsets)
            {
                if (subset.m_Index_Count == 0) // Empty Subset.
                {
                    AURORA_ERROR("empty!");
                    continue;
                }

                const Material& material = *subset.m_Material_Entity->GetComponent<Material>();

                /// Check material flags that we are indeed in the "right" rendering pass for the material's intended object. Ignore since we don't care about filtering at the moment.
                
                const RHI_PipelineState* pipelineStateObject = nullptr;
                // const RHI_PipelineState* pipelineStateObject_Backside = nullptr;  // Only when seperate backside rendering is required (transparent double-sided).

                /// If Is Wire Render.
                /// If Is Terrain.
                /// If is Custom Shader.
                // const BlendMode_Types blendMode = material.GetBlendMode();
                // ObjectRendering_DoubleSided isDoubleSided = (mesh->IsDoubleSided() || material.IsDoubleSided()) ? ObjectRendering_DoubleSided::ObjectRendering_DoubleSided_Enabled : ObjectRendering_DoubleSided::ObjectRendering_DoubleSided_Disabled;

                // Actually bind pipeline state based on our object shaders. For now, we're using a standard object pipeline.
                pipelineStateObject = &RendererGlobals::m_PSO_Object;
                AURORA_ASSERT(pipelineStateObject->IsValid());

                /// Rebind if double sided.

                if (pipelineStateObject == nullptr || !pipelineStateObject->IsValid())
                {
                    continue;
                }

                /// Bind Stencil Reference.
                /// Bind Shading Rate - DX12 Only.
                /// If Bindless, Use Push Constants - Vulkan.

                m_GraphicsDevice->BindConstantBuffer(Shader_Stage::Vertex_Shader, &material.m_ConstantBuffer, CB_GETBINDSLOT(ConstantBufferData_Material), 0);
                m_GraphicsDevice->BindConstantBuffer(Shader_Stage::Pixel_Shader, &material.m_ConstantBuffer, CB_GETBINDSLOT(ConstantBufferData_Material), 0);

                // Bind all material textures.
                const RHI_GPU_Resource* materialTextures[Texture_Slot::Texture_Slot_Count];
                material.WriteTextures(materialTextures, ARRAYSIZE(materialTextures));
                m_GraphicsDevice->BindResources(Shader_Stage::Pixel_Shader, materialTextures, TEXSLOT_RENDERER_BASECOLOR_MAP, ARRAYSIZE(materialTextures), 0);

                /// If Tesselation.
                /// If Terrain.

                // if (pipelineStateObject_Backside != nullptr)
                // {
                //    m_GraphicsDevice->BindPipelineState(pipelineStateObject_Backside, 0);
                //    m_GraphicsDevice->DrawIndexed(subset.m_Index_Count, subset.m_Index_Offset, 0, 0);
                // }

                m_GraphicsDevice->BindPipelineState(pipelineStateObject, 0);
                m_GraphicsDevice->DrawIndexed(subset.m_Index_Count, subset.m_Index_Offset, 0, 0);
            }           
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

    void Renderer::DrawSky()
    {
        if (m_WeatherSystem.m_SkyMapTexture != nullptr)  // Means we have a custom skymap avaliable.
        {
            m_GraphicsDevice->BindPipelineState(&RendererGlobals::m_PSO_Object_Sky[SkyRender_Type::SkyRender_Static], 0);
            m_GraphicsDevice->BindResource(Shader_Stage::Pixel_Shader, &m_WeatherSystem.m_SkyMapTexture->m_Texture, TEXSLOT_GLOBAL_ENVIRONMENTAL_MAP, 0);
        }
        else
        {
            m_GraphicsDevice->BindPipelineState(&RendererGlobals::m_PSO_Object_Sky[SkyRender_Type::SkyRender_Dynamic], 0);

            // m_GraphicsDevice->BindResource(Shader_Stage::Pixel_Shader, &RendererGlobals::g_Textures[Texture_Types::TextureType_2D_SkyAtmosphere_Sky_View_LUT], TEXSLOT_SKY_VIEW_LUT, 0);
            // m_GraphicsDevice->BindResource(Shader_Stage::Pixel_Shader, &RendererGlobals::g_Textures[Texture_Types::TextureType_2D_SkyAtmosphere_Transmittance_LUT], TEXSLOT_TRANSMITTANCE_LUT, 0);
            // m_GraphicsDevice->BindResource(Shader_Stage::Pixel_Shader, &RendererGlobals::g_Textures[Texture_Types::TextureType_2D_SkyAtmosphere_Multiscattered_Luminance_LUT], TEXSLOT_MULTISCATTERING_LUT, 0);
        }

        BindConstantBuffers(Shader_Stage::Vertex_Shader, 0);
        BindConstantBuffers(Shader_Stage::Pixel_Shader, 0);

        m_GraphicsDevice->Draw(3, 0, 0);
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