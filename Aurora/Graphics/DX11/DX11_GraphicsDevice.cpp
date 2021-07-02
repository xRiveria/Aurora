#include "Aurora.h"
#include "DX11_GraphicsDevice.h"
#include "../Window/WindowContext.h"

using namespace Microsoft::WRL;
using namespace Aurora::DX11_Utility;

namespace Aurora
{
    DX11_GraphicsDevice::DX11_GraphicsDevice(EngineContext* engineContext, bool isDebuggingEnabled)
    {
        m_EngineContext = engineContext;
        m_IsDebugDevicePresent = isDebuggingEnabled;

        uint32_t deviceCreationFlags = 0;   // See: https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_create_device_flag
        if (isDebuggingEnabled)
        {
            deviceCreationFlags |= D3D11_CREATE_DEVICE_DEBUG;
        }

        D3D_DRIVER_TYPE driverTypes[] =     // In order of priority.
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE
        };
        uint32_t numberOfDriverTypes = ARRAYSIZE(driverTypes);
        const char* deviceTypesToString[] = { "Hardware", "Warp", "Reference" };

        D3D_FEATURE_LEVEL featureLevels[] = // In order of priority. See: https://docs.microsoft.com/en-us/windows/win32/api/d3dcommon/ne-d3dcommon-d3d_feature_level.
        {
            D3D_FEATURE_LEVEL_11_1,         // We must make sure to explicitly specify level 11_1 as it is not included in the DX11's default features array.
            D3D_FEATURE_LEVEL_11_0
        };
        uint32_t numberOfFeatureLevels = ARRAYSIZE(featureLevels);

        for (uint32_t driverTypeIndex = 0; driverTypeIndex < numberOfDriverTypes; driverTypeIndex++)
        {
            m_DriverType = driverTypes[driverTypeIndex];
            if (BreakIfFailed(D3D11CreateDevice(nullptr, m_DriverType, nullptr, deviceCreationFlags, featureLevels, numberOfFeatureLevels, D3D11_SDK_VERSION, &m_Device, &m_FeatureLevel, &m_DeviceContextImmediate)))
            {
                AURORA_INFO("Successfully created DX11 Device. Device Type: %s, Feature Level: %s.", deviceTypesToString[driverTypeIndex], m_FeatureLevel == D3D_FEATURE_LEVEL_11_1 ? "11_1" : "11_0");
                break;
            }
        }

        ComPtr<IDXGIDevice2> pDXGIDevice2;
        BreakIfFailed(m_Device.As(&pDXGIDevice2));

        // Sets the number of frames that the system is allowed to queue for rendering.
        BreakIfFailed(pDXGIDevice2->SetMaximumFrameLatency(1)); // Ensure that DXGI does not queue more than 1 frame at a time. This reduces latency and ensures that the application will only render after each VSync, minimizing power consumption.

