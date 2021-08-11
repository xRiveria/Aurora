#pragma once
#include "EngineContext.h"
#include "../RHI_Implementation.h"
#include "../Renderer/Renderer.h"
#include "../Resource/Importers/Importer_Image.h"
#include "../Resource/Importers/Importer_Model.h"
#include "../DX11_Refactored/DX11_VertexBuffer.h"
#include "../DX11_Refactored/DX11_IndexBuffer.h"
#include "../DX11_Refactored/DX11_InputLayout.h"
#include "../DX11_Refactored/DX11_Sampler.h"
#include "../DX11_Refactored/DX11_RasterizerState.h"

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

        // Texture CreateTextureCube(UINT width, UINT height, DXGI_FORMAT format, UINT levels = 0) const;
        Texture CreateTexture(UINT width, UINT height, DXGI_FORMAT format, UINT levels = 0) const;
        Texture CreateTexture(const std::shared_ptr<ImageDerp>& image, DXGI_FORMAT format, UINT levels) const;
        // void CreateTextureUAV(Texture& texture, UINT mipSlice) const;

        std::shared_ptr<MeshBuffer> CreateMeshBuffer(const std::shared_ptr<class MeshDerp>& mesh) const;

        std::shared_ptr<DX11_InputLayout> m_InputLayout;
        Texture m_EnvironmentTextureEquirectangular;
        std::shared_ptr<MeshBuffer> m_SkyboxEntity;

        ComPtr<ID3D11DepthStencilState> m_SkyboxDepthStencilState;

        RHI_Shader m_VSSkyboxShader;
        RHI_Shader m_PSSkyboxShader;

        std::shared_ptr<DX11_Texture> m_EnvironmentTexture;
        std::shared_ptr<DX11_Texture> m_IrradianceMapTexture;
        std::shared_ptr<DX11_Texture> m_SpecularPrefilterBRDFLUT;

        std::shared_ptr<DX11_Sampler> m_ComputeSampler;
        std::shared_ptr<DX11_Sampler> m_DefaultSampler;
        std::shared_ptr<DX11_Sampler> m_SpecularBRDFSampler;

        MeshBuffer m_Skybox;
      
        EngineContext* m_EngineContext;
        Renderer* m_Renderer;
    };
}