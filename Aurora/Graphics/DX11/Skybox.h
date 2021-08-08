#pragma once
#include "EngineContext.h"
#include "../RHI_Implementation.h"
#include "../Renderer/Renderer.h"
#include "../Resource/Importers/Importer_Image.h"
#include "../Resource/Importers/Importer_Model.h"
#include "../DX11_Refactored/DX11_VertexBuffer.h"
#include "../DX11_Refactored/DX11_IndexBuffer.h"

namespace Aurora
{
    struct Texture
    {
        ComPtr<ID3D11Texture2D> m_Texture;
        ComPtr<ID3D11ShaderResourceView> m_SRV;
        ComPtr<ID3D11UnorderedAccessView> m_UAV;
        UINT m_Width, m_Height;
        UINT m_Levels;
    };

    struct MeshBuffer
    {
        std::shared_ptr<DX11_VertexBuffer> vertexBuffer;
        std::shared_ptr<DX11_IndexBuffer> indexBuffer;
        UINT stride;
        UINT offset;
        UINT numElements;
    };

    class Skybox
    {
    public:
        Skybox(EngineContext* engineContext);
        ~Skybox();

        bool InitializeResources();
        bool Render() const;


    public:
        template<typename T> 
        inline static constexpr T roundToPowerOfTwo(T value, int POT)
        {
            return (value + POT - 1) & -POT;
        }

        ComPtr<ID3D11SamplerState> CreateSamplerState(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode) const;
        Texture CreateTextureCube(UINT width, UINT height, DXGI_FORMAT format, UINT levels = 0) const;
        Texture CreateTexture(UINT width, UINT height, DXGI_FORMAT format, UINT levels = 0) const;
        Texture CreateTexture(const std::shared_ptr<ImageDerp>& image, DXGI_FORMAT format, UINT levels) const;
        void CreateTextureUAV(Texture& texture, UINT mipSlice) const;
        ComPtr<ID3D11Buffer> CreateConstantBuffer(const void* data, UINT size) const;
        template<typename T> ComPtr<ID3D11Buffer> CreateConstantBuffer(const T* data = nullptr) const
        {
            static_assert(sizeof(T) == roundToPowerOfTwo(sizeof(T), 16));
            return CreateConstantBuffer(data, sizeof(T));
        }

        std::shared_ptr<MeshBuffer> CreateMeshBuffer(const std::shared_ptr<class MeshDerp>& mesh) const;

        ComPtr<ID3D11InputLayout> m_InputLayout;
        Texture m_EnvironmentTextureEquirectangular;
        std::shared_ptr<MeshBuffer> m_SkyboxEntity;
        ComPtr<ID3D11RasterizerState> m_DefaultRasterizerState;
        ComPtr<ID3D11DepthStencilState> m_SkyboxDepthStencilState;
        RHI_Shader m_VSSkyboxShader;
        RHI_Shader m_PSSkyboxShader;

        Texture m_EnvironmentTexture;
        Texture m_IrradianceMapTexture;
        Texture m_SpecularPrefilterBRDFLUT;

        ComPtr<ID3D11SamplerState> m_ComputeSampler;
        ComPtr<ID3D11SamplerState> m_DefaultSampler;
        ComPtr<ID3D11SamplerState> m_SpecularBRDFSampler;

        MeshBuffer m_Skybox;
      
        EngineContext* m_EngineContext;
        Renderer* m_Renderer;
    };
}