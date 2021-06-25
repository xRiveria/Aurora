#pragma once
#include "../Log/Log.h"
#include "../RHI_Implementation.h"
#include "../RHI_Utilities.h"
#include <wrl/client.h>

using namespace Microsoft::WRL;

namespace Aurora::DX11_Utility
{
	constexpr uint32_t DX11_ParseResourceMiscFlags(uint32_t flags)
	{
		uint32_t parsedFlags = 0;

		if (flags & Resource_Misc_Flag::Resource_Misc_Shared)
		{
			parsedFlags |= D3D11_RESOURCE_MISC_SHARED;
		}
		if (flags & Resource_Misc_Flag::Resource_Misc_TextureCube)
		{
			parsedFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
		}
		if (flags & Resource_Misc_Flag::Resource_Misc_Indirect_Args)
		{
			parsedFlags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
		}
		if (flags & Resource_Misc_Flag::Resource_Misc_Buffer_Allow_Raw_Views)
		{
			parsedFlags |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
		}
		if (flags & Resource_Misc_Flag::Resource_Misc_Buffer_Structured)
		{
			parsedFlags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		}
		if (flags & Resource_Misc_Flag::Resource_Misc_Tiled)
		{
			parsedFlags |= D3D11_RESOURCE_MISC_TILED;
		}

		return parsedFlags;
	}

	constexpr uint32_t DX11_ParseCPUAccessFlags(uint32_t flags)
	{
		uint32_t parsedFlags = 0;

		if (flags & CPU_Access::CPU_Access_Write)
		{
			parsedFlags |= D3D11_CPU_ACCESS_WRITE;
		}
		if (flags & CPU_Access::CPU_Access_Read)
		{
			parsedFlags |= D3D11_CPU_ACCESS_READ;
		}

		return parsedFlags;
	}

	constexpr uint32_t DX11_ParseBindFlags(uint32_t flags)
	{
		uint32_t parsedFlags = 0;

		if (flags & Bind_Flag::Bind_Vertex_Buffer)
		{
			parsedFlags |= D3D11_BIND_VERTEX_BUFFER;
		}
		if (flags & Bind_Flag::Bind_Index_Buffer)
		{
			parsedFlags |= D3D11_BIND_INDEX_BUFFER;
		}
		if (flags & Bind_Flag::Bind_Constant_Buffer)
		{
			parsedFlags |= D3D11_BIND_CONSTANT_BUFFER;
		}
		if (flags & Bind_Flag::Bind_Shader_Resource)
		{
			parsedFlags |= D3D11_BIND_SHADER_RESOURCE;
		}
		if (flags & Bind_Flag::Bind_Stream_Output)
		{
			parsedFlags |= D3D11_BIND_STREAM_OUTPUT;
		}
		if (flags & Bind_Flag::Bind_Render_Target)
		{
			parsedFlags |= D3D11_BIND_RENDER_TARGET;
		}
		if (flags & Bind_Flag::Bind_Depth_Stencil)
		{
			parsedFlags |= D3D11_BIND_DEPTH_STENCIL;
		}
		if (flags & Bind_Flag::Bind_Unordered_Access)
		{
			parsedFlags |= D3D11_BIND_UNORDERED_ACCESS;
		}

		return parsedFlags;
	}

	constexpr D3D11_USAGE DX11_ConvertUsageFlags(Usage usage)
	{
		switch (usage)
		{
			case Usage::Default:
				return D3D11_USAGE_DEFAULT;
				break;

			case Usage::Immutable:
				return D3D11_USAGE_IMMUTABLE;
				break;

			case Usage::Dynamic:
				return D3D11_USAGE_DYNAMIC;
				break;

			case Usage::Staging:
				return D3D11_USAGE_STAGING;
				break;

			default:
				break;
		}

		return D3D11_USAGE_DEFAULT;
	}

