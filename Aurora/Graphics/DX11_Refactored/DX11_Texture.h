#pragma once
#include "../Resource/AuroraResource.h"
#include "../RHI_Implementation.h"

namespace Aurora
{
    enum DX11_ResourceViewFlag : uint32_t // #
    {
        Texture_Flag_SRV = 1 << 0,       // Shader Resource View
        Texture_Flag_UAV = 1 << 1,       // Unordered Access View
        Texture_Flag_RTV = 1 << 2,       // Render Target View
        Texture_Flag_DSV = 1 << 3,       // Depth Stencil View
    };

    class DX11_Texture : public AuroraResource
    {
    public:
        DX11_Texture(EngineContext* engineContext);
        ~DX11_Texture() override;

        bool SaveToFile(const std::string& filePath) override;
        bool LoadFromFile(const std::string& filePath) override;

        bool Initialize2DTexture(uint32_t textureWidth, uint32_t textureHeight, DXGI_FORMAT format, uint32_t textureFlags, DX11_Devices* devices, uint32_t sampleLevels = 1, uint32_t mipLevels = 0, uint32_t mipSlice = 1);
        bool Initialize2DTextureFromFile(const void* sourceData, uint32_t textureWidth, uint32_t textureHeight, DXGI_FORMAT format, uint32_t sampleLevels, uint32_t textureFlags, uint32_t mipSlice, DX11_Devices* devices);
        bool InitializeTextureCube(uint32_t textureWidth, uint32_t textureHeight, DXGI_FORMAT format, DX11_Devices* devices, uint32_t mipLevels = 0);

        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }
        uint32_t GetSampleLevel() const { return m_SampleLevel; }
        DXGI_FORMAT GetFormat() const { return m_Format; }
        uint32_t GetMipLevels() const { return m_MipLevels; }

        template<typename T>
        static inline constexpr T GetMipLevelsPossible(T width, T height)
        {
            T levels = 1;
            while ((width | height) >> levels)
            {
                ++levels;
            }

            return levels;
        }

        // Resource Views
        ComPtr<ID3D11Texture2D> GetTexture() const { return m_Texture; }
        ComPtr<ID3D11ShaderResourceView> GetShaderResourceView();
        ComPtr<ID3D11DepthStencilView> GetDepthStencilView();
        ComPtr<ID3D11UnorderedAccessView> GetUnorderedAccessView();
        ComPtr<ID3D11RenderTargetView> GetRenderTargetView();

        bool HasResourceView(DX11_ResourceViewFlag shaderViewType);

        bool CreateUnorderedAccessView(uint32_t mipSlice);

    private:
        bool CreateShaderResourceView(D3D11_SRV_DIMENSION viewDimension);
        bool CreateRenderTargetView();
        bool CreateDepthStencilView();

        void _DestroyTexture();

    private:
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        uint32_t m_SampleLevel = 0;
        uint32_t m_MipLevels = 0;
        uint32_t m_ArraySize = 0;
        DXGI_FORMAT m_Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;

        ComPtr<ID3D11Texture2D> m_Texture = nullptr;

        ComPtr<ID3D11RenderTargetView> m_RenderTargetView = nullptr;
        ComPtr<ID3D11ShaderResourceView> m_ShaderResourceView = nullptr;
        ComPtr<ID3D11DepthStencilView> m_DepthStencilView = nullptr;
        ComPtr<ID3D11UnorderedAccessView> m_UnorderedAccessView = nullptr;
        uint32_t m_ResourceViewFlags = 0;

        DX11_Devices* m_Devices = nullptr;
    };
}