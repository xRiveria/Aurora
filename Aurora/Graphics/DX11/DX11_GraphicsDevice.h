#pragma once
#include "../RHI_Implementation.h"
#include "../RHI_Utilities.h"
#include <wrl/client.h> // For ComPtr

// Code here will be abstracted in due time to make way for multiple render APIs, such as using virtual overrides from the main graphics class.
// To Do: Clean up code by using proper referral methods for retrievals, combine functions etc.

namespace Aurora
{
    // To abstract to respective header.
    typedef uint8_t RHI_CommandList;
    static const RHI_CommandList g_CommandList_Count   = 32;
    static const RHI_CommandList g_CommandList_Invalid = g_CommandList_Count;

    class EngineContext;

    class DX11_GraphicsDevice
    {
    public:
        DX11_GraphicsDevice(EngineContext* engineContext, bool isDebuggingEnabled = false);
        void QueryFeatureSupport();

        bool CreateSwapChain(const RHI_SwapChain_Description* swapChainDescription, RHI_SwapChain* swapChain) const;
        bool CreateBuffer(const RHI_GPU_Buffer_Description* bufferDescription, const RHI_Subresource_Data* initialData, RHI_GPU_Buffer* buffer) const;
        bool CreateTexture(const RHI_Texture_Description* textureDescription, const RHI_Subresource_Data* initialData, RHI_Texture* texture) const; // Automatically creates the needed views via CreateSubresource.
        bool CreateSampler(const RHI_Sampler_Description* samplerDescription, RHI_Sampler* samplerState);
        bool CreateShader(ShaderStage shaderStage, const void* shaderByteCode, size_t byteCodeLength, RHI_Shader* shader) const;

        int CreateSubresource(RHI_Texture* texture, Subresource_Type type, uint32_t firstSlice, uint32_t sliceCount, uint32_t firstMip, uint32_t mipCount) const;
        void Map(const RHI_GPU_Resource* resource, RHI_Mapping* mappingDescription);
        void Unmap(const RHI_GPU_Resource* resource);

        void BindVertexBuffers(const RHI_GPU_Buffer* const* vertexBuffers, uint32_t slot, uint32_t count, const uint32_t* strides, const uint32_t* offsets, RHI_CommandList commandList);
        void BindConstantBuffer(ShaderStage stage, const RHI_GPU_Buffer* buffer, uint32_t slot, RHI_CommandList commandList);
        void BindPipelineState(const RHI_PipelineState* pipelineStateObject, RHI_CommandList commandList);
        void UpdateBuffer(const RHI_GPU_Buffer* buffer, const void* data, RHI_CommandList commandList, int dataSize = -1);
        void Draw(uint32_t vertexCount, uint32_t startVertexLocation, RHI_CommandList commandList);
        void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, uint32_t baseVertexLocation, RHI_CommandList commandList);

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