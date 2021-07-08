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
        std::make_pair(".JPG",  Resource_Data_Type::Image),
        std::make_pair(".JPEG", Resource_Data_Type::Image),
        std::make_pair(".PNG",  Resource_Data_Type::Image),
        std::make_pair(".BMP",  Resource_Data_Type::Image),
        std::make_pair(".DDS",  Resource_Data_Type::Image),
        std::make_pair(".TGA",  Resource_Data_Type::Image),
    };

    ResourceCache::ResourceCache(EngineContext* engineContext) : ISubsystem(engineContext)
    {

    }

    ResourceCache::~ResourceCache()
    {

    }

    // File data here refers to the file path.
    std::shared_ptr<AuroraResource> ResourceCache::Load(const std::string& fileName, const std::string& filePath, uint32_t flags)
    {
        std::shared_ptr<AuroraResource> resource = std::make_shared<AuroraResource>();
        m_Resources[fileName] = resource;

        Resource_Data_Type resourceType = Resource_Data_Type::Empty;
        std::string extension = FileSystem::ConvertToUppercase(FileSystem::GetExtensionFromFilePath(filePath));

        resource->m_FilePath = filePath;

        // Retrieve resource type from file path.
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

                unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &bytesPerPixel, channelCount);
                
                if (data != nullptr)
                {
                    AURORA_INFO("Successfully loaded Texture with path: %s.", filePath.c_str());

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
                        textureDescription.m_MipLevels = 1; // (uint32_t)log2(std::max(width, height)) + 1; // + 1 for base level.
                        textureDescription.m_MiscFlags = 0;
                        textureDescription.m_Usage = Usage::Default;
                        textureDescription.m_Layout = Image_Layout::Image_Layout_Shader_Resource;

                        //uint32_t mipWidth = width;
                        RHI_Subresource_Data initializationData;
                        
                        initializationData.m_SystemMemory = data;
                        initializationData.m_SystemMemoryPitch = static_cast<uint32_t>(width * sizeof(unsigned char) * channelCount);
                        
                        loadSuccess = m_EngineContext->GetSubsystem<Renderer>()->m_GraphicsDevice->CreateTexture(&textureDescription, &initializationData, &resource->m_Texture);
                        /// Set resource name.
                    }
                }
                else
                {
                    AURORA_ERROR("Failed to load texture with path: %s.", filePath.c_str());
                    return nullptr;
                }

                stbi_image_free(data);
            }
            break;
        }

        return resource;
    }
}