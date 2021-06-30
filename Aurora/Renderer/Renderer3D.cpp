#include "Aurora.h"
#include "Renderer3D.h"
#include "../Scene/World.h"

/// Remember that as we proceed with our rendering, the binding of resources and updating of constant buffers cannot be neglected!

namespace Aurora
{
    Renderer3D::Renderer3D(EngineContext* engineContext) : ISubsystem(engineContext)
    {
        ResizeResources();
    }

    void Renderer3D::ResizeResources()
    {
        DX11_GraphicsDevice* graphicsDevice = m_EngineContext->GetSubsystem<Renderer>()->m_GraphicsDevice.get();
        XMUINT2 internalResolution = GetInternalResolution();

        m_Camera = std::make_shared<Camera>(m_EngineContext);
        m_Camera->ComputePerspectiveMatrix(90.0f, (float)internalResolution.x / (float)internalResolution.y, 0.1f, 1000.0f);

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

            graphicsDevice->CreateTexture(&gBufferDescription, nullptr, &m_RenderTarget_GBuffer[GBuffer_Types::GBuffer_Color]);
            AURORA_INFO("GBuffer_Color Texture Creation Success.");

            // Normal, Roughness
            gBufferDescription.m_BindFlags = Bind_Flag::Bind_Render_Target | Bind_Flag::Bind_Shader_Resource;
            gBufferDescription.m_Format = Format::FORMAT_R8G8B8A8_UNORM;

            graphicsDevice->CreateTexture(&gBufferDescription, nullptr, &m_RenderTarget_GBuffer[GBuffer_Types::GBuffer_Normal_Roughness]);
            AURORA_INFO("GBuffer_Normal_Roughness Texture Creation Success.");

            // Velocity
            gBufferDescription.m_Format = Format::FORMAT_R16G16_FLOAT;

            graphicsDevice->CreateTexture(&gBufferDescription, nullptr, &m_RenderTarget_GBuffer[GBuffer_Types::GBuffer_Velocity]);
            AURORA_INFO("GBuffer_Velocity Texture Creation Success.")
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

            graphicsDevice->CreateTexture(&depthBufferDescription, nullptr, &m_DepthBuffer_Main);
            AURORA_INFO("DepthBuffer_Main Texture Creation Success.");
        }

        // Render Passes - GBuffer
        {
            // Depth PrePass (GBuffer Velocity)
            RHI_RenderPass_Description renderPassDescription;
            renderPassDescription.m_Attachments.push_back(RHI_RenderPass_Attachment::DepthStencil(&m_DepthBuffer_Main, RHI_RenderPass_Attachment::LoadOperation_Clear,
                                                          RHI_RenderPass_Attachment::StoreOperation_Store, Image_Layout::Image_Layout_DepthStencil_ReadOnly, 
                                                          Image_Layout::Image_Layout_DepthStencil, Image_Layout::Image_Layout_Shader_Resource));

            renderPassDescription.m_Attachments.push_back(RHI_RenderPass_Attachment::RenderTarget(&m_RenderTarget_GBuffer[GBuffer_Types::GBuffer_Velocity], RHI_RenderPass_Attachment::LoadOperation_DontCare));
            graphicsDevice->CreateRenderPass(&renderPassDescription, &m_RenderPass_DepthPrePass);
            AURORA_INFO("DepthPrePass Render Pass Creation Success.");

            // Main (GBuffer Color, Normal & Roughtness)
            renderPassDescription.m_Attachments.clear();
            renderPassDescription.m_Attachments.push_back(RHI_RenderPass_Attachment::RenderTarget(&m_RenderTarget_GBuffer[GBuffer_Types::GBuffer_Color], RHI_RenderPass_Attachment::LoadOperation_DontCare));
            renderPassDescription.m_Attachments.push_back(RHI_RenderPass_Attachment::RenderTarget(&m_RenderTarget_GBuffer[GBuffer_Types::GBuffer_Normal_Roughness], RHI_RenderPass_Attachment::LoadOperation_DontCare));
            renderPassDescription.m_Attachments.push_back(RHI_RenderPass_Attachment::DepthStencil(&m_DepthBuffer_Main, RHI_RenderPass_Attachment::LoadOperation_Load, RHI_RenderPass_Attachment::StoreOperation_Store,
                                                          Image_Layout::Image_Layout_Shader_Resource, Image_Layout::Image_Layout_DepthStencil_ReadOnly, Image_Layout::Image_Layout_DepthStencil_ReadOnly));

            graphicsDevice->CreateRenderPass(&renderPassDescription, &m_RenderPass_Main);
            AURORA_INFO("Main Render Pass Creation Success.");
        }
    }

    void Renderer3D::Tick(float deltaTime)
    {
        if (m_RenderTarget_GBuffer[GBuffer_Types::GBuffer_Color].m_Description.m_SampleCount != GetMSAASampleCount())
        {
            ResizeResources();
        }

        // Camera Frustrum Culling - Layers, Flags etc. For now, we render everything we see.
        // XMUINT2 internalResolution = GetInternalResolution();
        /// Update Per Frame Data. We will make do without this for now.

        m_Camera->Tick(deltaTime);

        Render();

        // End of a frame.
        m_EngineContext->GetSubsystem<Renderer>()->m_GraphicsDevice->SubmitCommandLists();
    }

    // We simulate thread execution scopes with {} for now. As we currently have no multi-threading job system, we will run everything on the main thread as it is.

    void Renderer3D::Render() const
    {
        DX11_GraphicsDevice* graphicsDevice = m_EngineContext->GetSubsystem<Renderer>()->m_GraphicsDevice.get();
        Renderer* renderer = m_EngineContext->GetSubsystem<Renderer>();
        RHI_CommandList commandList;
    
        // Update Main Camera for Opaque Render Pass
        commandList = graphicsDevice->BeginCommandList();
        {
            renderer->UpdateCameraConstantBuffer(*m_Camera.get(), commandList);

            // Opaque Objects
            graphicsDevice->RenderPassBegin(&m_RenderPass_Main, commandList);

            RHI_Viewport viewport;
            viewport.m_Width = (float)m_DepthBuffer_Main.GetDescription().m_Width;
            viewport.m_Height = (float)m_DepthBuffer_Main.GetDescription().m_Height;
            graphicsDevice->BindViewports(1, &viewport, commandList);

            // We will draw everything as it is for now.
            // renderer->DrawScene(RenderPass_Type::RenderPass_Main, commandList, 0);

            graphicsDevice->RenderPassEnd(commandList);
        }
    }
}