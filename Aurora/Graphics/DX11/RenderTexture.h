#pragma once
#include "../Math/Vector3.h"
#include "../RHI_Implementation.h"

/*
    Render Textures will contain of 3 things - The texture itself, a render target view and shader resource view.
*/

using namespace Aurora::Math;

namespace Aurora
{
    class Renderer;

    class RenderTexture
    {
    public:
        RenderTexture(EngineContext* engineContext);
        ~RenderTexture();

        bool InitializeResources(int width, int height, int mipLevels);

        void SetRenderTarget(ID3D11DepthStencilView* depthStencilView) const;
        void ClearRenderTarget(ID3D11DepthStencilView* depthStencilView, Vector3 clearColor) const;

        ID3D11ShaderResourceView* GetSRV() const { return m_ShaderResourceView; }
        ID3D11Texture2D* GetTexture() const { return m_RenderTargetTexture; }

    private:
        int m_TextureWidth;
        int m_TextureHeight;

        ID3D11Texture2D* m_RenderTargetTexture;
        ID3D11RenderTargetView* m_RenderTargetView;
        ID3D11ShaderResourceView* m_ShaderResourceView;

        Renderer* m_Renderer;
        EngineContext* m_EngineContext;
    };
}