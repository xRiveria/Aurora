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

    bool DX11_GraphicsDevice::CreateSwapChain(const RHI_SwapChainDescription* swapChainDescription, RHI_SwapChain* swapChain) const
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
            swapChainCreationInfo.Stereo = false;
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

    bool DX11_GraphicsDevice::CreateBuffer(RHI_GPU_BufferDescription* bufferDescription, const SubresourceData* initialData, GPUBuffer* buffer) const
    {
        std::shared_ptr<DX11_Resource> internalState = std::make_shared<DX11_Resource>();
        buffer->m_InternalState = internalState; 
        buffer->m_Type = GPU_Resource_Type::Buffer;

        D3D11_BUFFER_DESC bufferCreationInfo = {};
        bufferCreationInfo.ByteWidth = bufferDescription->m_ByteWidth; // Size of our data.
        bufferCreationInfo.Usage = DX11_ConvertUsageFlags(bufferDescription->m_Usage);
        bufferCreationInfo.BindFlags = DX11_ParseBindFlags(bufferDescription->m_BindFlags);
        bufferCreationInfo.CPUAccessFlags = DX11_ParseCPUAccessFlags(bufferDescription->m_CPUAccessFlags);
        // bufferCreationInfo.MiscFlags = DX11_ParseResourceMiscFlags(bufferDescription->m_MiscFlags); /// ?
        bufferCreationInfo.StructureByteStride = bufferDescription->m_StructureByteStride;

        return true;
    }

    bool DX11_GraphicsDevice::CreateShader(ShaderStage shaderStage, const void* shaderByteCode, size_t byteCodeLength, RHI_Shader* shader) const
    {
        shader->m_Stage = shaderStage;

        switch (shaderStage)
        {
            case ShaderStage::Vertex_Shader:
            {
                std::shared_ptr<DX11_VertexShaderPackage> internalState = std::make_shared<DX11_VertexShaderPackage>();
                shader->m_InternalState = internalState;
                internalState->m_ShaderCode.resize(byteCodeLength);
                std::memcpy(internalState->m_ShaderCode.data(), shaderByteCode, byteCodeLength); // Copy the shader byte code into our package.

                BreakIfFailed(m_Device->CreateVertexShader(shaderByteCode, byteCodeLength, nullptr, &internalState->m_Resource));
                return true;
            }
            break;

            case ShaderStage::Domain_Shader:
            {
                /// Soon.
                return false;
            }
            break;

            case ShaderStage::Hull_Shader:
            {
                /// Soon.
                return false;
            }
            break;

            case ShaderStage::Geometry_Shader:
            {
                /// Soon.
                return false;
            }
            break;

            case ShaderStage::Pixel_Shader:
            {
                std::shared_ptr<DX11_PixelShaderPackage> internalState = std::make_shared<DX11_PixelShaderPackage>();
                shader->m_InternalState = internalState;

                BreakIfFailed(m_Device->CreatePixelShader(shaderByteCode, byteCodeLength, nullptr, &internalState->m_Resource));
                return true;
            }
            break;

            case ShaderStage::Compute_Shader:
            {
                /// Soon.
                return false;
            }
            break;
        }

        AURORA_ERROR("Shader stage not found.");
        return false;
    }
}