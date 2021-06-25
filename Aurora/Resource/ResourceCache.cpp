#include "Aurora.h"
#include "ResourceCache.h"
#include "Importers/stb_image/stb_image.h"
#include "Importers/tinyddsloader/tinyddsloader.h"
#include "FileSystem.h"
#include "../Renderer/Renderer.h"

namespace Aurora
{
    static const std::unordered_map<std::string, Resource_Data_Type> g_Types =
    {
        std::make_pair("JPG",  Resource_Data_Type::Image),
        std::make_pair("JPEG", Resource_Data_Type::Image),
        std::make_pair("PNG",  Resource_Data_Type::Image),
        std::make_pair("BMP",  Resource_Data_Type::Image),
        std::make_pair("DDS",  Resource_Data_Type::Image),
        std::make_pair("TGA",  Resource_Data_Type::Image),
        std::make_pair("WAV",  Resource_Data_Type::Sound),
        std::make_pair("OGG",  Resource_Data_Type::Sound),
    };

    ResourceCache::ResourceCache(EngineContext* engineContext) : ISubsystem(engineContext)
    {

    }

    ResourceCache::~ResourceCache()
    {

    }

    std::shared_ptr<AuroraResource> ResourceCache::Load(const std::string& name, uint32_t flags, const uint8_t* fileData, size_t fileSize)
    {
        std::shared_ptr<AuroraResource> resource = std::make_shared<AuroraResource>();
        m_Resources[name] = resource;

        Resource_Data_Type resourceType = Resource_Data_Type::Empty;
        std::string extension = FileSystem::ConvertToUppercase(FileSystem::GetExtensionFromFilePath(name));

        {
            auto it = g_Types.find(extension);
            if (it != g_Types.end())
            {
                resourceType = it->second;
            }
            else
            {
                AURORA_ERROR("Failed to find valid extension.");
                return nullptr;
            }
        }

        bool loadSuccess = false;

        switch (resourceType)
        {
            case Resource_Data_Type::Image:
            {
                // PNG, TGA, JPG etc.
                const int channelCount = 4;
                int width, height, bytesPerPixel;

                unsigned char* data = stbi_load_from_memory(fileData, (int)fileSize, &width, &height, &bytesPerPixel, channelCount);
                
                if (data != nullptr)
                {
                    RHI_Texture_Description textureDescription;
                    textureDescription.m_Width = (uint32_t)width;
                    textureDescription.m_Height = (uint32_t)height;
                    textureDescription.m_Layout = Image_Layout::Image_Layout_Shader_Resource;

                    if (flags & Resource_Flags::Import_Color_Grading_LUT)
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
                        for (uint32_t mip = 0; mip < textureDescription.m_MipLevels; mip++)
                        {
                            initializationData[mip].m_SystemMemory = data;
                            initializationData[mip].m_SystemMemoryPitch = static_cast<uint32_t>(mipWidth * channelCount);
                            mipWidth = std::max(1U, mipWidth / 2);
                        }

                        loadSuccess = m_EngineContext->GetSubsystem<Renderer>()->m_GraphicsDevice->CreateTexture(&textureDescription, initializationData.data(), &resource->m_Texture);
                        /// Set resource name.

                        for (uint32_t i = 0; i < resource->m_Texture.m_Description.m_MipLevels; ++i)
                        {
                            int subresourceIndex;
                            AURORA_ASSERT(subresourceIndex = m_EngineContext->GetSubsystem<Renderer>()->m_GraphicsDevice->CreateSubresourceTexture(&resource->m_Texture, Subresource_Type::ShaderResourceView, 0, 1, i, 1));
                            AURORA_ASSERT(subresourceIndex = m_EngineContext->GetSubsystem<Renderer>()->m_GraphicsDevice->CreateSubresourceTexture(&resource->m_Texture, Subresource_Type::UnorderedAccessView, 0, 1, i, 1));
                        }
                    }
                }

                stbi_image_free(data);
            }
            break;

            case Resource_Data_Type::Sound:
            {
                /// Create Sound?
            }
            break;
        }

        if (loadSuccess)
        {
            resource->m_Type = resourceType;
            resource->m_Flags = flags;

            if (resource->m_FileData.empty() && (flags & Resource_Load_Mode::Import_Allow_Retain_File_Data))
            {
                // Resource was loaded with external file data, and we want to retain the file data.
                resource->m_FileData.resize(fileSize);
                std::memcpy(resource->m_FileData.data(), fileData, fileSize);
            }
            else if (!resource->m_FileData.empty() && (flags & Resource_Load_Mode::Import_Allow_Retain_File_Data) == 0) // Retaining of data is false.
            {
                // Resource was loaded using file name and we want to discard file data.
                resource->m_FileData.clear();
            }

            /// Mip Levels.

            return resource;
        }

        return nullptr;
    }
}