#pragma once
#include "EngineContext.h"
#include "../Graphics/RHI_Implementation.h"
#include "../Graphics/RHI_GraphicsDevice.h"
#include "../Window/WindowContext.h"
#include "Renderer.h"
#include <memory>

/*
    Highest level abstraction of our renderer.
*/

namespace Aurora
{
    class Renderer3D : public ISubsystem
    {
    public:
        Renderer3D(EngineContext* engineContext);

        void Tick(float deltaTime) override;
        void Render() const;

        void ResizeResources();

        // Retrievals
        const constexpr RHI_Texture* GetGBuffer_Read() const { return m_RenderTarget_GBuffer; }
        const constexpr RHI_Texture* GetGBuffer_Read(GBuffer_Types type) const { return &m_RenderTarget_GBuffer[type]; }
        const RHI_Texture* GetDepthStencilTexture() const { return &m_DepthBuffer_Main; }
        const uint32_t GetMSAASampleCount() const { return m_MSAA_SampleCount; }

    public:
        // Render Targets
        RHI_Texture m_RenderTarget_GBuffer[GBuffer_Types::GBuffer_Count];
        /// We can have a resolved version of the GBuffer for MSAA.

        // Depth Buffer
        RHI_Texture m_DepthBuffer_Main; // Used for depth-testing. Can be used for MSAA.

    private:
        std::shared_ptr<Camera> m_Camera; // This will be part of our ECS in the future. For now, it is a standalone camera.

        // Returns the native resolution in pixels. Use this for texture allocations, scissors and viewports.
        float m_ResolutionScale = 1.0f;
        XMUINT2 GetInternalResolution() const 
        { 
            return XMUINT2(static_cast<uint32_t>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowWidth(0)) * m_ResolutionScale, 
                           static_cast<uint32_t>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowHeight(0)) * m_ResolutionScale);
        }

        // Parameters
        uint32_t m_MSAA_SampleCount = 1;
    };
}