        ComPtr<IDXGIAdapter> pDXGIAdapter;
        BreakIfFailed(pDXGIDevice2->GetAdapter(&pDXGIAdapter));                                   // Retrieve our adapter from the device.
        BreakIfFailed(pDXGIAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&m_DXGIFactory));  // Retrieve our factory from the adapter. This factory is created implicitly on device creation.

        QueryFeatureSupport();
    }

    void DX11_GraphicsDevice::QueryFeatureSupport()
    {
        if (m_IsDebugDevicePresent)
        {
            /// Debugging stuff.
        }

        D3D11_QUERY_DESC queryDescription = {};
        queryDescription.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        for (auto& query : m_DisjointQueries)
        {
            BreakIfFailed(m_Device->CreateQuery(&queryDescription, &query));
        }

        D3D_FEATURE_LEVEL acquiredFeatureLevel = m_Device->GetFeatureLevel();
        if (acquiredFeatureLevel >= D3D_FEATURE_LEVEL_11_0)
        {
            m_Capabilities |= GraphicsDevice_Capability::Tesselation;
        }

        D3D11_FEATURE_DATA_D3D11_OPTIONS2 supportedFeatures2;
        BreakIfFailed(m_Device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS2, &supportedFeatures2, sizeof(supportedFeatures2)));
        if (supportedFeatures2.ConservativeRasterizationTier >= D3D11_CONSERVATIVE_RASTERIZATION_TIER_1) // See: https://docs.microsoft.com/en-us/windows/win32/direct3d11/conservative-rasterization
        {
            m_Capabilities |= GraphicsDevice_Capability::Conservative_Rasterization;
            AURORA_INFO("Conservation Rasterization is supported.");
        }

        if (supportedFeatures2.ROVsSupported == TRUE) // See: https://docs.microsoft.com/en-us/windows/win32/direct3d11/rasterizer-order-views
        {
            m_Capabilities |= GraphicsDevice_Capability::Rasterizer_Ordered_Views;
            AURORA_INFO("Rasterizer Order Views are supported.")
        }

        if (supportedFeatures2.TypedUAVLoadAdditionalFormats)
        {
            m_Capabilities |= GraphicsDevice_Capability::UAV_Load_Format_Common;
            AURORA_INFO("Unordered Access View (UAV) Type Load for common formats is supported.");

            D3D11_FEATURE_DATA_FORMAT_SUPPORT2 supportedFormat = {};
            supportedFormat.InFormat = DXGI_FORMAT_R11G11B10_FLOAT;
            BreakIfFailed(m_Device->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT2, &supportedFormat, sizeof(supportedFormat)));

            if ((supportedFormat.OutFormatSupport2 & D3D11_FORMAT_SUPPORT2_UAV_TYPED_LOAD) != 0)
            {
                m_Capabilities |= GraphicsDevice_Capability::UAV_Load_Format_R11G11B10_Float;
                AURORA_INFO("Unordered Access View (UAV) Type Load for format R11G11B10_Float is supported.");
            }
        }

        D3D11_FEATURE_DATA_D3D11_OPTIONS3 supportedFeatures3;
        BreakIfFailed(m_Device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS3, &supportedFeatures3, sizeof(supportedFeatures3)));

        if (supportedFeatures3.VPAndRTArrayIndexFromAnyShaderFeedingRasterizer == TRUE)
        {
            m_Capabilities |= GraphicsDevice_Capability::RenderTarget_And_Viewport_ArrayIndex_Without_GS;
            AURORA_INFO("VPAndRTArrayIndexFromAnyShaderFeedingRasterizer is supported.");
        }

        AURORA_INFO("Completed querying of device features.");
    }

    bool DX11_GraphicsDevice::CreateSwapChain(const RHI_SwapChain_Description* swapChainDescription, RHI_SwapChain* swapChain) const
    {
        auto internalState = std::static_pointer_cast<DX11_SwapChainPackage>(swapChain->m_InternalState);
        if (swapChain->m_InternalState == nullptr)
        {
            internalState = std::make_shared<DX11_SwapChainPackage>();
        }

        swapChain->m_InternalState = internalState;
        swapChain->m_Description = *swapChainDescription;

        if (internalState->m_SwapChain == nullptr)
        {
            // Create Swapchain.
            DXGI_SWAP_CHAIN_DESC1 swapChainCreationInfo = {};
            swapChainCreationInfo.Width = swapChainDescription->m_Width;
            swapChainCreationInfo.Height = swapChainDescription->m_Height;
            swapChainCreationInfo.Format = DX11_ConvertFormat(swapChainDescription->m_Format);
            swapChainCreationInfo.Stereo = false; // 3D Glasses?
            swapChainCreationInfo.SampleDesc.Count = 1;
            swapChainCreationInfo.SampleDesc.Quality = 0;
            swapChainCreationInfo.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainCreationInfo.BufferCount = swapChainDescription->m_BufferCount;
            swapChainCreationInfo.Flags = 0;
            swapChainCreationInfo.AlphaMode = DXGI_ALPHA_MODE_IGNORE;         // Ignore transparency behavior of the swap-chain backbuffer.
            swapChainCreationInfo.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // How to handle pixels in surface after calling present. If flip discard, DXGI will discard the contents of the backbuffer after you call Present.
            swapChainCreationInfo.Scaling = DXGI_SCALING_STRETCH;             // Resize behavior if the size of the back buffer is not equal to the target output.

            DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenCreationInfo = {};
            fullscreenCreationInfo.RefreshRate.Numerator = 60;
            fullscreenCreationInfo.RefreshRate.Denominator = 1;
            fullscreenCreationInfo.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; // If we know the native resolution of the display and want to make sure we do not initiate a mode change when transitioning to full screen, we must use Unspecified. Else, selecting the Centered or Stretched modes can result in a mode change
            fullscreenCreationInfo.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE; // The method the rasterizer uses to create an image on the surface. If set to progressive, the image is created from the first scanline to the last without skipping.
            fullscreenCreationInfo.Windowed = !swapChainDescription->m_IsFullscreen;

            if (!BreakIfFailed(m_DXGIFactory->CreateSwapChainForHwnd(m_Device.Get(), m_EngineContext->GetSubsystem<WindowContext>()->GetWindowHWND(0), &swapChainCreationInfo, &fullscreenCreationInfo, nullptr, &internalState->m_SwapChain)))
            {
                AURORA_ERROR("Failed to create Swapchain.");
                return false;
            }

            AURORA_INFO("Successfully created Swapchain.");
        }
        else // Swapchain already exists...
        {
            internalState->m_BackBuffer.Reset();
            internalState->m_RenderTargetView.Reset();

            if (!BreakIfFailed(internalState->m_SwapChain->ResizeBuffers(swapChainDescription->m_BufferCount, swapChainDescription->m_Width, swapChainDescription->m_Height, DX11_ConvertFormat(swapChainDescription->m_Format), 0)))
            {
                AURORA_ERROR("Failed to resize SwapChain.");
                return false;
            }

            AURORA_INFO("Successfully resized SwapChain.");
        }

        if (!BreakIfFailed(internalState->m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &internalState->m_BackBuffer)))
        {
            AURORA_ERROR("Failed to retrieve SwapChain backbuffer.");
            return false;
        }

        AURORA_INFO("Successfully retrieved SwapChain backbuffer.");

        if (!BreakIfFailed(m_Device->CreateRenderTargetView(internalState->m_BackBuffer.Get(), nullptr, &internalState->m_RenderTargetView)))
        {
            AURORA_ERROR("Failed to retrieve SwapChain Render Target View.");
            return false;
        }

        AURORA_INFO("Successfully created SwapChain Render Target View.");

        return true;
    }

    bool DX11_GraphicsDevice::CreateBuffer(const RHI_GPU_Buffer_Description* bufferDescription, const RHI_Subresource_Data* initialData, RHI_GPU_Buffer* buffer) const
    {
        std::shared_ptr<DX11_ResourcePackage> internalState = std::make_shared<DX11_ResourcePackage>();
        buffer->m_InternalState = internalState; 
        buffer->m_Type = GPU_Resource_Type::Buffer;

        D3D11_BUFFER_DESC bufferCreationInfo = {};
        bufferCreationInfo.ByteWidth = bufferDescription->m_ByteWidth;
        bufferCreationInfo.Usage = DX11_ConvertUsageFlags(bufferDescription->m_Usage);
        bufferCreationInfo.BindFlags = DX11_ParseBindFlags(bufferDescription->m_BindFlags);
        bufferCreationInfo.CPUAccessFlags = DX11_ParseCPUAccessFlags(bufferDescription->m_CPUAccessFlags);
        bufferCreationInfo.MiscFlags = DX11_ParseResourceMiscFlags(bufferDescription->m_MiscFlags);
        bufferCreationInfo.StructureByteStride = bufferDescription->m_StructureByteStride;

        D3D11_SUBRESOURCE_DATA data;
        if (initialData != nullptr)
        {
            data = DX11_ConvertSubresourceData(*initialData);
        }

        buffer->m_Description = *bufferDescription;
        if (BreakIfFailed(m_Device->CreateBuffer(&bufferCreationInfo, initialData == nullptr ? nullptr : &data, (ID3D11Buffer**)internalState->m_Resource.ReleaseAndGetAddressOf())))
        {
            AURORA_INFO("Successfully created Buffer.");

            // Create resource views if needed.
            if (bufferDescription->m_BindFlags & Bind_Flag::Bind_Shader_Resource)
            {
                /// Create subresource.
            }

            if (bufferDescription->m_BindFlags & Bind_Flag::Bind_Unordered_Access)
            {
                /// Create subresource.
            }

            return true;
        }

        return false;      
    }

    bool DX11_GraphicsDevice::CreateTexture(const RHI_Texture_Description* textureDescription, const RHI_Subresource_Data* initialData, RHI_Texture* texture) const
    {
        std::shared_ptr<DX11_TexturePackage> internalState = std::make_shared<DX11_TexturePackage>();
        texture->m_InternalState = internalState;
        texture->m_Type = GPU_Resource_Type::Texture;

        texture->m_Description = *textureDescription;

        std::vector<D3D11_SUBRESOURCE_DATA> data;
        if (initialData != nullptr) // Slicing support for texture arrays. See: https://image.slidesharecdn.com/sa2008modernopengl-1231549184153966-1/95/siggraph-asia-2008-modern-opengl-56-638.jpg?cb=1422672731
        {
            uint32_t dataCount = textureDescription->m_ArraySize * std::max(1U, textureDescription->m_MipLevels);
            data.resize(dataCount);
            for (uint32_t slice = 0; slice < dataCount; ++slice)
            {
                data[slice] = DX11_ConvertSubresourceData(initialData[slice]);
            }
        }

        switch (texture->m_Description.m_Type)
        {
            case Texture_Type::Texture1D:
            {
                D3D11_TEXTURE1D_DESC description = DX11_ConvertTextureDescription1D(&texture->m_Description);
                BreakIfFailed(m_Device->CreateTexture1D(&description, data.data(), (ID3D11Texture1D**)internalState->m_Resource.ReleaseAndGetAddressOf()));
            }
            break;

            case Texture_Type::Texture2D:
            {
                D3D11_TEXTURE2D_DESC description = DX11_ConvertTextureDescription2D(&texture->m_Description);
                BreakIfFailed(m_Device->CreateTexture2D(&description, data.data(), (ID3D11Texture2D**)internalState->m_Resource.ReleaseAndGetAddressOf()));
            }
            break;

            case Texture_Type::Texture3D:
            {
                D3D11_TEXTURE3D_DESC description = DX11_ConvertTextureDescription3D(&texture->m_Description);
                BreakIfFailed(m_Device->CreateTexture3D(&description, data.data(), (ID3D11Texture3D**)internalState->m_Resource.ReleaseAndGetAddressOf()));
            }
            break;

            default:
            {
                AURORA_ERROR("Invalid texture type specified.");
                return false;
            }
        }

        AURORA_INFO("Successfully created Texture.");

        if (texture->m_Description.m_MipLevels == 0)
        {
            texture->m_Description.m_MipLevels = (uint32_t)log2(std::max(texture->m_Description.m_Width, texture->m_Description.m_Height)) + 1; // +1 for base.
        }

        if (texture->m_Description.m_BindFlags & Bind_Flag::Bind_Render_Target)
        {
            
        }

        if (texture->m_Description.m_BindFlags & Bind_Flag::Bind_Depth_Stencil)
        {
            CreateSubresource(texture, Subresource_Type::DepthStencilView, 0, -1, 0, -1);
        }

        if (texture->m_Description.m_BindFlags & Bind_Flag::Bind_Shader_Resource)
        {
            CreateSubresource(texture, Subresource_Type::ShaderResourceView, 0, -1, 0, -1);
        }

        if (texture->m_Description.m_BindFlags & Bind_Unordered_Access)
        {
            CreateSubresource(texture, Subresource_Type::UnorderedAccessView, 0, -1, 0, -1);
        }

        return true;
    }

    bool DX11_GraphicsDevice::CreateSampler(const RHI_Sampler_Description* samplerDescription, RHI_Sampler* samplerState) const
    {
        std::shared_ptr<DX11_SamplerPackage> internalState = std::make_shared<DX11_SamplerPackage>();
        samplerState->m_InternalState = internalState;

        D3D11_SAMPLER_DESC samplerCreationDescription = {};
        samplerCreationDescription.Filter = DX11_ConvertFilter(samplerDescription->m_Filter);
        samplerCreationDescription.AddressU = DX11_ConvertTextureAddressMode(samplerDescription->m_AddressU);
        samplerCreationDescription.AddressV = DX11_ConvertTextureAddressMode(samplerDescription->m_AddressV);
        samplerCreationDescription.AddressW = DX11_ConvertTextureAddressMode(samplerDescription->m_AddressW);
        samplerCreationDescription.MipLODBias = samplerDescription->m_MipLOD_Bias;
        samplerCreationDescription.MaxAnisotropy = samplerDescription->m_MaxAnisotropy;
        samplerCreationDescription.ComparisonFunc = DX11_ConvertComparisonFunction(samplerDescription->m_ComparisonFunction);
        samplerCreationDescription.BorderColor[0] = samplerDescription->m_BorderColor[0];
        samplerCreationDescription.BorderColor[1] = samplerDescription->m_BorderColor[1];
        samplerCreationDescription.BorderColor[2] = samplerDescription->m_BorderColor[2];
        samplerCreationDescription.BorderColor[3] = samplerDescription->m_BorderColor[3];
        samplerCreationDescription.MinLOD = samplerDescription->m_MinLOD;
        samplerCreationDescription.MaxLOD = samplerDescription->m_MaxLOD;

        samplerState->m_Description = *samplerDescription;
        if (BreakIfFailed(m_Device->CreateSamplerState(&samplerCreationDescription, &internalState->m_Resource)))
        {
            AURORA_INFO("Successfully created Sampler State.");
            return true;
        }

        AURORA_ERROR("Failed to create Sampler State.");
        return false;
    }

    bool DX11_GraphicsDevice::CreateShader(Shader_Stage shaderStage, const void* shaderByteCode, size_t byteCodeLength, RHI_Shader* shader) const
    {
        shader->m_Stage = shaderStage;

        switch (shaderStage)
        {
            case Shader_Stage::Vertex_Shader:
            {
                std::shared_ptr<DX11_VertexShaderPackage> internalState = std::make_shared<DX11_VertexShaderPackage>();
                shader->m_InternalState = internalState;
                internalState->m_ShaderCode.resize(byteCodeLength);
                std::memcpy(internalState->m_ShaderCode.data(), shaderByteCode, byteCodeLength); // Copy the shader byte code into our package.

                BreakIfFailed(m_Device->CreateVertexShader(shaderByteCode, byteCodeLength, nullptr, &internalState->m_Resource));
                AURORA_INFO("Successfully created Vertex Shader.");
                return true;
            }
            break;

            case Shader_Stage::Domain_Shader:
            {
                /// Soon.
                return false;
            }
            break;

            case Shader_Stage::Hull_Shader:
            {
                /// Soon.
                return false;
            }
            break;

            case Shader_Stage::Geometry_Shader:
            {
                /// Soon.
                return false;
            }
            break;

            case Shader_Stage::Pixel_Shader:
            {
                std::shared_ptr<DX11_PixelShaderPackage> internalState = std::make_shared<DX11_PixelShaderPackage>();
                shader->m_InternalState = internalState;

                BreakIfFailed(m_Device->CreatePixelShader(shaderByteCode, byteCodeLength, nullptr, &internalState->m_Resource));
                AURORA_INFO("Successfully created Pixel Shader.");
                return true;
            }
            break;

            case Shader_Stage::Compute_Shader:
            {
                /// Soon.
                return false;
            }
            break;
        }

        AURORA_ERROR("Shader stage not found.");
        return false;
    }

    /* Solid! */
    bool DX11_GraphicsDevice::CreatePipelineState(const RHI_PipelineState_Description* description, RHI_PipelineState* pipelineStateObject) const
    {
        std::shared_ptr<DX11_PipelineStatePackage> internalState = std::make_shared<DX11_PipelineStatePackage>();
        pipelineStateObject->m_InternalState = internalState;
        pipelineStateObject->m_Description = *description;

        if (description->m_InputLayout != nullptr)
        {
            std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementDescriptions(description->m_InputLayout->m_Elements.size());
            for (size_t i = 0; i < inputElementDescriptions.size(); ++i)
            {
                inputElementDescriptions[i].SemanticName = description->m_InputLayout->m_Elements[i].m_SemanticName.c_str();
                inputElementDescriptions[i].SemanticIndex = description->m_InputLayout->m_Elements[i].m_SemanticIndex;
                inputElementDescriptions[i].Format = DX11_ConvertFormat(description->m_InputLayout->m_Elements[i].m_Format);
                inputElementDescriptions[i].InputSlot = description->m_InputLayout->m_Elements[i].m_InputSlot;
                inputElementDescriptions[i].AlignedByteOffset = description->m_InputLayout->m_Elements[i].m_AlignedByteOffset;
                if (inputElementDescriptions[i].AlignedByteOffset == RHI_InputLayout::APPEND_ALIGNED_ELEMENT)
                {
                    inputElementDescriptions[i].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
                }         
                inputElementDescriptions[i].InputSlotClass = DX11_ConvertInputClassification(description->m_InputLayout->m_Elements[i].m_InputSlotClass);
                inputElementDescriptions[i].InstanceDataStepRate = 0;

                if (inputElementDescriptions[i].InputSlotClass == D3D11_INPUT_PER_INSTANCE_DATA)
                {
                    inputElementDescriptions[i].InstanceDataStepRate = -1;
                }
            }

            AURORA_ASSERT(description->m_VertexShader != nullptr);
            DX11_VertexShaderPackage* vertexInternal = static_cast<DX11_Utility::DX11_VertexShaderPackage*>(description->m_VertexShader->m_InternalState.get());
            if (BreakIfFailed(m_Device->CreateInputLayout(inputElementDescriptions.data(), (UINT)inputElementDescriptions.size(), vertexInternal->m_ShaderCode.data(), vertexInternal->m_ShaderCode.size(), &internalState->m_InputLayout)))
            {
                AURORA_INFO("Successfully created Input Layout.");
            }
        }

        if (description->m_BlendState != nullptr)
        {
            D3D11_BLEND_DESC blendDescription;
            blendDescription.AlphaToCoverageEnable = description->m_BlendState->m_IsAlphaToCoverageEnabled;
            blendDescription.IndependentBlendEnable = description->m_BlendState->m_IsIndependentBlendingEnabled;

            for (int i = 0; i < 8; ++i)
            {
                blendDescription.RenderTarget[i].BlendEnable = description->m_BlendState->m_RenderTarget[i].m_IsBlendingEnabled;
                blendDescription.RenderTarget[i].SrcBlend = DX11_ConvertBlendFactor(description->m_BlendState->m_RenderTarget[i].m_SourceBlendFactor);
                blendDescription.RenderTarget[i].DestBlend = DX11_ConvertBlendFactor(description->m_BlendState->m_RenderTarget[i].m_DestinationBlendFactor);
                blendDescription.RenderTarget[i].BlendOp = DX11_ConvertBlendOperation(description->m_BlendState->m_RenderTarget[i].m_BlendOperation);
                blendDescription.RenderTarget[i].SrcBlendAlpha = DX11_ConvertBlendFactor(description->m_BlendState->m_RenderTarget[i].m_SourceBlendAlpha);
                blendDescription.RenderTarget[i].DestBlendAlpha = DX11_ConvertBlendFactor(description->m_BlendState->m_RenderTarget[i].m_DestinationBlendAlpha);
                blendDescription.RenderTarget[i].BlendOpAlpha = DX11_ConvertBlendOperation(description->m_BlendState->m_RenderTarget[i].m_BlendOperationAlpha);
                blendDescription.RenderTarget[i].RenderTargetWriteMask = DX11_ParseColorWriteMask(description->m_BlendState->m_RenderTarget[i].m_RenderTargetWriteMask);
            }

            if (BreakIfFailed(m_Device->CreateBlendState(&blendDescription, &internalState->m_BlendState)))
            {
                AURORA_INFO("Successfully created Blend State.");
            }
        }
        
        if (description->m_DepthStencilState != nullptr)
        {
            D3D11_DEPTH_STENCIL_DESC depthStencilDescription = {};
            depthStencilDescription.DepthEnable = description->m_DepthStencilState->m_IsDepthEnabled;
            depthStencilDescription.DepthWriteMask = DX11_ConvertDepthWriteMask(description->m_DepthStencilState->m_DepthWriteMask);
            depthStencilDescription.DepthFunc = DX11_ConvertComparisonFunction(description->m_DepthStencilState->m_DepthComparisonFunction);   
            depthStencilDescription.StencilEnable = description->m_DepthStencilState->m_IsStencilEnabled;
            depthStencilDescription.StencilReadMask = description->m_DepthStencilState->m_StencilReadMask;
            depthStencilDescription.StencilWriteMask = description->m_DepthStencilState->m_StencilWriteMask;
            depthStencilDescription.FrontFace.StencilDepthFailOp = DX11_ConvertStencilOperation(description->m_DepthStencilState->m_FrontFaceOperation.m_StencilDepthFailOperation);
            depthStencilDescription.FrontFace.StencilFailOp = DX11_ConvertStencilOperation(description->m_DepthStencilState->m_FrontFaceOperation.m_StencilFailOperation);
            depthStencilDescription.FrontFace.StencilFunc = DX11_ConvertComparisonFunction(description->m_DepthStencilState->m_FrontFaceOperation.m_StencilComparisonFunction);
            depthStencilDescription.FrontFace.StencilPassOp = DX11_ConvertStencilOperation(description->m_DepthStencilState->m_FrontFaceOperation.m_StencilPassOperation);
            depthStencilDescription.BackFace.StencilDepthFailOp = DX11_ConvertStencilOperation(description->m_DepthStencilState->m_BackFaceOperation.m_StencilDepthFailOperation);
            depthStencilDescription.BackFace.StencilFailOp = DX11_ConvertStencilOperation(description->m_DepthStencilState->m_BackFaceOperation.m_StencilFailOperation);
            depthStencilDescription.BackFace.StencilFunc = DX11_ConvertComparisonFunction(description->m_DepthStencilState->m_BackFaceOperation.m_StencilComparisonFunction);
            depthStencilDescription.BackFace.StencilPassOp = DX11_ConvertStencilOperation(description->m_DepthStencilState->m_BackFaceOperation.m_StencilPassOperation);
            
            if (BreakIfFailed(m_Device->CreateDepthStencilState(&depthStencilDescription, &internalState->m_DepthStencilState)))
            {
                AURORA_INFO("Successfully created Depth Stencil State.");
            }
        }
        
        if (description->m_RasterizerState != nullptr)
        {
            D3D11_RASTERIZER_DESC rasterizerDescription = {};

            rasterizerDescription.FillMode = DX11_ConvertFillMode(description->m_RasterizerState->m_FillMode);
            rasterizerDescription.CullMode = DX11_ConvertCullMode(description->m_RasterizerState->m_CullMode);
            rasterizerDescription.FrontCounterClockwise = description->m_RasterizerState->m_IsFrontCounterClockwise;
            rasterizerDescription.DepthBias = description->m_RasterizerState->m_DepthBias;
            rasterizerDescription.DepthBiasClamp = description->m_RasterizerState->m_DepthBiasClamp;
            rasterizerDescription.SlopeScaledDepthBias = description->m_RasterizerState->m_DepthBiasSlopeScaled;
            rasterizerDescription.DepthClipEnable = description->m_RasterizerState->m_IsDepthClippingEnabled;
            rasterizerDescription.ScissorEnable = true;
            rasterizerDescription.MultisampleEnable = description->m_RasterizerState->m_IsMultisamplingEnabled;
            rasterizerDescription.AntialiasedLineEnable = description->m_RasterizerState->m_IsAntialiasedLiningEnabled;

            /// Perhaps we can support conservative rasterization in due time.
            if (BreakIfFailed(m_Device->CreateRasterizerState(&rasterizerDescription, &internalState->m_RasterizerState)))
            {
                AURORA_INFO("Successfully created Rasterizer State.");
            }
        }

        return true;
    }
    
    void DX11_GraphicsDevice::BindPipelineState(const RHI_PipelineState* pipelineStateObject, RHI_CommandList commandList)
    {
        const RHI_PipelineState_Description& pipelineDescription = pipelineStateObject != nullptr ? pipelineStateObject->GetDescription() : RHI_PipelineState_Description();
        auto internalState = ToInternal(pipelineStateObject);

        ID3D11VertexShader* vertexShader = pipelineDescription.m_VertexShader == nullptr ? nullptr : static_cast<DX11_VertexShaderPackage*>(pipelineDescription.m_VertexShader->m_InternalState.get())->m_Resource.Get();
        m_DeviceContextImmediate->VSSetShader(vertexShader, nullptr, 0);

        ID3D11PixelShader* pixelShader = pipelineDescription.m_PixelShader == nullptr ? nullptr : static_cast<DX11_PixelShaderPackage*>(pipelineDescription.m_PixelShader->m_InternalState.get())->m_Resource.Get();
        m_DeviceContextImmediate->PSSetShader(pixelShader, nullptr, 0);

        ID3D11BlendState* blendState = pipelineDescription.m_BlendState == nullptr ? nullptr : internalState->m_BlendState.Get();
        const float newBlendFactor[4] = { m_BlendFactor[commandList].x, m_BlendFactor[commandList].y, m_BlendFactor[commandList].z, m_BlendFactor[commandList].w };
        m_DeviceContextImmediate->OMSetBlendState(blendState, newBlendFactor, pipelineDescription.m_SampleMask);

        ID3D11RasterizerState* rasterizerState = pipelineDescription.m_RasterizerState == nullptr ? nullptr : internalState->m_RasterizerState.Get();
        m_DeviceContextImmediate->RSSetState(rasterizerState);

        ID3D11DepthStencilState* depthStencilState = pipelineDescription.m_DepthStencilState == nullptr ? nullptr : internalState->m_DepthStencilState.Get();
        m_DeviceContextImmediate->OMSetDepthStencilState(depthStencilState, 0); /// 0 for now.

        ID3D11InputLayout* inputLayout = pipelineDescription.m_InputLayout == nullptr ? nullptr : internalState->m_InputLayout.Get();
        m_DeviceContextImmediate->IASetInputLayout(inputLayout);

        D3D11_PRIMITIVE_TOPOLOGY primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        switch (pipelineDescription.m_PrimitiveTopology)
        {
        case Primitive_Topology::TriangleList:
            primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            break;
        case Primitive_Topology::TriangleStrip:
            primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            break;
        case Primitive_Topology::PointList:
            primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
            break;
        case Primitive_Topology::LineList:
            primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
            break;
        case Primitive_Topology::LineStrip:
            primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
            break;
        case Primitive_Topology::PatchList:
            primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
            break;
        default:
            primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
            break;
        }

        m_DeviceContextImmediate->IASetPrimitiveTopology(primitiveTopology);
    }

    bool DX11_GraphicsDevice::CreateRenderPass(const RHI_RenderPass_Description* renderPassDescription, RHI_RenderPass* renderPass) const
    {
        return false;
    }

    int DX11_GraphicsDevice::CreateSubresource(RHI_Texture* texture, Subresource_Type type, uint32_t firstSlice, uint32_t sliceCount, uint32_t firstMip, uint32_t mipCount) const
    {
        DX11_TexturePackage* internalState = ToInternal(texture);

        switch (type)
        {
            case Subresource_Type::ShaderResourceView:
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescription = {};

                // Try to resolve the resource format.
                switch (texture->m_Description.m_Format)
                {
                    case Format::FORMAT_R16_TYPELESS:
                        shaderResourceViewDescription.Format = DXGI_FORMAT_R16_UNORM;
                        break;

                    case Format::FORMAT_R32_TYPELESS:
                        shaderResourceViewDescription.Format = DXGI_FORMAT_R32_FLOAT;
                        break;

                    case Format::FORMAT_R24G8_TYPELESS:
                        shaderResourceViewDescription.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
                        break;

                    case Format::FORMAT_R32G8X24_TYPELESS:
                        shaderResourceViewDescription.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
                        break;

                    default:
                        shaderResourceViewDescription.Format = DX11_ConvertFormat(texture->m_Description.m_Format);
                        break;
                }

                if (texture->m_Description.m_Type == Texture_Type::Texture1D)
                {
                    if (texture->m_Description.m_ArraySize > 1)
                    {
                        shaderResourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
                        shaderResourceViewDescription.Texture1DArray.FirstArraySlice = firstSlice;
                        shaderResourceViewDescription.Texture1DArray.ArraySize = sliceCount;
                        shaderResourceViewDescription.Texture1DArray.MostDetailedMip = firstMip;
                        shaderResourceViewDescription.Texture1DArray.MipLevels = mipCount;
                    }
                    else
                    {
                        shaderResourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
                        shaderResourceViewDescription.Texture1D.MostDetailedMip = firstMip;
                        shaderResourceViewDescription.Texture1D.MipLevels = mipCount;
                    }
                }
                else if (texture->m_Description.m_Type == Texture_Type::Texture2D)
                {
                    if (texture->m_Description.m_ArraySize > 1)
                    {
                        ///
                    }
                    else
                    {
                        if (texture->m_Description.m_SampleCount > 1)
                        {
                            shaderResourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
                        }
                        else
                        {
                            shaderResourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                            shaderResourceViewDescription.Texture2D.MostDetailedMip = firstMip;
                            shaderResourceViewDescription.Texture2D.MipLevels = mipCount;
                        }
                    }
                }
                else if (texture->m_Description.m_Type == Texture_Type::Texture3D)
                {
                    ///
                }

                ComPtr<ID3D11ShaderResourceView> shaderResourceView;
                if (BreakIfFailed(m_Device->CreateShaderResourceView(internalState->m_Resource.Get(), &shaderResourceViewDescription, &shaderResourceView))) // Note that the resource here is our texture. We are creating a shader resource view for it.
                {
                    AURORA_INFO("Successfully created Shader Resource View.");

                    if (!internalState->m_ShaderResourceView)
                    {
                        internalState->m_ShaderResourceView = shaderResourceView;
                        return -1;
                    }
                    internalState->m_Subresources_ShaderResourceView.push_back(shaderResourceView);
                    return int(internalState->m_Subresources_ShaderResourceView.size() - 1);
                }
                else
                {
                    AURORA_ERROR("Failed to create Shader Resource View.");
                    AURORA_ASSERT(0);
                }
            }
            break;

            case Subresource_Type::UnorderedAccessView:
            {
                D3D11_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDescription = {};

                // Try to resolve resource format.
                switch (texture->m_Description.m_Format)
                {
                    case Format::FORMAT_R16_TYPELESS:
                        unorderedAccessViewDescription.Format = DXGI_FORMAT_R16_UNORM;
                        break;

                    case Format::FORMAT_R32_TYPELESS:
                        unorderedAccessViewDescription.Format = DXGI_FORMAT_R32_FLOAT;
                        break;

                    case Format::FORMAT_R24G8_TYPELESS:
                        unorderedAccessViewDescription.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
                        break;

                    case Format::FORMAT_R32G8X24_TYPELESS:
                        unorderedAccessViewDescription.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
                        break;

                    default:
                        unorderedAccessViewDescription.Format = DX11_ConvertFormat(texture->m_Description.m_Format);
                        break;
                }

                if (texture->m_Description.m_Type == Texture_Type::Texture1D)
                {
                    if (texture->m_Description.m_ArraySize > 1)
                    {

                    }
                    else
                    {
                        unorderedAccessViewDescription.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1D;
                        unorderedAccessViewDescription.Texture1D.MipSlice = firstMip;
                    }
                }
                else if (texture->m_Description.m_Type == Texture_Type::Texture2D)
                {
                    if (texture->m_Description.m_ArraySize > 1)
                    {

                    }
                    else
                    {
                        unorderedAccessViewDescription.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
                        unorderedAccessViewDescription.Texture2D.MipSlice = firstMip;
                    }
                }
                else if (texture->m_Description.m_Type == Texture_Type::Texture3D)
                {

                }

                ComPtr<ID3D11UnorderedAccessView> unorderedAccessView;
                if (BreakIfFailed(m_Device->CreateUnorderedAccessView(internalState->m_Resource.Get(), &unorderedAccessViewDescription, &unorderedAccessView)))
                {
                    AURORA_INFO("Successfully created Unordered Access View.");

                    if (!internalState->m_UnorderedAccessView)
                    {
                        internalState->m_UnorderedAccessView = unorderedAccessView;
                        return -1;
                    }
                    internalState->m_Subresources_UnorderedAccessView.push_back(unorderedAccessView);
                    return int(internalState->m_Subresources_UnorderedAccessView.size() - 1);
                }
                else
                {
                    AURORA_ERROR("Failed to create Unordered Access View.");
                    AURORA_ASSERT(0);
                }
            }
            break;

            case Subresource_Type::ConstantBufferView:
            {
            }
            break;

            case Subresource_Type::DepthStencilView:
            {
                D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDescription = {};

                // Try to resolve the resource format.
                switch (texture->m_Description.m_Format)
                {
                    case Format::FORMAT_R16_TYPELESS:
                        depthStencilViewDescription.Format = DXGI_FORMAT_D16_UNORM;
                        break;
                        
                    case Format::FORMAT_R32_TYPELESS:
                        depthStencilViewDescription.Format = DXGI_FORMAT_D32_FLOAT;
                        break;

                    case Format::FORMAT_R24G8_TYPELESS:
                        depthStencilViewDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
                        break;

                    case Format::FORMAT_R32G8X24_TYPELESS:
                        depthStencilViewDescription.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
                        break;

                    default:
                        depthStencilViewDescription.Format = DX11_ConvertFormat(texture->m_Description.m_Format);
                        break;
                }

                if (texture->m_Description.m_Type == Texture_Type::Texture1D)
                {
                    if (texture->m_Description.m_ArraySize > 1)
                    {
                        depthStencilViewDescription.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1DARRAY;
                        depthStencilViewDescription.Texture1DArray.FirstArraySlice = firstSlice;
                        depthStencilViewDescription.Texture1DArray.ArraySize = sliceCount;
                        depthStencilViewDescription.Texture1DArray.MipSlice = firstMip;
                    }
                    else
                    {
                        depthStencilViewDescription.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1D;
                        depthStencilViewDescription.Texture1D.MipSlice = firstMip;
                    }
                }
                else if (texture->m_Description.m_Type == Texture_Type::Texture2D)
                {
                    if (texture->m_Description.m_ArraySize > 1)
                    {
                        if (texture->m_Description.m_SampleCount > 1)
                        {
                            depthStencilViewDescription.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
                            depthStencilViewDescription.Texture2DMSArray.FirstArraySlice = firstSlice;          // MS - Multi-Sampled
                            depthStencilViewDescription.Texture2DMSArray.ArraySize = sliceCount;
                        }
                        else
                        {
                            depthStencilViewDescription.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
                            depthStencilViewDescription.Texture2DArray.FirstArraySlice = firstSlice;
                            depthStencilViewDescription.Texture2DArray.ArraySize = sliceCount;
                            depthStencilViewDescription.Texture2DArray.MipSlice = firstMip;

                        }
                    }
                    else
                    {
                        if (texture->m_Description.m_SampleCount > 1)
                        {
                            depthStencilViewDescription.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
                        }
                        else
                        {
                            depthStencilViewDescription.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
                            depthStencilViewDescription.Texture2D.MipSlice = firstMip;
                        }
                    }
                }

                ComPtr<ID3D11DepthStencilView> depthStencilView;

                if (BreakIfFailed(m_Device->CreateDepthStencilView(internalState->m_Resource.Get(), &depthStencilViewDescription, &depthStencilView)))
                {
                    AURORA_INFO("Successfully created Depth Stencil View.");

                    if (!internalState->m_DepthStencilView)
                    {
                        internalState->m_DepthStencilView = depthStencilView;
                        return -1;
                    }

                    internalState->m_Subresources_DepthStencilView.push_back(depthStencilView);

                    return int(internalState->m_Subresources_DepthStencilView.size() - 1);
                }
                else
                {
                    AURORA_ERROR("Failed to create Depth Stencil View.");
                    AURORA_ASSERT(0);
                }
            }
            break;

            default:
            {
                break;
            }
        }

        return -1;
    }

    void DX11_GraphicsDevice::Map(const RHI_GPU_Resource* resource, RHI_Mapping* mapping)
    {
        DX11_ResourcePackage* internalState = ToInternal(resource);

        D3D11_MAPPED_SUBRESOURCE mapResult = {};
        D3D11_MAP mapType = D3D11_MAP_READ_WRITE;

        if (mapping->m_Flags & Mapping_Flag::Flag_Read)
        {
            if (mapping->m_Flags & Mapping_Flag::Flag_Write)
            {
                mapType = D3D11_MAP_READ_WRITE;
            }
            else
            {
                mapType = D3D11_MAP_READ;
            }
        }
        else if (mapping->m_Flags & Mapping_Flag::Flag_Write)
        {
            mapType = D3D11_MAP_WRITE_NO_OVERWRITE;
        }

        /// Should we use D3D11_MAP_FLAG_DO_NOT_WAIT?
        if (BreakIfFailed(m_DeviceContextImmediate->Map(internalState->m_Resource.Get(), 0, mapType, 0, &mapResult)))
        {
            // AURORA_INFO("Successfully mapped resource."); // Spam...
            mapping->m_Data = mapResult.pData;
            mapping->m_RowPitch = mapResult.RowPitch;
        }
        else
        {
            AURORA_ERROR("Failed to map resource.");
            mapping->m_Data = nullptr;
            mapping->m_RowPitch = 0;
        }
    }

    void DX11_GraphicsDevice::Unmap(const RHI_GPU_Resource* resource)
    {
        DX11_ResourcePackage* internalState = ToInternal(resource);
        m_DeviceContextImmediate->Unmap(internalState->m_Resource.Get(), 0);
    }

    RHI_CommandList DX11_GraphicsDevice::BeginCommandList(Queue_Type queue)
    {
        return RHI_CommandList();
    }

    void DX11_GraphicsDevice::SubmitCommandLists()
    {
    }

    void DX11_GraphicsDevice::BindResource(Shader_Stage shaderStage, const RHI_GPU_Resource* resource, uint32_t slot, RHI_CommandList commandList, int subresource)
    {
        if (resource != nullptr && resource->IsValid())
        {
            DX11_ResourcePackage* internalState = ToInternal(resource);
            ID3D11ShaderResourceView* shaderResourceView;

            if (subresource < 0)
            {
                shaderResourceView = internalState->m_ShaderResourceView.Get();
            }
            else
            {
                AURORA_ASSERT(internalState->m_Subresources_ShaderResourceView.size() > static_cast<size_t>(subresource)); // Invalid size.
                shaderResourceView = internalState->m_Subresources_ShaderResourceView[subresource].Get();
            }

            switch (shaderStage)
            {
                case Shader_Stage::Vertex_Shader:
                    m_DeviceContextImmediate->VSSetShaderResources(slot, 1, &shaderResourceView);
                    break;

                case Shader_Stage::Pixel_Shader:
                    m_DeviceContextImmediate->PSSetShaderResources(slot, 1, &shaderResourceView);
                    break;

                default:
                    break;
            }
        }
    }

    void DX11_GraphicsDevice::RenderPassBegin(const RHI_RenderPass* renderPass, RHI_CommandList commandList)
    {
    }

    void DX11_GraphicsDevice::RenderPassEnd(RHI_CommandList commandList)
    {
    }

    void DX11_GraphicsDevice::BindViewports(uint32_t numberOfViewports, const RHI_Viewport* viewports, RHI_CommandList commandList)
    {
    }

    void DX11_GraphicsDevice::BindSampler(Shader_Stage shaderStage, const RHI_Sampler* sampler, uint32_t slot, RHI_CommandList commandList)
    {
        if (sampler != nullptr && sampler->IsValid())
        {
            DX11_SamplerPackage* internalState = ToInternal(sampler);
            ID3D11SamplerState* sampler = internalState->m_Resource.Get();

            switch (shaderStage)
            {
                 case Shader_Stage::Vertex_Shader:
                     m_DeviceContextImmediate->VSSetSamplers(slot, 1, &sampler);
                     break;
                 case Shader_Stage::Pixel_Shader:
                     m_DeviceContextImmediate->PSSetSamplers(slot, 1, &sampler);
                     break;

                 /// Bind for other shader stages as well.
                 default:
                     break;
            }
        }
    }

    void DX11_GraphicsDevice::BindVertexBuffers(const RHI_GPU_Buffer* const* vertexBuffers, uint32_t slot, uint32_t count, const uint32_t* strides, const uint32_t* offsets, RHI_CommandList commandList)
    {
        AURORA_ASSERT(count <= 8);

        ID3D11Buffer* buffers[8] = {};
        for (uint32_t i = 0; i < count; ++i)
        {
            buffers[i] = vertexBuffers[i] != nullptr && vertexBuffers[i]->IsValid() ? (ID3D11Buffer*)ToInternal(vertexBuffers[i])->m_Resource.Get() : nullptr; // Remember that a resource can be a texture or a buffer.
        }

        m_DeviceContextImmediate->IASetVertexBuffers(slot, count, buffers, strides, (offsets != nullptr ? offsets : reinterpret_cast<const uint32_t*>(__nullBlob)));
    }

    void DX11_GraphicsDevice::BindConstantBuffer(Shader_Stage stage, const RHI_GPU_Buffer* buffer, uint32_t slot, RHI_CommandList commandList)
    {
        ID3D11Buffer* constantBuffer = buffer != nullptr && buffer->IsValid() ? (ID3D11Buffer*)ToInternal(buffer)->m_Resource.Get() : nullptr;

        // We have to decide which shader type to bind our constant buffer to.
        switch (stage)
        {
            case Shader_Stage::Vertex_Shader:
                m_DeviceContextImmediate->VSSetConstantBuffers(slot, 1, &constantBuffer);
                break;
                
            case Shader_Stage::Pixel_Shader:
                m_DeviceContextImmediate->PSSetConstantBuffers(slot, 1, &constantBuffer);
                break;

            /// Bind for other shader stages as well.
            default:
                break;
        }
    }

    void DX11_GraphicsDevice::ValidatePSO(RHI_CommandList commandList)
    {
        const RHI_PipelineState* pipelineStateObject = &m_PSO_Active;
        const RHI_PipelineState_Description& pipelineDescription = pipelineStateObject != nullptr ? pipelineStateObject->GetDescription() : RHI_PipelineState_Description();

        auto internalState = ToInternal(pipelineStateObject);

        ID3D11VertexShader* vertexShader = pipelineDescription.m_VertexShader == nullptr ? nullptr : static_cast<DX11_VertexShaderPackage*>(pipelineDescription.m_VertexShader->m_InternalState.get())->m_Resource.Get();
        m_DeviceContextImmediate->VSSetShader(vertexShader, nullptr, 0);

        ID3D11PixelShader* pixelShader = pipelineDescription.m_PixelShader == nullptr ? nullptr : static_cast<DX11_PixelShaderPackage*>(pipelineDescription.m_PixelShader->m_InternalState.get())->m_Resource.Get();    
        m_DeviceContextImmediate->PSSetShader(pixelShader, nullptr, 0);
           
        ID3D11BlendState* blendState = pipelineDescription.m_BlendState == nullptr ? nullptr : internalState->m_BlendState.Get();
        const float newBlendFactor[4] = { m_BlendFactor[commandList].x, m_BlendFactor[commandList].y, m_BlendFactor[commandList].z, m_BlendFactor[commandList].w };
        m_DeviceContextImmediate->OMSetBlendState(blendState, newBlendFactor, pipelineDescription.m_SampleMask);

        //ID3D11RasterizerState* rasterizerState = pipelineDescription.m_RasterizerState == nullptr ? nullptr : internalState->m_RasterizerState.Get();      
        //m_DeviceContextImmediate->RSSetState(rasterizerState);

       // ID3D11DepthStencilState* depthStencilState = pipelineDescription.m_DepthStencilState == nullptr ? nullptr : internalState->m_DepthStencilState.Get();     
       // m_DeviceContextImmediate->OMSetDepthStencilState(depthStencilState, 0); /// 0 for now.
        
        ID3D11InputLayout* inputLayout = pipelineDescription.m_InputLayout == nullptr ? nullptr : internalState->m_InputLayout.Get();
        m_DeviceContextImmediate->IASetInputLayout(inputLayout);

   
        D3D11_PRIMITIVE_TOPOLOGY primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        switch (pipelineDescription.m_PrimitiveTopology)
        {
            case Primitive_Topology::TriangleList:
                primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
                break;
            case Primitive_Topology::TriangleStrip:
                primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
                break;
            case Primitive_Topology::PointList:
                primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
                break;
            case Primitive_Topology::LineList:
                primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
                break;
            case Primitive_Topology::LineStrip:
                primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
                break;
            case Primitive_Topology::PatchList:
                primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
                break;
            default:
                primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
                break;
        }
        
        m_DeviceContextImmediate->IASetPrimitiveTopology(primitiveTopology);
    }

    void DX11_GraphicsDevice::UpdateBuffer(const RHI_GPU_Buffer* buffer, const void* data, RHI_CommandList commandList, int dataSize)
    {
        if (buffer->m_Description.m_Usage == Usage::Immutable)
        {
            AURORA_ERROR("Immutable GPU Buffers cannot be updated.");
            return;
        }

        if (!((int)buffer->m_Description.m_ByteWidth >= dataSize) || !(dataSize < 0))
        {
            AURORA_ERROR("Data size is too big for the buffer.")
            return;
        }

        if (dataSize == 0)
        {
            AURORA_ERROR("Requested a buffer update but a data size of 0 was provided. Is this intentional?");
        }

        DX11_ResourcePackage* internalState = ToInternal(buffer);

        dataSize = std::min((int)buffer->m_Description.m_ByteWidth, dataSize);

        if (buffer->m_Description.m_Usage == Usage::Dynamic)
        {
            D3D11_MAPPED_SUBRESOURCE mappedResource;
            if (!BreakIfFailed(m_DeviceContextImmediate->Map(internalState->m_Resource.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
            {
                AURORA_ERROR("Failed to map dynamic resource for access.");
                return;
            }
            memcpy(mappedResource.pData, data, (dataSize >= 0 ? dataSize : buffer->m_Description.m_ByteWidth));
            m_DeviceContextImmediate->Unmap(internalState->m_Resource.Get(), 0);
        }

        else if (buffer->m_Description.m_BindFlags & Bind_Flag::Bind_Constant_Buffer || dataSize < 0)
        {
            m_DeviceContextImmediate->UpdateSubresource(internalState->m_Resource.Get(), 0, nullptr, data, 0, 0);
        }

        else
        {
            // See: https://docs.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-updatesubresource
            D3D11_BOX box = {};
            box.left = 0;
            box.right = static_cast<uint32_t>(dataSize);
            box.top = 0;
            box.bottom = 1;
            box.front = 0;
            box.back = 1;
            m_DeviceContextImmediate->UpdateSubresource(internalState->m_Resource.Get(), 0, &box, data, 0, 0);
        }
    }

    void DX11_GraphicsDevice::Draw(uint32_t vertexCount, uint32_t startVertexLocation, RHI_CommandList commandList)
    {
        // ValidatePSO(commandList);

        m_DeviceContextImmediate->Draw(vertexCount, startVertexLocation);
    }

    void DX11_GraphicsDevice::DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, uint32_t baseVertexLocation, RHI_CommandList commandList)
    {
        ValidatePSO(commandList);

        m_DeviceContextImmediate->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }


}
