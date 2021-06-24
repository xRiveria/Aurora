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
        FORMAT_R32G8X24_TYPELESS,      // 64-bit typeless, meaning we can use the bits freely. Thus, Depth (32-Bit) + Stencil (8-Bit) and RHI_Shader Resource (32-Bit).
        FORMAT_D32_FLOAT_S8X24_UINT,   // Depth (32-Bits), Stencil (8-Bits) and 24 bits unused.

        FORMAT_R10G10B10A2_UNORM,
        FORMAT_R10G10B10A2_UINT,
        FORMAT_R11G11B10_FLOAT,
        FORMAT_R8G8B8A8_UNORM,
    };

    enum Filter
    {
        FILTER_MIN_MAG_MIP_POINT,
        FILTER_MIN_MAG_POINT_MIP_LINEAR,
        FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,
        FILTER_MIN_POINT_MAG_MIP_LINEAR,
        FILTER_MIN_LINEAR_MAG_MIP_POINT,
        FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
        FILTER_MIN_MAG_LINEAR_MIP_POINT,
        FILTER_MIN_MAG_MIP_LINEAR,
        FILTER_ANISOTROPIC,
        FILTER_COMPARISON_MIN_MAG_MIP_POINT,
        FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR,
        FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT,
        FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR,
        FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT,
        FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
        FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
        FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
        FILTER_COMPARISON_ANISOTROPIC,
        FILTER_MINIMUM_MIN_MAG_MIP_POINT,
        FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR,
        FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
        FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR,
        FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT,
        FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
        FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT,
        FILTER_MINIMUM_MIN_MAG_MIP_LINEAR,
        FILTER_MINIMUM_ANISOTROPIC,
        FILTER_MAXIMUM_MIN_MAG_MIP_POINT,
        FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR,
        FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
        FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR,
        FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT,
        FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
        FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT,
        FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR,
        FILTER_MAXIMUM_ANISOTROPIC,
    };

    enum Usage
    {
        Default,
        Immutable,
        Dynamic,
        Staging
    };

    enum Bind_Flag
    {
        Bind_Vertex_Buffer      = 1 << 0,
        Bind_Index_Buffer       = 1 << 1,
        Bind_Constant_Buffer    = 1 << 2,
        Bind_Shader_Resource    = 1 << 3,
        Bind_Stream_Output      = 1 << 4,
        Bind_Render_Target      = 1 << 5,
        Bind_Depth_Stencil      = 1 << 6,
        Bind_Unordered_Access   = 1 << 7
    };

    enum CPU_Access
    {
        CPU_Access_Write        = 1 << 0,
        CPU_Access_Read         = 1 << 1
    };

    enum Resource_Misc_Flag
    {
        Resource_Misc_Shared                 = 1 << 0,
        Resource_Misc_TextureCube            = 1 << 1,
        Resource_Misc_Indirect_Args          = 1 << 2,
        Resource_Misc_Buffer_Allow_Raw_Views = 1 << 3,
        Resource_Misc_Buffer_Structured      = 1 << 4,
        Resource_Misc_Tilied                 = 1 << 5,
        Resource_Misc_Raytracing             = 1 << 6
    };

    enum Texture_Address_Mode
    {
        Texture_Address_Wrap,
        Texture_Address_Mirror,
        Texture_Address_Clamp,
        Texture_Address_Border
    };

    enum ComparisonFunction
    {
        Comparison_Never,
        Comparison_Less,
        Comparison_Equal,
        Comparison_Less_Equal,
        Comparison_Greater,
        Comparison_Not_Equal,
        Comparison_Greater_Equal,
        Comparison_Always
    };

    enum GraphicsDevice_Capability // Support for core feautues by our GPU.
    {
        Tesselation                                     = 1 << 0,
        Conservative_Rasterization                      = 1 << 1,
        Rasterizer_Ordered_Views                        = 1 << 2,
        UAV_Load_Format_Common                          = 1 << 3,     // Examples: R16G16B16A16_FLOAT, R8G8B8A8_UNORM and more common ones.
        UAV_Load_Format_R11G11B10_Float                 = 1 << 4,
        RenderTarget_And_Viewport_ArrayIndex_Without_GS = 1 << 5
    };

    enum ShaderStage
    {
        Vertex_Shader,
        Pixel_Shader,
        Geometry_Shader,
        Compute_Shader,
        Hull_Shader,
        Domain_Shader,

        ShaderStage_Count
    };

    enum Subresource_Type
    {
        ConstantBufferView,
        ShaderResourceView,
        UnorderedAccessView,
        RenderTargetView,
        DepthStencilView
    };

    enum class GPU_Resource_Type
    {
        Buffer,
        Texture,
        Unknown_Type
    };

    // Descriptions
    struct RHI_GPU_BufferDescription
    {
        uint32_t m_ByteWidth = 0;
        Usage m_Usage = Usage::Default;
        uint32_t m_BindFlags = 0;
        uint32_t m_CPUAccessFlags = 0;
        uint32_t m_MiscFlags = 0;
        uint32_t m_StructureByteStride = 0;         // Needed for Typed and Structured buffer types.
        Format m_Format = Format::FORMAT_UNKNOWN;   // Only needed for Typed buffer.
    };

    struct RHI_SwapChainDescription
    {
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        uint32_t m_BufferCount = 2;   // The amount of backbuffers for the swapchain.
        Format m_Format = Format::FORMAT_R10G10B10A2_UNORM;
        bool m_IsFullscreen = false;
        bool m_IsVSyncEnabled = true;
        float m_ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    };

    struct RHI_SamplerDescription
    {
        Filter m_Filter                 = Filter::FILTER_MIN_MAG_MIP_POINT;
        Texture_Address_Mode m_AddressU = Texture_Address_Mode::Texture_Address_Clamp;
        Texture_Address_Mode m_AddressV = Texture_Address_Mode::Texture_Address_Clamp;
        Texture_Address_Mode m_AddressW = Texture_Address_Mode::Texture_Address_Clamp;
        ComparisonFunction m_ComparisonFunction = ComparisonFunction::Comparison_Never;

        uint32_t m_MaxAnisotropy = 0;
        float m_MIPLOD_Bias = 0.0f;
        float m_MinLOD = 0.0f;
        float m_MaxLOD = FLT_MAX;
        float m_BorderColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    };

    // Resources

    struct SubresourceData
    {
        const void* m_SystemMemory = nullptr;
        uint32_t m_SystemMemoryPitch = 0;
        uint32_t m_SystemMemorySlicePitch = 0;
    };

    struct RHI_GraphicsDeviceInternal
    {
        std::shared_ptr<void> m_InternalState; // Actual implementation struct by each unique Graphics API. We will cast this to said struct accordingly.

        inline bool IsValid() const { return m_InternalState.get() != nullptr; }
    };

    struct RHI_SwapChain : public RHI_GraphicsDeviceInternal
    {
        RHI_SwapChainDescription m_Description;

        const RHI_SwapChainDescription& GetDescription() const { return m_Description; }
    };

    struct RHI_Sampler : public RHI_GraphicsDeviceInternal
    {
        RHI_SamplerDescription m_Description;
        
        const RHI_SamplerDescription& RetrieveDescription() const { return m_Description; }
    };

    struct StaticSampler
    {
        RHI_Sampler m_Sampler;
        uint32_t m_Slot = 0;
    };

    struct RHI_Shader : public RHI_GraphicsDeviceInternal
    {
        ShaderStage m_Stage = ShaderStage::ShaderStage_Count;
        std::vector<StaticSampler> m_AutoSamplers;  // Ability to set static samplers without explict root signature.
    };

    struct RHI_GPUResource : public RHI_GraphicsDeviceInternal
    {
        GPU_Resource_Type m_Type;

        inline bool IsTexture() const { return m_Type == GPU_Resource_Type::Texture; }
        inline bool IsBuffer() const { return m_Type == GPU_Resource_Type::Buffer; }
    };

    struct GPUBuffer : public RHI_GPUResource
    {
        RHI_GPU_BufferDescription m_Description;

        const RHI_GPU_BufferDescription& GetDescription() const { return m_Description; }
    };
}