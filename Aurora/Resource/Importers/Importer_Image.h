#pragma once
#include <memory>
#include "../Resource/ResourceUtilities.h"
#include "../Renderer/Renderer.h"

namespace Aurora
{
    class ImageHDR; 

    class Importer_Image
    {
    public:
        Importer_Image(EngineContext* engineContext);
        ~Importer_Image() = default;

        std::shared_ptr<AuroraResource> LoadTexture(const std::string& filePath, const std::string& fileName, uint32_t loadFlags);
        std::shared_ptr<AuroraResource> LoadHDRTexture(const std::string& filePath, int channels);


    private:
        EngineContext* m_EngineContext;
    };
}