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
        FORMAT_D24_UNORM_S8_UINT,

        FORMAT_R16_TYPELESS,            // Depth (16-bit) / Shader Resource (16-bit)
        FORMAT_R32_TYPELESS,            // Depth (32-bit) / Shader Resource (32-bit)
        FORMAT_R24G8_TYPELESS,          // Depth (24-bit) + Stencil (8-bit) + Shader Resource (24-bit)

        FORMAT_R16G16_FLOAT,

        FORMAT_R32_UINT,

        FORMAT_R16_UINT,
        FORMAT_R8_UINT
    };

    inline uint32_t GetFormatStride(Format value)
    {
        switch (value)
        {
        case FORMAT_R32G32B32A32_FLOAT:
        case FORMAT_R32G32B32A32_UINT:
        case FORMAT_R32G32B32A32_SINT:
            return 16;

        case FORMAT_R32G32B32_FLOAT:
        case FORMAT_R32G32B32_UINT:
        case FORMAT_R32G32B32_SINT:
            return 12;

        case FORMAT_R16G16B16A16_FLOAT:
        case FORMAT_R16G16B16A16_UNORM:
        case FORMAT_R16G16B16A16_UINT:
        case FORMAT_R16G16B16A16_SNORM:
        case FORMAT_R16G16B16A16_SINT:
            return 8;

        case FORMAT_R32G32_FLOAT:
        case FORMAT_R32G32_UINT:
        case FORMAT_R32G32_SINT:
        case FORMAT_R32G8X24_TYPELESS:
        case FORMAT_D32_FLOAT_S8X24_UINT:
            return 8;

        case FORMAT_R10G10B10A2_UNORM:
        case FORMAT_R10G10B10A2_UINT:
        case FORMAT_R11G11B10_FLOAT:
        case FORMAT_R8G8B8A8_UNORM:
        case FORMAT_R16G16_FLOAT:
        case FORMAT_R32_TYPELESS:
        case FORMAT_R32_UINT:
        case FORMAT_R24G8_TYPELESS:
        case FORMAT_D24_UNORM_S8_UINT:
            return 4;

        case FORMAT_R16_TYPELESS:
        case FORMAT_R16_UINT:
            return 2;

        case FORMAT_R8_UINT:
            return 1;

        default:
            assert(0); // didn't catch format!
            break;
        }

        return 16;
    }

    enum IndexBuffer_Format
    {
        Format_16Bit,
        Format_32Bit
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
        Resource_Misc_Buffer_Allow_Raw_Views = 1 << 3,   // Contents are addressable by a byte offset. See: https://docs.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-advanced-stages-cs-resources#byte-address-buffer
        Resource_Misc_Buffer_Structured      = 1 << 4,
        Resource_Misc_Tiled                  = 1 << 5,
        Resource_Misc_Raytracing             = 1 << 6
    };

    enum Texture_Type
    {
        Texture1D,
        Texture2D,
        Texture3D
    };

    enum Image_Layout
    {
        Image_Layout_Undefined,                 // Invalid State
        Image_Layout_RenderTarget,              // Render Target, Write Enabled
        Image_Layout_DepthStencil,              // Depth Stencil, Write Enabled
        Image_Layout_DepthStencil_ReadOnly,     // Depth Stencil, Read Only
        Image_Layout_Shader_Resource,           // Shader Resource, Read Only
        Image_Layout_Shader_Resource_Compute,   // Shader Resource, Read Only, Non-Pixel Shader
        Image_Layout_Unordered_Access,          // Shader Resource, Write Enabled
        Image_Layout_Copy_Source,               // Copy From
        Image_Layout_Copy_Destination,          // Copy To
        Image_Layout_Shading_Rate_Source        // Shading Rate Control Per Tile
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

    enum Shader_Stage
    {
        Vertex_Shader,
        Pixel_Shader,
        Geometry_Shader,
        Compute_Shader,
        Hull_Shader,
        Domain_Shader,

        ShaderStage_Count
    };

    enum Primitive_Topology
    {
        Undefined,
        TriangleList,
        TriangleStrip,
        PointList,
        LineList,
        LineStrip,
        PatchList
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

    enum Fill_Mode
    {
        Fill_Wireframe,
        Fill_Solid
    };

    enum Cull_Mode
    {
        Cull_None,
        Cull_Front,
        Cull_Back
    };

    enum Depth_Write_Mask
    {
        Depth_Write_Mask_Zero,
        Depth_Write_Mask_All
    };

    enum Stencil_Operation
    {
        Stencil_Operation_Keep,
        Stencil_Operation_Zero,
        Stencil_Operation_Replace,
        Stencil_Operation_Increment_Saturation,
        Stencil_Operation_Decrement_Saturation,
        Stencil_Operation_Invert,
        Stencil_Operation_Increment,
        Stencil_Operation_Decrement
    };

    struct Depth_Stencil_Operation
    {
        Stencil_Operation m_StencilFailOperation = Stencil_Operation::Stencil_Operation_Keep;
        Stencil_Operation m_StencilDepthFailOperation = Stencil_Operation::Stencil_Operation_Keep;
        Stencil_Operation m_StencilPassOperation = Stencil_Operation::Stencil_Operation_Keep;
        ComparisonFunction m_StencilComparisonFunction = ComparisonFunction::Comparison_Never;
    };

    enum Blend_Operation
    {
        Blend_Operation_Add,
        Blend_Operation_Subtract,
        Blend_Operation_ReverseSubtract,
        Blend_Operation_Minimum,
        Blend_Operation_Maximum
    };

    enum Blend_Factor
    {
        Blend_Zero,
        Blend_One,
        Blend_Source_Color,
        Blend_Inverse_Source_Color,
        Blend_Source_Alpha,
        Blend_Inverse_Source_Alpha,
        Blend_Destination_Alpha,
        Blend_Inverse_Destination_Alpha,
        Blend_Destination_Color,
        Blend_Inverse_Destination_Color,
        Blend_Source_Alpha_Saturate,
        Blend_Blend_Factor,
        Blend_Inverse_Blend_Factor,
        Blend_Source1_Color,
        Blend_Inverse_Source1_Color,
        Blend_Source1_Alpha,
        Blend_Inverse_Source1_Alpha
    };

    enum Color_Write_Mask
    {
        Color_Write_Disabled     = 0,
        Color_Write_Enable_Red   = 1,
        Color_Write_Enable_Green = 2,
        Color_Write_Enable_Blue  = 4,
        Color_Write_Enable_Alpha = 5,
        Color_Write_Enable_All   = ((( Color_Write_Enable_Red | Color_Write_Enable_Green) | Color_Write_Enable_Blue) | Color_Write_Enable_Alpha)
    };

    enum Input_Classification
    {
        Input_Per_Vertex_Data,
        Input_Per_Instance_Data
    };

    // ===================================================================================================

    struct RHI_Viewport
    {
        float m_TopLeftX = 0.0f;
        float m_TopLeftY = 0.0f;
        float m_Width = 0.0f;
        float m_Height = 0.0f;
        float m_MinimumDepth = 0.0f;
        float m_MaximumDepth = 1.0f;
    };

    union ClearValue
    {
        float m_Color[4];
        struct ClearDepthStencil
        {
            float m_DepthValue;
            uint32_t m_StencilValue;
        } m_DepthStencil;
    };

    struct RHI_RasterizerState
    {
        Fill_Mode m_FillMode                        = Fill_Mode::Fill_Solid;
        Cull_Mode m_CullMode                        = Cull_Mode::Cull_None;
        bool m_IsFrontCounterClockwise              = false;
        int32_t m_DepthBias                         = 0;
        float m_DepthBiasClamp                      = 0.0f;
        float m_DepthBiasSlopeScaled                = 0.0f;
        bool m_IsDepthClippingEnabled               = false;
        bool m_IsMultisamplingEnabled               = false;
        bool m_IsAntialiasedLiningEnabled           = false;
        bool m_IsConservativeRasterizationEnabled   = false;
        uint32_t m_ForcedSampleCount                = 0;
    };

    struct RHI_DepthStencilState
    {
        bool m_IsDepthEnabled                        = false;
        Depth_Write_Mask m_DepthWriteMask            = Depth_Write_Mask::Depth_Write_Mask_Zero;
        ComparisonFunction m_DepthComparisonFunction = ComparisonFunction::Comparison_Never;
        bool m_IsStencilEnabled                      = false;
        uint8_t m_StencilReadMask                    = 0xff;
        uint8_t m_StencilWriteMask                   = 0xff;
        Depth_Stencil_Operation m_FrontFaceOperation;
        Depth_Stencil_Operation m_BackFaceOperation;
    };

    struct RHI_BlendState
    {
        bool m_IsAlphaToCoverageEnabled = false;
        bool m_IsIndependentBlendingEnabled = false;

        struct RenderTargetBlendState
        {
            bool m_IsBlendingEnabled               = false;
            Blend_Factor m_SourceBlendFactor       = Blend_Factor::Blend_Source_Alpha;
            Blend_Factor m_DestinationBlendFactor  = Blend_Factor::Blend_Inverse_Source_Alpha;
            Blend_Operation m_BlendOperation       = Blend_Operation::Blend_Operation_Add;
            Blend_Factor m_SourceBlendAlpha        = Blend_Factor::Blend_One;
            Blend_Factor m_DestinationBlendAlpha   = Blend_Factor::Blend_One;
            Blend_Operation m_BlendOperationAlpha  = Blend_Operation::Blend_Operation_Add;
            uint8_t m_RenderTargetWriteMask        = Color_Write_Mask::Color_Write_Enable_All;
        };

        RenderTargetBlendState m_RenderTarget[8];
    };

    struct RHI_InputLayout
    {
        static const uint32_t APPEND_ALIGNED_ELEMENT = 0xffffffff; // Automatically figure out AlignedByteOffset depending on format.

        struct Element
        {
            std::string m_SemanticName;                                                                         // Semantic name in Shader.
            uint32_t m_SemanticIndex = 0;                                                                       // Semantic index in Shader.
            Format m_Format = Format::FORMAT_UNKNOWN;                                                           // Semantic format in shader.
            uint32_t m_InputSlot = 0;                                                                           // Semantic Slot
            uint32_t m_AlignedByteOffset = APPEND_ALIGNED_ELEMENT;                                              // Byte Offset
            Input_Classification m_InputSlotClass = Input_Classification::Input_Per_Vertex_Data;                // Input Slot Type
        };

        std::vector<Element> m_Elements;
    };

    struct RHI_GPU_Buffer_Description
    {
        uint32_t m_ByteWidth = 0;
        Usage m_Usage = Usage::Default;
        uint32_t m_BindFlags = 0;
        uint32_t m_CPUAccessFlags = 0;
        uint32_t m_MiscFlags = 0;
        uint32_t m_StructureByteStride = 0;         // Needed for Typed and Structured buffer types.
        Format m_Format = Format::FORMAT_UNKNOWN;   // Only needed for Typed buffer.
    };

    struct RHI_Texture_Description
    {
        Texture_Type m_Type = Texture_Type::Texture2D;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        uint32_t m_Depth = 0;
        uint32_t m_ArraySize = 1;
        uint32_t m_MipLevels = 1;
        Format m_Format = Format::FORMAT_UNKNOWN;
        uint32_t m_SampleCount = 1;
        Usage m_Usage = Usage::Default;
        uint32_t m_BindFlags = 0;
        uint32_t m_CPUAccessFlags = 0;
        uint32_t m_MiscFlags = 0;
        ClearValue m_ClearValue = {};
        Image_Layout m_Layout = Image_Layout::Image_Layout_Shader_Resource;
    };

    struct RHI_Sampler_Description
    {
        Filter m_Filter = Filter::FILTER_MIN_MAG_MIP_POINT;
        Texture_Address_Mode m_AddressU = Texture_Address_Mode::Texture_Address_Clamp;
        Texture_Address_Mode m_AddressV = Texture_Address_Mode::Texture_Address_Clamp;
        Texture_Address_Mode m_AddressW = Texture_Address_Mode::Texture_Address_Clamp;
        ComparisonFunction m_ComparisonFunction = ComparisonFunction::Comparison_Never;

        uint32_t m_MaxAnisotropy = 0;
        float m_MipLOD_Bias = 0.0f;
        float m_MinLOD = 0.0f;
        float m_MaxLOD = FLT_MAX;
        float m_BorderColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    };

    struct RHI_SwapChain_Description
    {
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        uint32_t m_BufferCount = 2;   // The amount of backbuffers for the swapchain.
        Format m_Format = Format::FORMAT_R10G10B10A2_UNORM;
        bool m_IsFullscreen = false;
        bool m_IsVSyncEnabled = true;
        float m_ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    };

    enum Mapping_Flag
    {
        Flag_Empty = 0,
        Flag_Read  = 1 << 0,
        Flag_Write = 1 << 1
    };

    struct RHI_Mapping
    {
        uint32_t m_Flags = Mapping_Flag::Flag_Empty;
        size_t m_Offset = 0;
        size_t m_Size = 0;
        uint32_t m_RowPitch = 0;  // Output
        void* m_Data = nullptr;   // Output
    };

    enum Shader_Format
    {
        ShaderFormat_None,
        ShaderFormat_HLSL5,
        ShaderFormat_HLSL6,
        ShaderFormat_SPIRV
    };

    enum Shader_Model
    {
        ShaderModel_5_0,
        ShaderModel_6_0,
        ShaderModel_6_1,
        ShaderModel_6_2,
        ShaderModel_6_3,
        ShaderModel_6_4,
        ShaderModel_6_5
    };

    // Resources

    struct RHI_Subresource_Data
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

    struct RHI_GPU_Resource : public RHI_GraphicsDeviceInternal
    {
        GPU_Resource_Type m_Type = GPU_Resource_Type::Unknown_Type;

        inline bool IsTexture() const { return m_Type == GPU_Resource_Type::Texture; }
        inline bool IsBuffer() const { return m_Type == GPU_Resource_Type::Buffer; }
    };

    struct RHI_SwapChain : public RHI_GraphicsDeviceInternal
    {
        RHI_SwapChain_Description m_Description;

        const RHI_SwapChain_Description& GetDescription() const { return m_Description; }
    };

    struct RHI_Sampler : public RHI_GraphicsDeviceInternal
    {
        RHI_Sampler_Description m_Description;
        
        const RHI_Sampler_Description& RetrieveDescription() const { return m_Description; }
    };

    struct StaticSampler
    {
        RHI_Sampler m_Sampler;
        uint32_t m_Slot = 0;
    };

    struct RHI_Shader : public RHI_GraphicsDeviceInternal
    {
        Shader_Stage m_Stage = Shader_Stage::ShaderStage_Count;
        std::vector<StaticSampler> m_AutoSamplers;  // Ability to set static samplers without explict root signature.
    };

    struct RHI_GPU_Buffer : public RHI_GPU_Resource
    {
        RHI_GPU_Buffer_Description m_Description;

        const RHI_GPU_Buffer_Description& GetDescription() const { return m_Description; }
    };

    struct RHI_Texture : public RHI_GPU_Resource
    {
        RHI_Texture_Description m_Description;

        const RHI_Texture_Description& GetDescription() const { return m_Description; }
    };

    struct RHI_RenderPass_Attachment
    {
        enum RenderPass_Attachment_Type
        {
            AttachmentType_Render_Target,
            AttachmentType_Depth_Stencil,
            AttachmentType_Resolve,
            AttachmentType_Shading_Rate_Source
        } m_Type = RenderPass_Attachment_Type::AttachmentType_Render_Target;

        enum RenderPass_Load_Operation
        {
            LoadOperation_Load,
            LoadOperation_Clear,
            LoadOperation_DontCare
        } m_LoadOperation = RenderPass_Load_Operation::LoadOperation_Load;

        const RHI_Texture* m_Texture = nullptr;
        int m_Subresource = -1;

        enum RenderPass_Store_Operation
        {
            StoreOperation_Store,
            StoreOperation_DontCare
        } m_StoreOperation = RenderPass_Store_Operation::StoreOperation_Store;

        Image_Layout m_InitialLayout = Image_Layout::Image_Layout_Undefined;    // Layout before the render pass.
        Image_Layout m_SubpassLayout = Image_Layout::Image_Layout_Undefined;    // Layout within the render pass.
        Image_Layout m_FinalLayout = Image_Layout::Image_Layout_Undefined;      // Layout after the render pass.

        static RHI_RenderPass_Attachment RenderTarget(const RHI_Texture* resource = nullptr, RenderPass_Load_Operation loadOperation = RenderPass_Load_Operation::LoadOperation_Clear,
                                                      RenderPass_Store_Operation storeOperation = RenderPass_Store_Operation::StoreOperation_DontCare, Image_Layout initialLayout = Image_Layout::Image_Layout_Shader_Resource,
                                                      Image_Layout subpassLayout = Image_Layout::Image_Layout_RenderTarget, Image_Layout finalLayout = Image_Layout::Image_Layout_Shader_Resource)
        {
            RHI_RenderPass_Attachment attachment;
            attachment.m_Type = RenderPass_Attachment_Type::AttachmentType_Render_Target;
            attachment.m_Texture = resource;
            attachment.m_LoadOperation = loadOperation;
            attachment.m_StoreOperation = storeOperation;
            attachment.m_InitialLayout = initialLayout;
            attachment.m_SubpassLayout = subpassLayout;
            attachment.m_FinalLayout = finalLayout;

            return attachment;
        }

        static RHI_RenderPass_Attachment DepthStencil(const RHI_Texture* resource = nullptr, RenderPass_Load_Operation loadOperation = RenderPass_Load_Operation::LoadOperation_Load,
                                                      RenderPass_Store_Operation storeOperation = RenderPass_Store_Operation::StoreOperation_Store, Image_Layout initialLayout = Image_Layout::Image_Layout_DepthStencil,
                                                      Image_Layout subpassLayout = Image_Layout::Image_Layout_DepthStencil, Image_Layout finalLayout = Image_Layout::Image_Layout_DepthStencil)
        {
            RHI_RenderPass_Attachment attachment;
            attachment.m_Type = RenderPass_Attachment_Type::AttachmentType_Depth_Stencil;
            attachment.m_Texture = resource;
            attachment.m_LoadOperation = loadOperation;
            attachment.m_StoreOperation = storeOperation;
            attachment.m_InitialLayout = initialLayout;
            attachment.m_SubpassLayout = subpassLayout;
            attachment.m_FinalLayout = finalLayout;

            return attachment;
        }

        static RHI_RenderPass_Attachment Resolve(const RHI_Texture* resource = nullptr, Image_Layout initialLayout = Image_Layout::Image_Layout_Shader_Resource,
                                                 Image_Layout finalLayout = Image_Layout::Image_Layout_Shader_Resource)
        {
            RHI_RenderPass_Attachment attachment;
            attachment.m_Type = RenderPass_Attachment_Type::AttachmentType_Resolve;
            attachment.m_Texture = resource;
            attachment.m_InitialLayout = initialLayout;
            attachment.m_FinalLayout = finalLayout;

            return attachment;
        }

        static RHI_RenderPass_Attachment ShadingRateSource(const RHI_Texture* resource = nullptr, Image_Layout initialLayout = Image_Layout::Image_Layout_Shading_Rate_Source,
                                                           Image_Layout finalLayout = Image_Layout::Image_Layout_Shading_Rate_Source)
        {
            RHI_RenderPass_Attachment attachment;
            attachment.m_Type = RenderPass_Attachment_Type::AttachmentType_Shading_Rate_Source;
            attachment.m_Texture = resource;
            attachment.m_InitialLayout = initialLayout;
            attachment.m_SubpassLayout = Image_Layout::Image_Layout_Shading_Rate_Source;
            attachment.m_FinalLayout = finalLayout;

            return attachment;
        }
    };

    // Descriptions
    struct RHI_RenderPass_Description
    {
        enum RenderPass_Flag
        {
            Flag_Empty = 0,
            Flag_Allow_UAV_Writes = 1 << 0
        };

        uint32_t m_Flags = RenderPass_Flag::Flag_Empty;
        std::vector<RHI_RenderPass_Attachment> m_Attachments;
    };

    struct RHI_PipelineState_Description
    {
        const RHI_Shader* m_VertexShader = nullptr;
        const RHI_Shader* m_PixelShader = nullptr;

        const RHI_BlendState* m_BlendState = nullptr;
        const RHI_RasterizerState* m_RasterizerState = nullptr;
        const RHI_DepthStencilState* m_DepthStencilState = nullptr;
        const RHI_InputLayout* m_InputLayout = nullptr;
        Primitive_Topology m_PrimitiveTopology = Primitive_Topology::TriangleList;
        uint32_t m_SampleMask = 0xFFFFFFFF;
    };

    struct RHI_PipelineState : public RHI_GraphicsDeviceInternal
    {
        size_t m_Hash = 0;
        RHI_PipelineState_Description m_Description;

        const RHI_PipelineState_Description& GetDescription() const { return m_Description; }
    };

    struct RHI_RenderPass : public RHI_GraphicsDeviceInternal
    {
        size_t m_Hash = 0;
        RHI_RenderPass_Description m_Description;

        const RHI_RenderPass_Description& GetDescription() const { return m_Description; }
    };
}