#include "Aurora.h"
#include "Material.h"
#include "../Renderer/Renderer.h"
#include "../Resource/ResourceCache.h"

namespace Aurora
{
    Material::Material(EngineContext* engineContext, Entity* entity, uint32_t componentID) : IComponent(engineContext, entity, componentID)
    {
    }

    uint32_t Material::GetRenderTypes() const
    {
        /// No custom shader support for now. In the future, we will have to retrieve render types from shaders created by users.
        if (m_UserBlendMode == BlendMode_Types::BlendMode_Opaque)
        {
            return Render_Types::Render_Type_Opaque;
        }

        return Render_Types::Render_Type_Transparent;
    }

    void Material::WriteShaderMaterial(ShaderMaterial* destination) const
    {
        destination->material_BaseColor = m_BaseColor;
        destination->material_SpecularColor = m_SpecularColor;
        destination->material_EmissiveColor = m_EmissiveColor;

        destination->material_Roughness = m_Roughness;
        destination->material_Metalness = m_Metalness;
        destination->material_NormalMapStrength = (m_Textures[Texture_Slot::Normal_Map].m_Resource == nullptr ? 0 : m_NormalMapStrength);
        destination->material_DisplacementMapping = m_DisplacementMapping;

        destination->material_UVSet_BaseColorMap = m_Textures[Texture_Slot::Base_Color_Map].GetUVSet();
        destination->material_UVSet_SurfaceMap = m_Textures[Texture_Slot::Surface_Map].GetUVSet();
        destination->material_UVSet_NormalMap = m_Textures[Texture_Slot::Normal_Map].GetUVSet();
        destination->material_UVSet_DisplacementMap = m_Textures[Texture_Slot::Displacement_Map].GetUVSet();
        destination->material_UVSet_EmissiveMap = m_Textures[Texture_Slot::Emissive_Map].GetUVSet();
        destination->material_UVSet_OcclusionMap = m_Textures[Texture_Slot::Occlusion_Map].GetUVSet();
        destination->material_UVSet_TransmissionMap = m_Textures[Texture_Slot::Transmission_Map].GetUVSet();
        destination->material_UVSet_SpecularMap = m_Textures[Texture_Slot::Specular_Map].GetUVSet();

        destination->material_Options = 0;

        if (IsUsingVertexColors())
        {
            destination->material_Options |= ShaderMaterial_Option_Bit_Use_Vertex_Colors;
        }

        if (IsReceivingShadow())
        {
            destination->material_Options |= ShaderMaterial_Option_Bit_Receive_Shadow;
        }

        if (IsCastingShadow())
        {
            destination->material_Options |= ShaderMaterial_Option_Bit_Cast_Shadow;
        }

        /// Set Descriptor Indexes (For DX12, Vulkan).
    }

    void Material::WriteTextures(const RHI_GPU_Resource** destination, int count) const
    {
        count = std::min(count, (int)Texture_Slot::Texture_Slot_Count);
        for (int i = 0; i < count; ++i)
        {
            destination[i] = m_Textures[i].GetGPUResource();
        }
    }

    void Material::CreateRenderData()
    {
        for (TextureMap& texture : m_Textures)
        {
            if (!texture.m_FilePath.empty())
            {
                texture.m_Resource = m_EngineContext->GetSubsystem<ResourceCache>()->Load(FileSystem::GetNameFromFilePath(texture.m_FilePath), texture.m_FilePath);
            }
        }

        ShaderMaterial shaderMaterialBuffer;
        WriteShaderMaterial(&shaderMaterialBuffer);

        RHI_Subresource_Data resourceData;
        resourceData.m_SystemMemory = &shaderMaterialBuffer;

        DX11_GraphicsDevice* graphicsDevice = m_EngineContext->GetSubsystem<Renderer>()->m_GraphicsDevice.get();

        RHI_GPU_Buffer_Description bufferDescription;
        bufferDescription.m_Usage = Usage::Default;
        bufferDescription.m_BindFlags = Bind_Flag::Bind_Constant_Buffer;
        /// Bindless Descriptors Support.
        bufferDescription.m_ByteWidth = sizeof(ConstantBufferData_Material);

        graphicsDevice->CreateBuffer(&bufferDescription, &resourceData, &m_ConstantBuffer);
    }
}