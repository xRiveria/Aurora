#pragma once
#include "../Log/Log.h"
#include "../RHI_Implementation.h"
#include "../RHI_Utilities.h"
#include <wrl/client.h>

namespace Aurora::DX11_Utility
{
    struct DX11SwapChainPackage
    {
        Microsoft::WRL::ComPtr<IDXGISwapChain1> m_SwapChain;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RenderTargetView;
        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_BackBuffer;
    };

    inline DX11SwapChainPackage* ToInternal(const SwapChain* swapchain)
    {
        return static_cast<DX11SwapChainPackage*>(swapchain->m_InternalState.get());
    }

    constexpr DXGI_FORMAT ConvertFormatToDX11Format(Format format)
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
		}

		return DXGI_FORMAT_UNKNOWN;
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
}