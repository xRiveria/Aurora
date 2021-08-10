#pragma once
#include "../RHI_Implementation.h"
#include "DX11_Texture.h"

/* ==== Utilities ====

    Several items are packaged up together to avoid the need of creating multiple resources manually. By packaging said items, code can be abstracted and ensures that we don't 
    forget certain things.
*/

namespace Aurora
{
    // A framebuffer object allows us to specify attachments that are used for rendering. This includes the render textures (color/depth) and their respective views.
    struct DX11_Framebuffer
    {
        DX11_Texture m_RenderTargetTexture;
        DX11_Texture m_DepthStencilTexture;
    };

    inline UINT ParseBindFlags(uint32_t resourceFlags)
    {
        UINT bindFlags = 0;

        if (resourceFlags & DX11_ResourceViewFlag::Texture_Flag_SRV)
        {
            bindFlags |= D3D11_BIND_SHADER_RESOURCE;
        }

        if (resourceFlags & DX11_ResourceViewFlag::Texture_Flag_RTV)
        {
            bindFlags |= D3D11_BIND_RENDER_TARGET;
        }

        if (resourceFlags & DX11_ResourceViewFlag::Texture_Flag_DSV)
        {
            bindFlags |= D3D11_BIND_DEPTH_STENCIL;
        }

        if (resourceFlags & DX11_ResourceViewFlag::Texture_Flag_UAV)
        {
            bindFlags |= D3D11_BIND_UNORDERED_ACCESS;
        }

        return bindFlags;
    }
}