#pragma once
#include "EngineContext.h"
#include "RenderTexture.h"
#include "../RHI_Implementation.h"

namespace Aurora
{
    class Renderer;

    class Cubemap
    {
    public:
        Cubemap(EngineContext* engineContext);
        ~Cubemap();

        bool InitializeResources(std::vector<std::shared_ptr<RenderTexture>> faces, int width, int height, int mipLevels);
        void Copy(std::vector<std::shared_ptr<RenderTexture>> faces, int width, int height, int mipSlice) const;

        ID3D11Texture2D* GetTexture() const { return m_Texture; }
        ID3D11ShaderResourceView* GetSRV() const { return m_ShaderResourceView; }

    private:
        ID3D11Texture2D* m_Texture;
        ID3D11ShaderResourceView* m_ShaderResourceView;
        int m_MipLevels;

        EngineContext* m_EngineContext;
        Renderer* m_Renderer;
    };
}