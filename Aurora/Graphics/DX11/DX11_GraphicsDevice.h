#pragma once
#include "../RHI_Implementation.h"
#include "../RHI_Utilities.h"
#include <wrl/client.h> // For ComPtr

// Code here will be abstracted in due time to make way for multiple render APIs, such as using virtual overrides from the main graphics class.
// To Do: Clean up code by using proper referral methods for retrievals, combine functions etc.

using namespace DirectX;

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

        /*
            Rendering commands that expect a CommandList as a parameter are not executed immediately. They will be recorded into command lists and submitted to the GPU for execution upon calling 
            the SubmitCommandLists() function. The CommandList is a simple handle that associates rendering commands to a CPU execution timeline. The CommandList is not threadsafe, so every 
            CommandList can be used by a single CPU thread at a time to record commands.

            In a multithreading scenario, every CPU thread should have its own command list. Command lists can also be retrieved from our graphics device by calling GraphicsDevice::BeginCommandList(). that 
            will return a CommandList handle that is free to be used from that point by the calling thread. All such handles will be in use until SubmitCommandLists() or PresentEnd() was called, where 
            GPU submission takes place. The command lists will be submitted in the order they were retrieved with GraphicsDevice::BeginCommandList. The order of submission correlates with the order of 
            actual GPU execution. 
        */

        void BindSampler(ShaderStage shaderStage, const RHI_Sampler* sampler, uint32_t slot, RHI_CommandList commandList);
        void BindVertexBuffers(const RHI_GPU_Buffer* const* vertexBuffers, uint32_t slot, uint32_t count, const uint32_t* strides, const uint32_t* offsets, RHI_CommandList commandList);
        void BindConstantBuffer(ShaderStage stage, const RHI_GPU_Buffer* buffer, uint32_t slot, RHI_CommandList commandList);
        void BindPipelineState(const RHI_PipelineState* pipelineStateObject, RHI_CommandList commandList);
        void UpdateBuffer(const RHI_GPU_Buffer* buffer, const void* data, RHI_CommandList commandList, int dataSize = -1);
        void Draw(uint32_t vertexCount, uint32_t startVertexLocation, RHI_CommandList commandList);
        void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, uint32_t baseVertexLocation, RHI_CommandList commandList);

        void CommitAllocations(RHI_CommandList commandList);

    public: 
        /// Will be temporarily public.
        static const uint32_t BUFFER_COUNT = 2;     // Amount of backbuffers for our swapchain.
        uint32_t m_Capabilities;                    // Flag for certain core features supported by our GPU.
        const void* const __nullBlob[128] = {};     // This is initialized to nullptrs and is used to unbind resources.

        XMFLOAT4 m_BlendFactor[g_CommandList_Count];

        // Keep a record of previously bound resources to avoid double binding.
        XMFLOAT4                        m_Previous_BlendFactor[g_CommandList_Count];
        ID3D11VertexShader*             m_Previous_VertexShaders[g_CommandList_Count] = {};
        ID3D11PixelShader*              m_Previous_PixelShaders[g_CommandList_Count] = {};
        const RHI_BlendState*           m_Previous_BlendState[g_CommandList_Count] = {};
        const RHI_RasterizerState*      m_Previous_RasterizerState[g_CommandList_Count] = {};
        const RHI_DepthStencilState*    m_Previous_DepthStencilState[g_CommandList_Count] = {};
        const RHI_InputLayout*          m_Previous_InputLayout[g_CommandList_Count] = {};
        Primitive_Topology              m_Previous_PrimitiveTopology[g_CommandList_Count] = {};
        uint32_t                        m_Previous_SampleMask[g_CommandList_Count] = {};

        std::vector<const RHI_SwapChain*> m_SwapChains[g_CommandList_Count] = {};
        
        const RHI_PipelineState* m_PSO_Active[g_CommandList_Count] = {};
        bool m_PSO_IsDirty[g_CommandList_Count] = {};
        void ValidatePSO(RHI_CommandList commandList);

        D3D_DRIVER_TYPE m_DriverType;
        D3D_FEATURE_LEVEL m_FeatureLevel;
        Microsoft::WRL::ComPtr<IDXGIFactory2> m_DXGIFactory;
        Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_DeviceContextImmediate;
        Microsoft::WRL::ComPtr<ID3D11Query> m_DisjointQueries[BUFFER_COUNT + 3];

        Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_DeviceContexts[g_CommandList_Count];


        bool m_IsDebugDevicePresent = false; // Or known as debug layer in DX11.

    private:
        EngineContext* m_EngineContext;
    };
}