#include "Aurora.h"
#include "Importer_Image.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#include "../Renderer/Renderer.h"

namespace Aurora
{
    Importer_Image::Importer_Image(EngineContext* engineContext) : m_EngineContext(engineContext)
    {

    }

    std::shared_ptr<AuroraResource> Importer_Image::LoadTexture(const std::string& filePath, const std::string& fileName, uint32_t loadFlags)
    {
        Renderer* renderer = m_EngineContext->GetSubsystem<Renderer>();
        std::shared_ptr<AuroraResource> resource = std::make_shared<AuroraResource>();
        resource->m_FilePath = filePath;

        const int channelCount = 4;
        int width, height, bytesPerPixel;
        unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &bytesPerPixel, channelCount);

        if (data != nullptr)
        {
            AURORA_INFO("Successfully loaded Texture with path: %s.", filePath.c_str());

            RHI_Texture_Description textureDescription;
            textureDescription.m_Width = (uint32_t)width;
            textureDescription.m_Height = (uint32_t)height;
            textureDescription.m_Layout = Image_Layout::Image_Layout_Shader_Resource;

            if (loadFlags & Resource_Flags::Import_Color_Grading_LUT)
            {

            }
            else
            {
                textureDescription.m_BindFlags = Bind_Flag::Bind_Shader_Resource | Bind_Flag::Bind_Unordered_Access;
                textureDescription.m_CPUAccessFlags = 0;
                textureDescription.m_Format = Format::FORMAT_R8G8B8A8_UNORM;
                textureDescription.m_MipLevels = (uint32_t)log2(std::max(width, height)) + 1; // + 1 for base level.
                textureDescription.m_MiscFlags = 0;
                textureDescription.m_Usage = Usage::Default;
                textureDescription.m_Layout = Image_Layout::Image_Layout_Shader_Resource;

                uint32_t mipWidth = width;
                std::vector<RHI_Subresource_Data> initializationData(textureDescription.m_MipLevels);
                for (uint32_t mipIndex = 0; mipIndex < textureDescription.m_MipLevels; ++mipIndex)
                {
                    initializationData[mipIndex].m_SystemMemory = data; // We don't fill the mips correctly here. It will point to Mip0 by default. Mip levels will be created using compute shaders when needed.
                    initializationData[mipIndex].m_SystemMemoryPitch = static_cast<uint32_t>(mipWidth * channelCount);
                    mipWidth = std::max(1u, mipWidth / 2); 
                }

                renderer->m_GraphicsDevice->CreateTexture(&textureDescription, initializationData.data(), &resource->m_Texture);
                /// Set resource name.

                for (uint32_t i = 0; i < resource->m_Texture.m_Description.m_MipLevels; ++i)
                {
                    int subresourceIndex;
                    subresourceIndex = renderer->m_GraphicsDevice->CreateSubresource(&resource->m_Texture, Subresource_Type::ShaderResourceView, 0, 1, i, 1);
                    AURORA_ASSERT(subresourceIndex == i);
                    subresourceIndex = renderer->m_GraphicsDevice->CreateSubresource(&resource->m_Texture, Subresource_Type::UnorderedAccessView, 0, 1, i, 1);
                    AURORA_ASSERT(subresourceIndex == i);
                }
            }
        }

        else
        {
            AURORA_ERROR("Failed to load texture with path: %s.", filePath.c_str());
            return nullptr;
        }

        stbi_image_free(data);

        return resource;
    }

    std::shared_ptr<AuroraResource> Importer_Image::LoadHDRTexture(const std::string& filePath, int channels)
    {
        std::shared_ptr<AuroraResource> resource = std::make_shared<AuroraResource>();
        int width, height, bytesPerChannel;

        if (stbi_is_hdr(filePath.c_str()))
        {
            float* pixels = stbi_loadf(filePath.c_str(), &width, &height, &bytesPerChannel, 4);
            if (pixels)
            {
                RHI_Texture_Description textureDescription;
                textureDescription.m_Type = Texture_Type::Texture2D;
                textureDescription.m_Width = width;
                textureDescription.m_Height = height;
                textureDescription.m_MipLevels = 1;
                textureDescription.m_ArraySize = 1;
                textureDescription.m_Format = Format::FORMAT_R32G32B32A32_FLOAT;
                textureDescription.m_SampleCount = 1;
                textureDescription.m_Usage = Usage::Default;
                textureDescription.m_BindFlags = Bind_Flag::Bind_Shader_Resource | Bind_Flag::Bind_Unordered_Access;

                RHI_Subresource_Data initializationData;
                initializationData.m_SystemMemory = pixels;

                int bytesPerPixel = 4 * sizeof(float);
                int pitch = textureDescription.m_Width * bytesPerPixel;
                initializationData.m_SystemMemoryPitch = pitch;

                m_EngineContext->GetSubsystem<Renderer>()->m_GraphicsDevice->CreateTexture(&textureDescription, &initializationData, &resource->m_Texture);
                m_EngineContext->GetSubsystem<Renderer>()->m_GraphicsDevice->CreateSubresource(&resource->m_Texture, Subresource_Type::ShaderResourceView, 0, 1, 0, 1);

                AURORA_INFO("Successfully created HDR texture: %s.", filePath.c_str());
                return resource;
            }
        }

        return nullptr;
    }
}