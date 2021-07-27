#pragma once
#include "../Log/Log.h"
#include "../RHI_Implementation.h"
#include "../RHI_Utilities.h"
#include <wrl/client.h>

using namespace Microsoft::WRL;

namespace Aurora::DX11_Utility
{
	constexpr D3D11_BLEND DX11_ConvertBlendFactor(Blend_Factor factor)
	{
		switch (factor)
		{
			case Blend_Factor::Blend_Zero:
				return D3D11_BLEND_ZERO;
				break;

			case Blend_Factor::Blend_One:
				return D3D11_BLEND_ONE;
				break;
			
			case Blend_Factor::Blend_Source_Color:
				return D3D11_BLEND_SRC_COLOR;
				break;

			case Blend_Factor::Blend_Inverse_Source_Color:
				return D3D11_BLEND_INV_SRC_COLOR;
				break;

			case Blend_Factor::Blend_Source_Alpha:
				return D3D11_BLEND_SRC_ALPHA;
				break;

			case Blend_Factor::Blend_Inverse_Source_Alpha:
				return D3D11_BLEND_INV_SRC_ALPHA;
				break;

			case Blend_Factor::Blend_Destination_Alpha:
				return D3D11_BLEND_DEST_ALPHA;
				break;

			case Blend_Factor::Blend_Inverse_Destination_Alpha:
				return D3D11_BLEND_INV_DEST_ALPHA;
				break;

			case Blend_Factor::Blend_Destination_Color:
				return D3D11_BLEND_DEST_COLOR;
				break;

			case Blend_Factor::Blend_Inverse_Destination_Color:
				return D3D11_BLEND_INV_DEST_COLOR;
				break;

			case Blend_Factor::Blend_Source_Alpha_Saturate:
				return D3D11_BLEND_SRC_ALPHA_SAT;
				break;

			case Blend_Factor::Blend_Blend_Factor:
				return D3D11_BLEND_BLEND_FACTOR;
				break;

			case Blend_Factor::Blend_Inverse_Blend_Factor:
				return D3D11_BLEND_INV_BLEND_FACTOR;
				break;

			case Blend_Factor::Blend_Source1_Color:
				return D3D11_BLEND_SRC1_COLOR;
				break;

			case Blend_Factor::Blend_Inverse_Source1_Color:
				return D3D11_BLEND_INV_SRC1_COLOR;
				break;

			case Blend_Factor::Blend_Source1_Alpha:
				return D3D11_BLEND_SRC1_ALPHA;
				break;

			case Blend_Factor::Blend_Inverse_Source1_Alpha:
				return D3D11_BLEND_INV_SRC1_ALPHA;
				break;
				
			default:
				break;
		}

		return D3D11_BLEND_ZERO;
	}

	constexpr D3D11_BLEND_OP DX11_ConvertBlendOperation(Blend_Operation operation)
	{
		switch (operation)
		{
			case Blend_Operation::Blend_Operation_Add:
				return D3D11_BLEND_OP_ADD;
				break;

			case Blend_Operation::Blend_Operation_Subtract:
				return D3D11_BLEND_OP_SUBTRACT;
				break;

			case Blend_Operation::Blend_Operation_ReverseSubtract:
				return D3D11_BLEND_OP_REV_SUBTRACT;
				break;

			case Blend_Operation::Blend_Operation_Minimum:
				return D3D11_BLEND_OP_MIN;
				break;

			case Blend_Operation::Blend_Operation_Maximum:
				return D3D11_BLEND_OP_MAX;
				break;

			default:
				break;
		}

		return D3D11_BLEND_OP_ADD;
	}

	constexpr uint32_t DX11_ParseColorWriteMask(uint32_t mask)
	{
		uint32_t flags = 0;

		if (mask == Color_Write_Mask::Color_Write_Enable_All)
		{
			return D3D11_COLOR_WRITE_ENABLE_ALL;
		}
		else
		{
			if (mask & Color_Write_Mask::Color_Write_Enable_Red)
			{
				flags |= D3D11_COLOR_WRITE_ENABLE_RED;
			}
			if (mask & Color_Write_Mask::Color_Write_Enable_Green)
			{
				flags |= D3D11_COLOR_WRITE_ENABLE_GREEN;
			}
			if (mask & Color_Write_Mask::Color_Write_Enable_Blue)
			{
				flags |= D3D11_COLOR_WRITE_ENABLE_BLUE;
			}
			if (mask & Color_Write_Mask::Color_Write_Enable_Alpha)
			{
				flags |= D3D11_COLOR_WRITE_ENABLE_ALPHA;
			}
		}

		return flags;
	}

