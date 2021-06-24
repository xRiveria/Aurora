#pragma once
#include <memory>
#include <vector>
#include <string>

namespace Aurora
{
    enum Format
    {
        FORMAT_UNKNOWN,

        FORMAT_R32G32B32A32_FLOAT,
        FORMAT_R32G32B32A32_UINT,
        FORMAT_R32G32B32A32_SINT,

        FORMAT_R32G32B32_FLOAT,
        FORMAT_R32G32B32_UINT,
        FORMAT_R32G32B32_SINT,

        FORMAT_R16G16B16A16_FLOAT,
        FORMAT_R16G16B16A16_UNORM,
        FORMAT_R16G16B16A16_UINT,
        FORMAT_R16G16B16A16_SNORM,
        FORMAT_R16G16B16A16_SINT,

        FORMAT_R32G32_FLOAT,
        FORMAT_R32G32_UINT,
        FORMAT_R32G32_SINT,
        FORMAT_R32G8X24_TYPELESS,      // 64-bit typeless, meaning we can use the bits freely. Thus, Depth (32-Bit) + Stencil (8-Bit) and Shader Resource (32-Bit).
        FORMAT_D32_FLOAT_S8X24_UINT,   // Depth (32-Bits), Stencil (8-Bits) and 24 bits unused.

        FORMAT_R10G10B10A2_UNORM,
        FORMAT_R10G10B10A2_UINT,
        FORMAT_R11G11B10_FLOAT,
        FORMAT_R8G8B8A8_UNORM,
    };

    enum GraphicsDevice_Capability // Support for core feautres by our GPU.
    {
        Tesselation                                     = 1 << 0,
        Conservative_Rasterization                      = 1 << 1,
        Rasterizer_Ordered_Views                        = 1 << 2,
        UAV_Load_Format_Common                          = 1 << 3,     // Examples: R16G16B16A16_FLOAT, R8G8B8A8_UNORM and more common ones.
        UAV_Load_Format_R11G11B10_Float                 = 1 << 4,
        RenderTarget_And_Viewport_ArrayIndex_Without_GS = 1 << 5
    };

    struct SwapChainDescription
    {
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        uint32_t m_BufferCount = 2;   // The amount of backbuffers for the swapchain.
        Format m_Format = Format::FORMAT_R10G10B10A2_UNORM;
        bool m_IsFullscreen = false;
        bool m_IsVSyncEnabled = true;
        float m_ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    };

    // Resources
    struct GraphicsDeviceChild
    {
        std::shared_ptr<void> m_InternalState; // Actual implementation struct by each unique Graphics API. We will cast this to said struct accordingly.

        inline bool IsValid() const { return m_InternalState.get() != nullptr; }
    };

    struct SwapChain : public GraphicsDeviceChild
    {
        SwapChainDescription m_Description;

        const SwapChainDescription& GetDescription() const { return m_Description; }
    };
}