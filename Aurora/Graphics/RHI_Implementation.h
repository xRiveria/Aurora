#pragma once

/* == RHI Implementation ==

    The implementation class includes/loads the respective dependencies required for our graphics API to function.
*/


#define AURORA_GRAPHICS_DX11 // Explicit for now.

#if defined(AURORA_GRAPHICS_DX11)

#include <d3d11_3.h>                            // Link against the Win32 Library
#include <dxgi1_3.h>                            // DirectX Graphics Interface
#include <d3dcompiler.h>                        // RHI_Shader Compiler
#include <DirectXMath.h>                        // Temporary. DirectX Math Library.
#include "../Graphics/DX11/DX11_Utilities.h"    // Our own utility helper to assist in rendering commands.

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3dcompiler.lib")

struct DX11_Devices
{
    ID3D11Device* m_Device = nullptr;
    ID3D11DeviceContext* m_DeviceContextImmediate = nullptr;
};

#endif