	constexpr D3D11_STENCIL_OP DX11_ConvertStencilOperation(Stencil_Operation operation)
	{
		switch (operation)
		{
			case Stencil_Operation::Stencil_Operation_Keep:
				return D3D11_STENCIL_OP_KEEP;
				break;
			case Stencil_Operation::Stencil_Operation_Zero:
				return D3D11_STENCIL_OP_ZERO;
				break;
			case Stencil_Operation::Stencil_Operation_Replace:
				return D3D11_STENCIL_OP_REPLACE;
				break;
			case Stencil_Operation::Stencil_Operation_Increment_Saturation:
				return D3D11_STENCIL_OP_INCR_SAT;
				break;
			case Stencil_Operation::Stencil_Operation_Decrement_Saturation:
				return D3D11_STENCIL_OP_DECR_SAT;
				break;
			case Stencil_Operation::Stencil_Operation_Invert:
				return D3D11_STENCIL_OP_INVERT;
				break;
			case Stencil_Operation::Stencil_Operation_Increment:
				return D3D11_STENCIL_OP_INCR;
				break;
			case Stencil_Operation::Stencil_Operation_Decrement:
				return D3D11_STENCIL_OP_DECR;
				break;
		}

		return D3D11_STENCIL_OP_KEEP;
	}

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

	constexpr D3D11_FILL_MODE DX11_ConvertFillMode(Fill_Mode fillMode)
	{
		switch (fillMode)
		{
			case Fill_Mode::Fill_Wireframe:
				return D3D11_FILL_WIREFRAME;
				break;

			case Fill_Mode::Fill_Solid:
				return D3D11_FILL_SOLID;
				break;

			default:
				break;
		}

		return D3D11_FILL_WIREFRAME;
	}
	
	constexpr D3D11_DEPTH_WRITE_MASK DX11_ConvertDepthWriteMask(Depth_Write_Mask mask)
	{
		switch (mask)
		{
			case Depth_Write_Mask::Depth_Write_Mask_Zero:
				return D3D11_DEPTH_WRITE_MASK_ZERO;
				break;

			case Depth_Write_Mask::Depth_Write_Mask_All:
				return D3D11_DEPTH_WRITE_MASK_ALL;
				break;

			default:
				break;
		}

		return D3D11_DEPTH_WRITE_MASK_ZERO;
	}

	constexpr D3D11_CULL_MODE DX11_ConvertCullMode(Cull_Mode cullMode)
	{
		switch (cullMode)
		{
			case Cull_Mode::Cull_None:
				return D3D11_CULL_NONE;
				break;

			case Cull_Mode::Cull_Front:
				return D3D11_CULL_FRONT;
				break;

			case Cull_Mode::Cull_Back:
				return D3D11_CULL_BACK;
				break;

			default:
				break;
		}

		return D3D11_CULL_NONE;
	}

	constexpr D3D11_INPUT_CLASSIFICATION DX11_ConvertInputClassification(Input_Classification classification)
	{
		switch (classification)
		{
			case Input_Per_Vertex_Data:
				return D3D11_INPUT_PER_VERTEX_DATA;
				break;

			case Input_Per_Instance_Data:
				return D3D11_INPUT_PER_INSTANCE_DATA;
				break;
			default:
				break;
		}

		return D3D11_INPUT_PER_VERTEX_DATA;
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

			case FORMAT_R32_UINT:
				return DXGI_FORMAT_R32_UINT;
				break;

			case FORMAT_R16_UINT:
				return DXGI_FORMAT_R16_UINT;
				break;

			case FORMAT_R16G16_FLOAT:
				return DXGI_FORMAT_R16G16_FLOAT;
				break;

			case FORMAT_BC6H_SF16:
				return DXGI_FORMAT_BC6H_SF16;
				break;
		}

