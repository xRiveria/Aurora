#pragma once
#include "../RHI_Implementation.h"
#include "../RHI_Utilities.h"
#include <wrl/client.h> // For ComPtr

// Code here will be abstracted in due time to make way for multiple render APIs.

namespace Aurora
{
    class EngineContext;

    class DX11_GraphicsDevice
    {
    public:
        DX11_GraphicsDevice(EngineContext* engineContext, bool isDebuggingEnabled = false);
        void QueryFeatureSupport();
        bool CreateSwapChain(const SwapChainDescription* swapChainDescription, SwapChain* swapChain) const;


    public: 
        /// Will be temporarily public.

        static const uint32_t BUFFER_COUNT = 2; // Amount of backbuffers for our swapchain.
        uint32_t m_Capabilities; // Flag for certain core features supported by our GPU.

        D3D_DRIVER_TYPE m_DriverType;
        D3D_FEATURE_LEVEL m_FeatureLevel;
        Microsoft::WRL::ComPtr<IDXGIFactory2> m_DXGIFactory;
        Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_DeviceContextImmediate;
        Microsoft::WRL::ComPtr<ID3D11Query> m_DisjointQueries[BUFFER_COUNT + 3];


        bool m_IsDebugDevicePresent = false; // Or known as debug layer in DX11.

    private:
        EngineContext* m_EngineContext;
    };
}