    constexpr DXGI_FORMAT DX11_ConvertFormat(Format format)
    {
        switch (format)
        {
			case FORMAT_UNKNOWN:
				return DXGI_FORMAT_UNKNOWN;
				break;

			case FORMAT_R32G32B32A32_FLOAT:
				return DXGI_FORMAT_R32G32B32A32_FLOAT;
				break;

			case FORMAT_R32G32B32A32_UINT:
				return DXGI_FORMAT_R32G32B32A32_UINT;
				break;

			case FORMAT_R32G32B32A32_SINT:
				return DXGI_FORMAT_R32G32B32A32_SINT;
				break;

			case FORMAT_R32G32B32_FLOAT:
				return DXGI_FORMAT_R32G32B32_FLOAT;
				break;

			case FORMAT_R32G32B32_UINT:
				return DXGI_FORMAT_R32G32B32_UINT;
				break;

			case FORMAT_R32G32B32_SINT:
				return DXGI_FORMAT_R32G32B32_SINT;
				break;

			case FORMAT_R16G16B16A16_FLOAT:
				return DXGI_FORMAT_R16G16B16A16_FLOAT;
				break;

			case FORMAT_R16G16B16A16_UNORM:
				return DXGI_FORMAT_R16G16B16A16_UNORM;
				break;

			case FORMAT_R16G16B16A16_UINT:
				return DXGI_FORMAT_R16G16B16A16_UINT;
				break;

			case FORMAT_R16G16B16A16_SNORM:
				return DXGI_FORMAT_R16G16B16A16_SNORM;
				break;

			case FORMAT_R16G16B16A16_SINT:
				return DXGI_FORMAT_R16G16B16A16_SINT;
				break;

			case FORMAT_R32G32_FLOAT:
				return DXGI_FORMAT_R32G32_FLOAT;
				break;

			case FORMAT_R32G32_UINT:
				return DXGI_FORMAT_R32G32_UINT;
				break;

			case FORMAT_R32G32_SINT:
				return DXGI_FORMAT_R32G32_SINT;
				break;

			case FORMAT_R32G8X24_TYPELESS:
				return DXGI_FORMAT_R32G8X24_TYPELESS;
				break;

			case FORMAT_D32_FLOAT_S8X24_UINT:
				return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
				break;

			case FORMAT_R10G10B10A2_UNORM:
				return DXGI_FORMAT_R10G10B10A2_UNORM;
				break;

			case FORMAT_R10G10B10A2_UINT:
				return DXGI_FORMAT_R10G10B10A2_UINT;
				break;

			case FORMAT_R11G11B10_FLOAT:
				return DXGI_FORMAT_R11G11B10_FLOAT;
				break;

			case FORMAT_R8G8B8A8_UNORM:
				return DXGI_FORMAT_R8G8B8A8_UNORM;
				break;

			case FORMAT_D24_UNORM_S8_UINT:
				return DXGI_FORMAT_D24_UNORM_S8_UINT;
				break;
		}

		return DXGI_FORMAT_UNKNOWN;
    }

	inline D3D11_TEXTURE1D_DESC DX11_ConvertTextureDescription1D(const RHI_Texture_Description* description)
	{
		D3D11_TEXTURE1D_DESC textureDescription = {};
		textureDescription.Width = description->m_Width;
		textureDescription.MipLevels = description->m_MipLevels;
		textureDescription.ArraySize = description->m_ArraySize;
		textureDescription.Format = DX11_ConvertFormat(description->m_Format);
		textureDescription.Usage = DX11_ConvertUsageFlags(description->m_Usage);
		textureDescription.BindFlags = DX11_ParseBindFlags(description->m_BindFlags);
		textureDescription.CPUAccessFlags = DX11_ParseCPUAccessFlags(description->m_CPUAccessFlags);
		textureDescription.MiscFlags = DX11_ParseResourceMiscFlags(description->m_MiscFlags);

		return textureDescription;
	}

	inline D3D11_TEXTURE2D_DESC DX11_ConvertTextureDescription2D(const RHI_Texture_Description* description)
	{
		D3D11_TEXTURE2D_DESC textureDescription = {};
		textureDescription.Width = description->m_Width;
		textureDescription.Height = description->m_Height;
		textureDescription.MipLevels = description->m_MipLevels;
		textureDescription.ArraySize = description->m_ArraySize;
		textureDescription.Format = DX11_ConvertFormat(description->m_Format);
		textureDescription.SampleDesc.Count = description->m_SampleCount;
		textureDescription.SampleDesc.Quality = 0;
		textureDescription.Usage = DX11_ConvertUsageFlags(description->m_Usage);
		textureDescription.BindFlags = DX11_ParseBindFlags(description->m_BindFlags);
		textureDescription.CPUAccessFlags = DX11_ParseCPUAccessFlags(description->m_CPUAccessFlags);
		textureDescription.MiscFlags = DX11_ParseResourceMiscFlags(description->m_MiscFlags);

		return textureDescription;
	}

	inline D3D11_TEXTURE3D_DESC DX11_ConvertTextureDescription3D(const RHI_Texture_Description* description)
	{
		D3D11_TEXTURE3D_DESC textureDescription = {};
		textureDescription.Width = description->m_Width;
		textureDescription.Height = description->m_Height;
		textureDescription.Depth = description->m_Height;
		textureDescription.MipLevels = description->m_MipLevels;
		textureDescription.Format = DX11_ConvertFormat(description->m_Format);
		textureDescription.Usage = DX11_ConvertUsageFlags(description->m_Usage);
		textureDescription.BindFlags = DX11_ParseBindFlags(description->m_BindFlags);
		textureDescription.CPUAccessFlags = DX11_ParseCPUAccessFlags(description->m_CPUAccessFlags);
		textureDescription.MiscFlags = DX11_ParseResourceMiscFlags(description->m_MiscFlags);

		return textureDescription;
	}