		return DXGI_FORMAT_UNKNOWN;
    }

	constexpr D3D11_FILTER DX11_ConvertFilter(Filter filter)
	{
		switch (filter)
		{
			case FILTER_MIN_MAG_MIP_POINT:
				return D3D11_FILTER_MIN_MAG_MIP_POINT;
				break;
			case FILTER_MIN_MAG_POINT_MIP_LINEAR:
				return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
				break;
			case FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT:
				return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
				break;
			case FILTER_MIN_POINT_MAG_MIP_LINEAR:
				return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
				break;
			case FILTER_MIN_LINEAR_MAG_MIP_POINT:
				return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
				break;
			case FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
				return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
				break;
			case FILTER_MIN_MAG_LINEAR_MIP_POINT:
				return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
				break;
			case FILTER_MIN_MAG_MIP_LINEAR:
				return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				break;
			case FILTER_ANISOTROPIC:
				return D3D11_FILTER_ANISOTROPIC;
				break;
			case FILTER_COMPARISON_MIN_MAG_MIP_POINT:
				return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
				break;
			case FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR:
				return D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
				break;
			case FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT:
				return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
				break;
			case FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR:
				return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
				break;
			case FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT:
				return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
				break;
			case FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
				return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
				break;
			case FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT:
				return D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
				break;
			case FILTER_COMPARISON_MIN_MAG_MIP_LINEAR:
				return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
				break;
			case FILTER_COMPARISON_ANISOTROPIC:
				return D3D11_FILTER_COMPARISON_ANISOTROPIC;
				break;
			case FILTER_MINIMUM_MIN_MAG_MIP_POINT:
				return D3D11_FILTER_MINIMUM_MIN_MAG_MIP_POINT;
				break;
			case FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR:
				return D3D11_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR;
				break;
			case FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:
				return D3D11_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
				break;
			case FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR:
				return D3D11_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR;
				break;
			case FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT:
				return D3D11_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT;
				break;
			case FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
				return D3D11_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
				break;
			case FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT:
				return D3D11_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT;
				break;
			case FILTER_MINIMUM_MIN_MAG_MIP_LINEAR:
				return D3D11_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;
				break;
			case FILTER_MINIMUM_ANISOTROPIC:
				return D3D11_FILTER_MINIMUM_ANISOTROPIC;
				break;
			case FILTER_MAXIMUM_MIN_MAG_MIP_POINT:
				return D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_POINT;
				break;
			case FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR:
				return D3D11_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR;
				break;
			case FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:
				return D3D11_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
				break;
			case FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR:
				return D3D11_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR;
				break;
			case FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT:
				return D3D11_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT;
				break;
			case FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
				return D3D11_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
				break;
			case FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT:
				return D3D11_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT;
				break;
			case FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR:
				return D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;
				break;
			case FILTER_MAXIMUM_ANISOTROPIC:
				return D3D11_FILTER_MAXIMUM_ANISOTROPIC;
				break;
			default:
				break;
			}
		return D3D11_FILTER_MIN_MAG_MIP_POINT;
	}

	constexpr D3D11_TEXTURE_ADDRESS_MODE DX11_ConvertTextureAddressMode(Texture_Address_Mode mode)
	{
		switch (mode)
		{
			case Texture_Address_Mode::Texture_Address_Wrap:
			{
				return D3D11_TEXTURE_ADDRESS_WRAP;
				break;
			}
			case Texture_Address_Mode::Texture_Address_Mirror:
			{
				return D3D11_TEXTURE_ADDRESS_MIRROR;
				break;
			}
			case Texture_Address_Mode::Texture_Address_Clamp:
			{
				return D3D11_TEXTURE_ADDRESS_CLAMP;
				break;
			}
			case Texture_Address_Mode::Texture_Address_Border:
			{
				return D3D11_TEXTURE_ADDRESS_BORDER;
				break;
			}
			default:
			{
				break;
			}
		}

		return D3D11_TEXTURE_ADDRESS_WRAP;
	}

	constexpr D3D11_COMPARISON_FUNC DX11_ConvertComparisonFunction(ComparisonFunction function)
	{
		switch (function)
		{
			case ComparisonFunction::Comparison_Never:
			{
				return D3D11_COMPARISON_NEVER;
				break;
			}
			case ComparisonFunction::Comparison_Less:
			{
				return D3D11_COMPARISON_LESS;
				break;
			}
			case ComparisonFunction::Comparison_Equal:
			{
				return D3D11_COMPARISON_EQUAL;
				break;
			}
			case ComparisonFunction::Comparison_Less_Equal:
			{
				return D3D11_COMPARISON_LESS_EQUAL;
				break;
			}
			case ComparisonFunction::Comparison_Greater:
			{
				return D3D11_COMPARISON_GREATER;
				break;
			}
			case ComparisonFunction::Comparison_Not_Equal:
			{
				return D3D11_COMPARISON_NOT_EQUAL;
				break;
			}
			case ComparisonFunction::Comparison_Greater_Equal:
			{
				return D3D11_COMPARISON_GREATER_EQUAL;
				break;
			}
			case ComparisonFunction::Comparison_Always:
			{
				return D3D11_COMPARISON_ALWAYS;
				break;
			}
			default:
			{
				break;
			}
		}

		return D3D11_COMPARISON_NEVER;
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

	struct DX11_PipelineStatePackage
	{
		ComPtr<ID3D11BlendState> m_BlendState;
		ComPtr<ID3D11DepthStencilState> m_DepthStencilState;
		ComPtr<ID3D11RasterizerState> m_RasterizerState;
		ComPtr<ID3D11InputLayout> m_InputLayout;
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

	inline DX11_PipelineStatePackage* ToInternal(const RHI_PipelineState* pipelineState)
	{
		return static_cast<DX11_PipelineStatePackage*>(pipelineState->m_InternalState.get());
	}

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

	inline DX11_ResourcePackage* ToInternal(const RHI_GPU_Resource* resource)
	{
		return static_cast<DX11_ResourcePackage*>(resource->m_InternalState.get());
	}
}