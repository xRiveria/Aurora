#pragma once
#include <memory>
#include "../Resource/ResourceUtilities.h"
#include "../Renderer/Renderer.h"

namespace Aurora
{
    class ImageHDR; 

    class ImageDerp
    {
    public:
        static std::shared_ptr<ImageDerp> fromFile(const std::string& filename, int channels = 4);

        int width() const { return m_width; }
        int height() const { return m_height; }
        int channels() const { return m_channels; }
        int bytesPerPixel() const { return m_channels * (m_hdr ? sizeof(float) : sizeof(unsigned char)); }
        int pitch() const { return m_width * bytesPerPixel(); }

        bool isHDR() const { return m_hdr; }

        template<typename T>
        const T* pixels() const
        {
            return reinterpret_cast<const T*>(m_pixels.get());
        }

    private:
        ImageDerp();

        int m_width;
        int m_height;
        int m_channels;
        bool m_hdr;
        std::unique_ptr<unsigned char> m_pixels;
    };

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