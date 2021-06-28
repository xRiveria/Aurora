#include "Aurora.h"
#include "Renderer3D.h"

namespace Aurora
{
    Renderer3D::Renderer3D(EngineContext* engineContext) : ISubsystem(engineContext)
    {

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

        // Render Passes
    }

    void Renderer3D::Tick(float deltaTime)
    {

    }

    void Renderer3D::Render() const
    {

    }
}