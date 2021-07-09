#include "Aurora.h"
#include "Material.h"
#include "../Renderer/Renderer.h"
#include "../Resource/ResourceCache.h"

namespace Aurora
{
    void Material::WriteShaderMaterial(ShaderMaterial* destination) const
    {
    }

    void Material::WriteTextures(const RHI_GPU_Resource** destination, int count) const
    {
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