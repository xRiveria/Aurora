#pragma once
#include <vector>
#include <fstream>

namespace Aurora
{
    class Entity;

    enum SerializerFlag : uint32_t
    {
        SerializerFlag_Read   = 1 << 0,
        SerializerFlag_Write  = 1 << 1,
        SerializerFlag_Append = 1 << 2
    };

    class StreamSerializer
    {
    public:
        /*
        StreamSerializer(const std::string& filePath, uint32_t serializerFlags);
        ~StreamSerializer();

        bool IsStreamOpen() const { return m_IsStreamOpen; }
        void CloseStream();

        // === Writing ===
        template<class T, class = typename std::enable_if<
            std::is_same<T, bool>::value ||
            std::is_same<T, unsigned char>::value ||
            std::is_same<T, int>::value ||
            std::

        >::type>
        void Write(T value)
        {

        }

        void Write(const std::string& value);
        void Write(const std::vector<std::string>& value);
        void Write(const std::vector<uint32_t>& value);
        void Write(const std::vector<unsigned char>& value);
        void Write(const std::vector<std::byte>& value);

        // === Reading ===
        */

    private:
        bool m_IsStreamOpen = false;
        uint32_t m_SerializerFlags = 0;
        std::ofstream m_OutputStream;
        std::ifstream m_InputStream;
    };
}