	inline D3D11_SUBRESOURCE_DATA DX11_ConvertSubresourceData(const RHI_Subresource_Data& initialData)
	{
		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = initialData.m_SystemMemory;
		data.SysMemPitch = initialData.m_SystemMemoryPitch;
		data.SysMemSlicePitch = initialData.m_SystemMemorySlicePitch;

		return data;
	}

    inline const char* DXGI_Error_ToString(const HRESULT errorCode)
    {
        switch (errorCode)
        {
            case DXGI_ERROR_DEVICE_HUNG:                    return "DXGI_ERROR_DEVICE_HUNG";                    // The application's device failed due to badly formed commands sent by the application. This is a design-time issue that should be investigated and fixed.
            case DXGI_ERROR_DEVICE_REMOVED:                 return "DXGI_ERROR_DEVICE_REMOVED";                 // The video card has been physically removed from the system, or a driver upgrade for the video card has occured. The application should destroy and recreate the device. For help debugging the problem, call ID3D10Device::GetDeviceRemovedReason.
            case DXGI_ERROR_DEVICE_RESET:                   return "DXGI_ERROR_DEVICE_RESET";                   // The device failed due to a badly formed command. This is a run-time issue; The application should destroy and recreate the device.
            case DXGI_ERROR_DRIVER_INTERNAL_ERROR:          return "DXGI_ERROR_DRIVER_INTERNAL_ERROR";          // The driver encountered a problem and was put into the device removed state.
            case DXGI_ERROR_FRAME_STATISTICS_DISJOINT:      return "DXGI_ERROR_FRAME_STATISTICS_DISJOINT";      // An event (for example, a power cycle) interrupted the gathering of presentation statistics.
            case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE:   return "DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE";   // The application attempted to acquire exclusive ownership of an output, but failed because some other application (or device within the application) already acquired ownership.
            case DXGI_ERROR_INVALID_CALL:                   return "DXGI_ERROR_INVALID_CALL";                   // The application provided invalid parameter data; this must be debugged and fixed before the application is released.
            case DXGI_ERROR_MORE_DATA:                      return "DXGI_ERROR_MORE_DATA";                      // The buffer supplied by the application is not big enough to hold the requested data.
            case DXGI_ERROR_NONEXCLUSIVE:                   return "DXGI_ERROR_NONEXCLUSIVE";                   // A global counter resource is in use, and the Direct3D device can't currently use the counter resource.
            case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE:        return "DXGI_ERROR_NOT_CURRENTLY_AVAILABLE";        // The resource or request is not currently available, but it might become available later.
            case DXGI_ERROR_NOT_FOUND:                      return "DXGI_ERROR_NOT_FOUND";                      // When calling IDXGIObject::GetPrivateData, the GUID passed in is not recognized as one previously passed to IDXGIObject::SetPrivateData or IDXGIObject::SetPrivateDataInterface. When calling IDXGIFentityy::EnumAdapters or IDXGIAdapter::EnumOutputs, the enumerated ordinal is out of range.
            case DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED:     return "DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED";     // Reserved
            case DXGI_ERROR_REMOTE_OUTOFMEMORY:             return "DXGI_ERROR_REMOTE_OUTOFMEMORY";             // Reserved
            case DXGI_ERROR_WAS_STILL_DRAWING:              return "DXGI_ERROR_WAS_STILL_DRAWING";              // The GPU was busy at the moment when a call was made to perform an operation, and did not execute or schedule the operation.
            case DXGI_ERROR_UNSUPPORTED:                    return "DXGI_ERROR_UNSUPPORTED";                    // The requested functionality is not supported by the device or the driver.
            case DXGI_ERROR_ACCESS_LOST:                    return "DXGI_ERROR_ACCESS_LOST";                    // The desktop duplication interface is invalid. The desktop duplication interface typically becomes invalid when a different type of image is displayed on the desktop.
            case DXGI_ERROR_WAIT_TIMEOUT:                   return "DXGI_ERROR_WAIT_TIMEOUT";                   // The time-out interval elapsed before the next desktop frame was available.
            case DXGI_ERROR_SESSION_DISCONNECTED:           return "DXGI_ERROR_SESSION_DISCONNECTED";           // The Remote Desktop Services session is currently disconnected.
            case DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE:       return "DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE";       // The DXGI output (monitor) to which the swap chain content was restricted is now disconnected or changed.
            case DXGI_ERROR_CANNOT_PROTECT_CONTENT:         return "DXGI_ERROR_CANNOT_PROTECT_CONTENT";         // DXGI can't provide content protection on the swap chain. This error is typically caused by an older driver, or when you use a swap chain that is incompatible with content protection.
            case DXGI_ERROR_ACCESS_DENIED:                  return "DXGI_ERROR_ACCESS_DENIED";                  // You tried to use a resource to which you did not have the required access privileges. This error is most typically caused when you write to a shared resource with read-only access.
            case DXGI_ERROR_NAME_ALREADY_EXISTS:            return "DXGI_ERROR_NAME_ALREADY_EXISTS";            // The supplied name of a resource in a call to IDXGIResource1::CreateSharedHandle is already associated with some other resource.
            case DXGI_ERROR_SDK_COMPONENT_MISSING:          return "DXGI_ERROR_SDK_COMPONENT_MISSING";          // The application requested an operation that depends on an SDK component that is missing or mismatched.
            case DXGI_ERROR_NOT_CURRENT:                    return "DXGI_ERROR_NOT_CURRENT";                    // The DXGI objects that the application has created are no longer current & need to be recreated for this operation to be performed.
            case DXGI_ERROR_HW_PROTECTION_OUTOFMEMORY:      return "DXGI_ERROR_HW_PROTECTION_OUTOFMEMORY";      // Insufficient HW protected memory exits for proper function.
            case DXGI_ERROR_DYNAMIC_CODE_POLICY_VIOLATION:  return "DXGI_ERROR_DYNAMIC_CODE_POLICY_VIOLATION";  // Creating this device would violate the process's dynamic code policy.
            case DXGI_ERROR_NON_COMPOSITED_UI:              return "DXGI_ERROR_NON_COMPOSITED_UI";              // The operation failed because the compositor is not in control of the output.
            case E_INVALIDARG:                              return "E_INVALIDARG";                              // One or more arguments are invalid.        
        }

        return "Unknown Error Code.";
    }

    constexpr bool BreakIfFailed(const HRESULT result)
    {
        if (FAILED(result))
        {
            AURORA_ERROR("%s.", DXGI_Error_ToString(result));
            return false;
        }

        return true;
    }

	// ==========================================================================================================

	struct DX11_ResourcePackage
	{
		ComPtr<ID3D11Resource> m_Resource; // ID3D11Buffer inherits from ID3D11Resource.
		ComPtr<ID3D11ShaderResourceView> m_ShaderResourceView;
		ComPtr<ID3D11UnorderedAccessView> m_UnorderedAccessView;
		std::vector<ComPtr<ID3D11ShaderResourceView>> m_Subresources_ShaderResourceView;
		std::vector<ComPtr<ID3D11UnorderedAccessView>> m_Subresources_UnorderedAccessView;
	};

	struct DX11_TexturePackage : public DX11_ResourcePackage
	{
		ComPtr<ID3D11RenderTargetView> m_RenderTargetView;
		ComPtr<ID3D11DepthStencilView> m_DepthStencilView;
		std::vector<ComPtr<ID3D11RenderTargetView>> m_Subresources_RenderTargetView;
		std::vector<ComPtr<ID3D11DepthStencilView>> m_Subresources_DepthStencilView;
 	};

	struct DX11_VertexShaderPackage
	{
		ComPtr<ID3D11VertexShader> m_Resource;
		std::vector<uint8_t> m_ShaderCode;
	};

	struct DX11_PixelShaderPackage
	{
		ComPtr<ID3D11PixelShader> m_Resource;
	};

	struct DX11_HullShaderPackage
	{
		ComPtr<ID3D11HullShader> m_Resource;
	};

	struct DX11_DomainShaderPackage
	{
		ComPtr<ID3D11DomainShader> m_Resource;
	};

	struct DX11_GeometryShaderPackage
	{
		ComPtr<ID3D11GeometryShader> m_Resource;
	};

	struct DX11_ComputeShaderPackage
	{
		ComPtr<ID3D11ComputeShader> m_Resource;
	};

	struct DX11_SwapChainPackage
	{
		Microsoft::WRL::ComPtr<IDXGISwapChain1> m_SwapChain;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RenderTargetView;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_BackBuffer;
	};

	struct DX11_SamplerPackage
	{
		ComPtr<ID3D11SamplerState> m_Resource;
	};

	inline DX11_SwapChainPackage* ToInternal(const RHI_SwapChain* swapchain)
	{
		return static_cast<DX11_SwapChainPackage*>(swapchain->m_InternalState.get());
	}

	inline DX11_SamplerPackage* ToInternal(const RHI_Sampler* sampler)
	{
		return static_cast<DX11_SamplerPackage*>(sampler->m_InternalState.get());
	}

	inline DX11_TexturePackage* ToInternal(const RHI_Texture* texture)
	{
		return static_cast<DX11_TexturePackage*>(texture->m_InternalState.get());
	}

	inline DX11_ResourcePackage* ToInternal(const RHI_GPU_Buffer* buffer)
	{
		return static_cast<DX11_ResourcePackage*>(buffer->m_InternalState.get());
	}
}